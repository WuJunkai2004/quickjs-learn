#!/usr/bin/env python3
"""analyze.py — quickjs.c 拆分前的符号分析。

扫描 quickjs.c 顶层（depth 0）的所有定义：函数/变量/typedef/宏，
结合模块区间表做跨模块引用检测，输出 manifest.json 供 split.py 使用。
"""
import re
import json
import sys
from collections import defaultdict

SRC = "quickjs.c"
OUT = "tools/split/manifest.json"

# ---------------------------------------------------------------- 模块区间表
# (module, [(start, end), ...])  行号为 quickjs.c 的闭区间，已逐一核实切点。
MODULES = [
    ("runtime.c",             [(1414, 2959), (3849, 3922), (7349, 7377), (7400, 7893)]),
    ("atom.c",                [(1110, 1114), (2961, 3847)]),
    ("string.c",              [(3923, 5130)]),
    ("object.c",              [(5132, 6162)]),
    ("gc.c",                  [(6164, 7347)]),
    ("property.c",            [(7895, 11061)]),
    ("convert.c",             [(11063, 12336), (12941, 13676)]),
    ("bigint.c",              [(12338, 12939), (14578, 15395), (56180, 56375)]),
    ("dump.c",                [(13678, 14566)]),
    ("interp.c",              [(15397, 21044)]),
    ("async.c",               [(21046, 21781), (53320, 53330)]),
    ("parser.c",              [(21783, 21784), (22164, 22174), (22187, 29661)]),
    ("module.c",              [(29663, 31856)]),
    ("compiler.c",            [(31858, 37374)]),
    ("serialize.c",           [(37376, 39962)]),
    ("builtin_object.c",      [(39964, 41667), (50410, 50576), (51566, 51893)]),
    ("builtin_array.c",       [(41669, 44907)]),
    ("builtin_number.c",      [(44909, 45174), (46952, 47448)]),
    ("builtin_string.c",      [(45176, 46950)]),
    ("builtin_regexp.c",      [(47552, 49316)]),
    ("builtin_json.c",        [(49317, 50408)]),
    ("builtin_proxy.c",       [(50578, 51564)]),
    ("builtin_collections.c", [(51895, 53318), (61133, 61424)]),
    ("builtin_promise.c",     [(53332, 54716)]),
    ("builtin_global.c",      [(54718, 55012)]),
    ("builtin_date.c",        [(47450, 47551), (55013, 56178)]),
    ("intrinsics.c",          [(56377, 56729)]),
    ("builtin_typedarray.c",  [(56731, 61131)]),
]

# 特殊区段（不属于任何 .c 模块，split.py 单独处理）
# internal.h 原样区段:
HDR_FULL = [(1, 1108), (1116, 1144), (21785, 22162), (22176, 22186)]
# 前向声明块（分发到 .h / .c 头部，不整体进任何文件）
FWD_DECL_BLOCK = (1146, 1408)
# exotic methods tentative 声明（internal.h extern 化）
EXOTIC_DECLS = (1410, 1413)
# 搬 internal.h 的函数定义（改 static inline）
MOVE_TO_HDR = [(7379, 7398), (14568, 14576)]   # dbuf_put_leb128/sleb128, js_pow
# opcode_info 定义（进 parser.c，去 static）
OPCODE_INFO_DEF = (22164, 22174)

# ---------------------------------------------------------------- 读文件 + 词法
def read_lines(path):
    with open(path, "r", encoding="utf-8") as f:
        return f.read().split("\n")

def strip_comments_strings(lines):
    """返回每行的 code text：注释与字符串字面量内容替换为空格。
    处理跨行注释。返回 (code_lines, brace_delta_per_line)。"""
    code_lines = []
    braces = []
    in_block_comment = False
    for line in lines:
        out = []
        i = 0
        n = len(line)
        in_string = None  # '"' or "'"
        while i < n:
            c = line[i]
            if in_block_comment:
                if c == "*" and i + 1 < n and line[i + 1] == "/":
                    in_block_comment = False
                    out.append("  ")
                    i += 2
                else:
                    out.append(" " if c != "\t" else "\t")
                    i += 1
                continue
            if in_string:
                if c == "\\":
                    out.append("  ")
                    i += 2
                    continue
                if c == in_string:
                    in_string = None
                out.append(" " if c != "\t" else "\t")
                i += 1
                continue
            if c == "/" and i + 1 < n and line[i + 1] == "*":
                in_block_comment = True
                out.append("  ")
                i += 2
                continue
            if c == "/" and i + 1 < n and line[i + 1] == "/":
                out.append(" " * (n - i))
                break
            if c == '"' or c == "'":
                in_string = c
                out.append(" ")
                i += 1
                continue
            out.append(c)
            i += 1
        code = "".join(out)
        code_lines.append(code)
        braces.append(code.count("{") - code.count("}"))
    return code_lines, braces

ATTR_RE = re.compile(r"__attribute__\s*\(\(.*?\)\)")

def extract_func_name(text):
    """从函数声明/定义头部文本提取函数名。"""
    t = ATTR_RE.sub(" ", text)
    # 处理返回函数指针等：取第一个 '(' 前最后的标识符
    idx = t.find("(")
    if idx < 0:
        return None
    head = t[:idx]
    ids = re.findall(r"[A-Za-z_]\w*", head)
    if not ids:
        return None
    return ids[-1]

def extract_var_name(text):
    """从变量声明/定义文本提取变量名。"""
    t = ATTR_RE.sub(" ", text)
    t = re.sub(r"=.*", "", t, flags=re.S)
    t = re.sub(r"\[[^\]]*\]", " [] ", t)   # 去数组维度（避免维度内标识符误当变量名）
    t = t.rstrip(";").rstrip()
    m = re.findall(r"[A-Za-z_]\w*", t)
    return m[-1] if m else None

def extract_typedef_name(text):
    t = ATTR_RE.sub(" ", text)
    m = re.search(r"\(\s*\*\s*([A-Za-z_]\w*)\s*\)", t)   # 函数指针 typedef
    if m:
        return m.group(1)
    m = re.search(r"([A-Za-z_]\w*)\s*\(", t)             # 函数类型 typedef
    if m:
        return m.group(1)
    ids = re.findall(r"[A-Za-z_]\w*", t.rstrip(";"))
    return ids[-1] if ids else None

# ---------------------------------------------------------------- 顶层语句解析
def parse_top_level(code_lines, braces):
    """扫描 depth 0 的语句，返回 entries 列表。
    entry: {kind, name, is_static, start, end, decl_start, decl_end, head_lines}
    kind: func_def | func_decl | inline_func | var_def | var_tentative | typedef | macro | undef
    """
    entries = []
    depth = 0
    n = len(code_lines)
    i = 0
    while i < n:
        line = code_lines[i]
        stripped = line.strip()
        lineno = i + 1
        if depth == 0:
            if stripped.startswith("#"):
                if re.match(r"#\s*define\b", stripped) and code_lines[i].rstrip().endswith("\\"):
                    # 多行宏：跳过整个 define 语句（含续行），防止宏体成为孤儿顶层语句
                    j = i
                    while j < n and code_lines[j].rstrip().endswith("\\"):
                        j += 1
                    i = j + 1
                    continue
                m = re.match(r"#\s*(define|undef)\s+([A-Za-z_]\w*)", stripped)
                if m:
                    entries.append({
                        "kind": m.group(1), "name": m.group(2),
                        "start": lineno, "end": lineno, "is_static": False,
                    })
                i += 1
                continue
            if (not stripped or stripped.startswith("}")
                    or stripped in ("{", "}") or stripped.startswith("*/")):
                i += 1
                continue
            # 顶层语句开始
            head = []          # 语句头部行（code text，到 '{' 或 ';' 前）
            head_start = lineno
            stmt_has_eq = False
            stmt_is_func = False
            j = i
            done = False
            while j < n and not done:
                l = code_lines[j]
                s = l.strip()
                if s.startswith("#"):
                    # 预处理指令是独立行级实体，不属于累积中的语句
                    if re.match(r"#\s*define\b", s) and code_lines[j].rstrip().endswith("\\"):
                        while j < n and code_lines[j].rstrip().endswith("\\"):
                            j += 1
                        j += 1
                    else:
                        j += 1
                    continue
                # 检测 '=' 顶层赋值（排除 ==, <=, >=, !=）
                if not stmt_has_eq:
                    t = ATTR_RE.sub(" ", s)
                    eq = re.search(r"(?<![=!<>])=(?!=)", t)
                    if eq:
                        stmt_has_eq = True
                head.append(s)
                if braces[j] > 0:
                    brace_pos = l.find("{")
                    text_before = " ".join(head[:-1]) + " " + l[:brace_pos]
                    stmt_is_func = text_before.strip().endswith(")")
                    # 函数/类型体/初始化表结束：depth 回 0
                    d = depth
                    k = j
                    while k < n:
                        d += braces[k]
                        if d == 0:
                            break
                        k += 1
                    entry_end = k + 1
                    head_text = " ".join(head)
                    if stmt_is_func:
                        name = extract_func_name(head_text)
                        is_static = head[0].startswith("static")
                        is_inline = bool(re.match(r"static\s+(?:inline|force_inline)\b", head[0]))
                        kind = "inline_func" if is_inline else "func_def"
                        entries.append({
                            "kind": kind, "name": name, "is_static": is_static,
                            "start": head_start, "end": entry_end,
                            "decl_start": head_start, "decl_end": j + 1,  # 签名区间(含 '{' 行，合成声明时截 '{' 前)
                        })
                    elif stmt_has_eq:
                        # '= {...}' 初始化表：变量定义
                        name = extract_var_name(head_text)
                        entries.append({
                            "kind": "var_def", "name": name,
                            "is_static": head[0].startswith("static"),
                            "start": head_start, "end": entry_end,
                            "decl_start": head_start, "decl_end": entry_end,
                        })
                    else:
                        # 类型定义块（enum/struct）— 提取类型名供跨模块检测
                        block_text = "\n".join(code_lines[j] for j in range(head_start - 1, entry_end))
                        tnames = set()
                        tnames.update(re.findall(r"\}\s*([A-Za-z_]\w*)\s*;", block_text))
                        tnames.update(re.findall(r"(?:typedef\s+)?(?:struct|enum|union)\s+([A-Za-z_]\w*)", block_text))
                        tnames.discard("struct")
                        # 无名 enum 的枚举常量（行首 IDENTIFIER , / = / } 形态）
                        for bl in block_text.split("\n"):
                            m2 = re.match(r"\s*([A-Za-z_]\w*)\s*(?:,|=|\})", bl)
                            if m2:
                                tnames.add(m2.group(1))
                        entries.append({
                            "kind": "type_def_block", "name": None,
                            "type_names": sorted(tnames),
                            "is_static": False,
                            "start": head_start, "end": entry_end,
                        })
                    depth = 0
                    i = entry_end
                    done = True
                elif s.endswith(";") and depth + sum(braces[i:j + 1]) == 0:
                    # 语句在单行或累积若干行后以 ';' 结束且无嵌套
                    stmt_text = " ".join(head)
                    name = None
                    is_static = head[0].startswith("static")
                    if head[0].startswith("typedef"):
                        name = extract_typedef_name(stmt_text)
                        kind = "typedef"
                    elif "(" in ATTR_RE.sub(" ", stmt_text):
                        name = extract_func_name(stmt_text)
                        kind = "func_decl"
                    elif stmt_has_eq:
                        name = extract_var_name(stmt_text)
                        kind = "var_def"
                    else:
                        name = extract_var_name(stmt_text)
                        kind = "var_tentative"
                    entries.append({
                        "kind": kind, "name": name, "is_static": is_static,
                        "start": head_start, "end": j + 1,
                        "decl_start": head_start, "decl_end": j + 1,
                    })
                    i = j + 1
                    done = True
                elif s.endswith(";") and depth + sum(braces[i:j + 1]) == 0:
                    pass
                else:
                    j += 1
                    # 语句头部累积中（尚未 '{' 或 ';'）
                    if j < n and depth + sum(braces[i:j + 1]) < 0:
                        # 异常（顶层 '}'）— 放弃本语句
                        i = j
                        done = True
            if not done:
                # 到文件尾未闭合 — 忽略
                i = j + 1
            continue
        else:
            # depth > 0：函数体内部 — 由主循环上面的逻辑处理
            # 实际不会到这里：函数体在解析时被整体跳过
            depth += braces[i]
            i += 1
    return entries

# ---------------------------------------------------------------- 主分析
def line_to_module(lineno):
    for mod, regions in MODULES:
        for a, b in regions:
            if a <= lineno <= b:
                return mod
    return None

def main():
    lines = read_lines(SRC)
    code_lines, braces = strip_comments_strings(lines)
    n = len(lines)
    print(f"read {n} lines")

    entries = parse_top_level(code_lines, braces)
    print(f"top-level entries: {len(entries)}")
    from collections import Counter
    print(Counter(e["kind"] for e in entries))

    # ---- 符号表（定义）
    symbols = {}   # name -> entry (第一个定义优先; 重名定义记录 conflicts)
    conflicts = []
    for e in entries:
        if e["kind"] in ("func_def", "inline_func", "var_def", "typedef", "type_def_block", "macro", "undef"):
            if e["name"] is None:
                continue
            if e["kind"] in ("macro", "undef"):
                continue
            if e["name"] in symbols:
                prev = symbols[e["name"]]
                if prev["kind"] == "func_decl":
                    symbols[e["name"]] = e   # 定义覆盖声明
                else:
                    conflicts.append((e["name"], prev["start"], e["start"]))
            else:
                symbols[e["name"]] = e
    # 声明表
    decls = {}     # name -> entry (func_decl / var_tentative)
    for e in entries:
        if e["kind"] in ("func_decl", "var_tentative") and e["name"]:
            decls.setdefault(e["name"], []).append(e)

    # ---- 宏生命周期（全文件扫描：函数体内的 #define 不受 C 作用域限制，事实全局）
    macro_events = defaultdict(list)   # name -> [(line, 'define'|'undef')]
    macro_define_text = {}             # name -> define 首行
    for i in range(n):
        s = code_lines[i].strip()
        m = re.match(r"#\s*(define|undef)\s+([A-Za-z_]\w*)", s)
        if m:
            macro_events[m.group(2)].append((i + 1, m.group(1)))
            if m.group(1) == "define":
                macro_define_text.setdefault(m.group(2), i + 1)
    macro_lifetimes = {}   # name -> [(start, end)]
    for name, evs in macro_events.items():
        lifetimes = []
        open_at = None
        for ln, kind in evs:
            if kind == "define":
                open_at = ln
            elif kind == "undef" and open_at is not None:
                lifetimes.append((open_at, ln))
                open_at = None
        if open_at is not None:
            lifetimes.append((open_at, n))
        macro_lifetimes[name] = lifetimes

    # ---- 模块 token 集合（引用检测用；code text 的标识符）
    module_tokens = {}    # mod -> set of identifiers
    module_lines = {}     # mod -> list of line numbers
    name_lines = defaultdict(list)   # identifier -> [出现行号]（模块区域内）
    for mod, regions in MODULES:
        toks = set()
        lns = []
        for a, b in regions:
            for ln in range(a, min(b, n) + 1):
                lns.append(ln)
                for t in re.findall(r"[A-Za-z_]\w*", code_lines[ln - 1]):
                    toks.add(t)
                    name_lines[t].append(ln)
        module_tokens[mod] = toks
        module_lines[mod] = lns

    # quickjs.h 符号集（公共 API 判定）
    qjs_h_toks = set()
    with open("quickjs.h", "r", encoding="utf-8") as f:
        for t in re.findall(r"[A-Za-z_]\w*", f.read()):
            qjs_h_toks.add(t)

    # ---- 外部化行集合（这些行会进入 internal.h，其中的引用视作跨模块）：
    #      inline 函数体 + 所有 #define 语句行（宏值里的引用会在任意 TU 展开）
    external_lines = set()
    for e in entries:
        if e["kind"] == "inline_func":
            external_lines.update(range(e["start"], e["end"] + 1))
    i = 0
    while i < n:
        if code_lines[i].strip().startswith("#define"):
            j = i
            while True:
                external_lines.add(j + 1)
                if not lines[j].rstrip().endswith("\\"):
                    break
                j += 1
            i = j + 1
        else:
            i += 1

    # ---- 跨模块引用检测
    fwd_a, fwd_b = FWD_DECL_BLOCK
    exotic_a, exotic_b = EXOTIC_DECLS
    def refs_of(name, def_mod):
        """返回 (refs, used_by_inline)。inline 体 / 宏值里的引用视作跨模块。"""
        refs = set()
        ubi = False
        for ln in name_lines.get(name, []):
            if ln in external_lines:
                ubi = True
            else:
                m2 = line_to_module(ln)
                if m2 is not None and m2 != def_mod:
                    refs.add(m2)
        return sorted(refs), ubi

    manifest = {"modules": {}, "symbols": {}, "macros": {}, "typedefs": {}, "types": {},
                "conflicts": conflicts, "tentative_vars": [], "exotic_tentatives": [],
                "move_to_header": []}

    for name, e in symbols.items():
        if e["kind"] == "type_def_block":
            continue
        def_mod = line_to_module(e["start"])
        if def_mod is None:
            # 定义在特殊区段
            if OPCODE_INFO_DEF[0] <= e["start"] <= OPCODE_INFO_DEF[1]:
                def_mod = "parser.c"
            elif any(a <= e["start"] <= b for a, b in MOVE_TO_HDR):
                manifest["move_to_header"].append({
                    "name": name, "kind": e["kind"], "start": e["start"],
                    "end": e["end"],
                })
                continue
            else:
                continue
        refs, ubi = refs_of(name, def_mod)
        manifest["symbols"][name] = {
            "kind": e["kind"], "is_static": e["is_static"],
            "def_module": def_mod, "def_line": e["start"], "def_end": e["end"],
            "decl_start": e.get("decl_start"), "decl_end": e.get("decl_end"),
            "refs": refs, "used_by_inline": ubi,
            "in_quickjs_h": name in qjs_h_toks,
        }

    # typedef 单独归并
    for name, e in symbols.items():
        if e["kind"] == "typedef":
            def_mod = line_to_module(e["start"])
            if def_mod is None:
                continue
            refs, ubi = refs_of(name, def_mod)
            manifest["typedefs"][name] = {
                "def_module": def_mod, "def_line": e["start"], "def_end": e["end"],
                "refs": refs, "used_by_inline": ubi,
            }

    # ---- 类型定义块跨模块检测（任一类型名被其他模块或 inline 引用 → 整块搬头）
    for e in entries:
        if e["kind"] != "type_def_block":
            continue
        def_mod = line_to_module(e["start"])
        if def_mod is None:
            continue   # 头区类型已随头区进 internal.h
        cross = False
        refs = set()
        ubi = False
        for tn in e.get("type_names", []):
            for ln in name_lines.get(tn, []):
                if ln in range(e["start"], e["end"] + 1):
                    continue
                if ln in external_lines:
                    ubi = True
                else:
                    m2 = line_to_module(ln)
                    if m2 is not None and m2 != def_mod:
                        refs.add(m2)
        cross = bool(refs) or ubi
        manifest["types"][str(e["start"])] = {
            "names": e.get("type_names", []), "start": e["start"], "end": e["end"],
            "def_module": def_mod, "cross": cross,
        }

    # ---- 宏跨模块检测（生命周期内其他模块或 inline 区真实出现才算）
    # XMACRO 机制宏（quickjs-opcode.h/quickjs-atom.h 的展开宏）绝不搬移：
    # 它们的 #define...#include...#undef 生命周期由宿主 XMACRO 自管理，
    # 且 quickjs-opcode.h 尾部自带 #undef DEF/#undef def。
    # 曾因 \bdef\b 误匹配把 JS_CallInternal 函数体内的 #define def（空）
    # 搬进 internal.h，导致 opcode_info 数组丢失 19 个临时 opcode 条目。
    XMACRO_NAMES = {"DEF", "def", "FMT", "FMT_ALL"}
    for name, lifetimes in macro_lifetimes.items():
        info = {"lifetimes": lifetimes, "def_module": None, "cross": False}
        if name in XMACRO_NAMES:
            manifest["macros"][name] = info
            continue
        for a, b in lifetimes:
            def_mod = line_to_module(a)
            if def_mod is None:
                continue   # 头区宏：随头区进 internal.h，无需处理
            info["def_module"] = info["def_module"] or def_mod
            # 生命周期内其他模块区域 / inline 区的真实使用
            for ln in name_lines.get(name, []):
                if a <= ln <= b and ln != a:
                    if ln in external_lines:
                        info["cross"] = True
                        info.setdefault("used_in", set()).add("<inline>")
                    else:
                        m2 = line_to_module(ln)
                        if m2 is not None and m2 != def_mod:
                            info["cross"] = True
                            info.setdefault("used_in", set()).add(m2)
        if info["def_module"] is not None and lifetimes:
            info["def_line"] = lifetimes[0][0]
            if "used_in" in info:
                info["used_in"] = sorted(info["used_in"])
        manifest["macros"][name] = info

    # ---- tentative 变量
    for name, es in decls.items():
        for e in es:
            if e["kind"] == "var_tentative":
                rec = {"name": name, "decl_start": e["start"], "decl_end": e["end"],
                       "is_static": e["is_static"]}
                if exotic_a <= e["start"] <= exotic_b:
                    manifest["exotic_tentatives"].append(rec)
                elif e["start"] < 1414:
                    manifest["tentative_vars"].append(rec)

    with open(OUT, "w", encoding="utf-8") as f:
        json.dump(manifest, f, indent=1, ensure_ascii=False)
    print(f"wrote {OUT}")
    print(f"symbols: {len(manifest['symbols'])}, macros: {len(manifest['macros'])}, "
          f"typedefs: {len(manifest['typedefs'])}, conflicts: {len(conflicts)}")
    cross = {k: v for k, v in manifest["macros"].items() if v["cross"]}
    print(f"cross-module macros: {len(cross)}, move_to_header: {len(manifest['move_to_header'])}, "
          f"cross types: {sum(1 for v in manifest['types'].values() if v['cross'])}")
    n_extern = sum(1 for v in manifest["symbols"].values()
                   if v["is_static"] and (v["refs"] or v.get("used_by_inline"))
                   and v["kind"] != "inline_func")
    n_decl_only = sum(1 for v in manifest["symbols"].values()
                      if not v["is_static"] and (v["refs"] or v.get("used_by_inline"))
                      and not v["in_quickjs_h"])
    print(f"static symbols to extern: {n_extern}, non-static needing .h decl: {n_decl_only}")
    if conflicts:
        print("CONFLICTS (same name defined multiple times):")
        for c in conflicts[:20]:
            print("  ", c)

if __name__ == "__main__":
    main()

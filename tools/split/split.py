#!/usr/bin/env python3
"""split.py — 按 manifest.json 把 quickjs.c 切分为 src/ 下多个 .c/.h。

铁律：保持原文件行序（mv + 删行语义），diff 可机械审计。
"""
import re
import os
import json
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from analyze import (MODULES, read_lines, strip_comments_strings,
                     parse_top_level, ATTR_RE)

SRC = "quickjs.c"
OUT_DIR = "src"
MANIFEST = "tools/split/manifest.json"

# internal.h 原样区段（quickjs.c 行号闭区间）
HDR_REGIONS = [
    (1, 1108),      # license/include/宏/核心结构/atom 枚举（192 行特判替换）
    (1116, 1144),   # opcode 枚举 XMACRO
    (21785, 22162), # parser/compiler 类型区（TOK_*/JSVarDef/JSFunctionDef/JSOpCode）
    (22176, 22186), # short_opcode_info 宏
]
EXOTIC_REGION = (1410, 1413)   # tentative exotic 声明 → extern
FWD_DECL_REGION = (1146, 1408)
MOVE_TO_HDR = [(7379, 7398), (14568, 14576)]  # dbuf_put_leb128/sleb128, js_pow → static inline
TYPED_ARRAY_DECL_LINE = 192

HDR_INCLUDE = '#include "quickjs-internal.h"'

def de_static(text):
    return re.sub(r"^(\s*)static\s+", r"\1", text, count=1)

def main():
    lines = read_lines(SRC)
    n = len(lines)
    manifest = json.load(open(MANIFEST))
    symbols = manifest["symbols"]

    # ---- 重新解析拿 entries（含前向声明块条目）
    code_lines, braces = strip_comments_strings(lines)
    entries = parse_top_level(code_lines, braces)

    # ---- 前向声明块条目索引
    fwd_a, fwd_b = FWD_DECL_REGION
    fwd_entries = {}   # name -> entry（声明块中的 func_decl）
    for e in entries:
        if e["kind"] == "func_decl" and fwd_a <= e["start"] <= fwd_b:
            fwd_entries[e["name"]] = e

    # ---- 搬移行集合（inline 函数 + cross 宏 + 特判函数）
    # 先建 name -> 全部同名定义条目索引（条件编译会产生同名多定义）
    entries_by_name = {}
    for e in entries:
        if e["kind"] in ("func_def", "inline_func") and e["name"]:
            entries_by_name.setdefault(e["name"], []).append(e)

    move_lines = set()
    macro_lines = set()    # cross 宏（搬头，最先）
    type_lines = set()     # cross 类型定义块（含传递依赖）
    inline_lines_moved = set()  # inline 函数 + 特判函数（最后）
    inline_syms = {}     # name -> (def_line, def_end)
    for name, v in symbols.items():
        if v["kind"] == "inline_func":
            inline_syms[name] = (v["def_line"], v["def_end"])
            # 同名全部定义（#if/#elif/#else 多平台版本）都搬
            for e in entries_by_name.get(name, []):
                if e["kind"] == "inline_func":
                    inline_lines_moved.update(range(e["start"], e["end"] + 1))
    for v in manifest["move_to_header"]:
        inline_lines_moved.update(range(v["start"], v["end"] + 1))
    # cross 宏 define 行（含续行）+ 宏值传递传播（值里引用的其他模块宏也搬）
    macro_def_ranges = {}   # name -> (def_line, end_line)
    for name, v in manifest["macros"].items():
        if v.get("def_line"):
            ln = v["def_line"]
            end = ln
            while end <= n and lines[end - 1].rstrip().endswith("\\"):
                end += 1
            macro_def_ranges[name] = (ln, end)
    moved_macros = set()
    for name, v in manifest["macros"].items():
        if v["cross"] and v.get("def_module"):
            moved_macros.add(name)
    changed = True
    while changed:
        changed = False
        for name in list(moved_macros):
            a, b = macro_def_ranges.get(name, (None, None))
            if a is None:
                continue
            text = "\n".join(code_lines[ln - 1] for ln in range(a, b + 1))
            for ident in set(re.findall(r"[A-Za-z_]\w*", text)):
                if ident in macro_def_ranges and ident not in moved_macros:
                    v2 = manifest["macros"][ident]
                    if v2.get("def_module"):   # 头区宏已随头区进 internal.h
                        moved_macros.add(ident)
                        changed = True
    for name in moved_macros:
        a, b = macro_def_ranges[name]
        macro_lines.update(range(a, b + 1))
    # cross 类型定义块
    type_entries = []     # (start, end, names) 所有模块区间的类型块（含 typedef 枚举块）
    for e in entries:
        if e["kind"] == "type_def_block":
            mod = None
            for m, regions in MODULES:
                for a, b in regions:
                    if a <= e["start"] <= b:
                        mod = m
            if mod is not None:
                type_entries.append((e["start"], e["end"], e.get("type_names", [])))
        elif e["kind"] == "typedef":
            for m, regions in MODULES:
                for a, b in regions:
                    if a <= e["start"] <= b:
                        type_entries.append((e["start"], e["end"], [e["name"]]))
    type_entry_by_name = {}
    for start, end, names in type_entries:
        for nm in names:
            type_entry_by_name.setdefault(nm, (start, end))
    moved_type_starts = set()
    for key, v in manifest.get("types", {}).items():
        if v["cross"]:
            moved_type_starts.add(v["start"])
    for name, v in manifest.get("typedefs", {}).items():
        if v["refs"] or v.get("used_by_inline"):
            moved_type_starts.add(v["def_line"])

    def block_text(a, b):
        return "\n".join(code_lines[ln - 1] for ln in range(a, b + 1))

    # 传递依赖传播：已搬类型块文本引用的其他模块类型块也搬；
    # extern 化符号的声明文本里的类型同样触发（调用方 TU 需要类型可见）。
    def trigger_types_from_text(text):
        hit = False
        for ident in set(re.findall(r"[A-Za-z_]\w*", text)):
            ent = type_entry_by_name.get(ident)
            if ent and ent[0] not in moved_type_starts:
                moved_type_starts.add(ent[0])
                hit = True
        return hit

    def type_fixpoint():
        changed = True
        while changed:
            changed = False
            for start in list(moved_type_starts):
                for s2, e2, _ in type_entries:
                    if s2 == start:
                        if trigger_types_from_text(block_text(s2, e2)):
                            changed = True

    type_fixpoint()
    # extern 化符号的签名/声明里的类型也必须对调用方可见 → 触发传播
    for name, v in symbols.items():
        if v["kind"] == "inline_func":
            continue
        exported = bool(v["refs"]) or v.get("used_by_inline")
        if not exported:
            continue
        if not v["is_static"] and v["in_quickjs_h"]:
            continue
        if v["kind"] == "var_def":
            text = block_text(v["def_line"], min(v["def_line"] + 3, v["def_end"]))
        else:
            text = block_text(v["decl_start"], v["decl_end"])
        trigger_types_from_text(text)
    type_fixpoint()

    for start, end, _ in type_entries:
        if start in moved_type_starts:
            type_lines.update(range(start, end + 1))

    move_lines = macro_lines | type_lines | inline_lines_moved

    # ---- #if 块全搬扩展（块内所有代码行都在 move_lines → 整块搬，含指令行）
    # 注意用 code_lines（注释剔除后）判断"真实代码行"，注释行随块走。
    def pp_blocks():
        stack, blocks = [], []
        for i, l in enumerate(code_lines):
            s = l.strip()
            if s.startswith(("#if", "#ifdef", "#ifndef")):
                stack.append(i + 1)
            elif s.startswith("#endif"):
                if stack:
                    blocks.append((stack.pop(), i + 1))
        return blocks

    changed = True
    while changed:
        changed = False
        for a, b in pp_blocks():
            if a in move_lines and b in move_lines:
                continue
            inner = [ln for ln in range(a + 1, b)
                     if code_lines[ln - 1].strip()
                     and not code_lines[ln - 1].strip().startswith("#")]
            if inner and all(ln in move_lines for ln in inner):
                # 区分新增行属于哪一层：类型块扩展进 type_lines，否则 inline 层
                if any(ln in type_lines for ln in inner):
                    type_lines.update(range(a, b + 1))
                elif any(ln in macro_lines for ln in inner):
                    macro_lines.update(range(a, b + 1))
                else:
                    inline_lines_moved.update(range(a, b + 1))
                move_lines.update(range(a, b + 1))
                changed = True

    # ---- extern 化：定义首行去 static（被其他模块或搬头的 inline 引用）
    destatic_lines = set()
    export_names = set()
    for name, v in symbols.items():
        if v["kind"] == "inline_func":
            continue
        if v["is_static"] and (v["refs"] or v.get("used_by_inline")):
            destatic_lines.add(v["def_line"])
            export_names.add(name)
    # extern 化符号的**所有** static 声明行（含模块区间内的前向声明）也去 static
    for e in entries:
        if e["kind"] == "func_decl" and e["name"] in export_names and e["is_static"]:
            destatic_lines.add(e["start"])

    # ---- 声明文本工具
    def join_decl(start, end):
        return " ".join(l.strip() for l in lines[start - 1:end]).strip()

    def count_array_elems(def_line, def_end):
        """数初始化器顶层元素数（空维度数组 extern 声明需要显式尺寸）。
        用 code text（字符串/注释剔除）避免字面量逗号误计。返回 None 表示非空维度/失败。"""
        text = " ".join(code_lines[ln - 1] for ln in range(def_line, def_end + 1))
        if "#define" in text:
            return None   # 宏生成元素，静态计数不可靠；使用点若无 sizeof 则 [] 声明即可
        i = text.find("{")
        if i < 0:
            return None
        depth = 0
        commas = 0
        content = 0
        last_comma_pos = -1
        j = i
        end_pos = i
        while j < len(text):
            c = text[j]
            if c == "{":
                depth += 1
                if depth == 1:
                    j += 1
                    continue
            elif c == "}":
                depth -= 1
                if depth == 0:
                    end_pos = j
                    break
            elif c in "([":
                depth += 1
            elif c in ")]":
                depth -= 1
            if depth >= 1:
                if c == "," and depth == 1:
                    commas += 1
                    last_comma_pos = j
                elif not c.isspace():
                    content += 1
            j += 1
        if content == 0:
            return 0
        # 尾逗号：最后顶层逗号后无内容 → 元素数 = 逗号数，否则 +1
        if last_comma_pos >= 0 and not text[last_comma_pos + 1:end_pos].strip():
            return commas
        return commas + 1

    def synth_decl_for(sym):
        """从定义合成原型（无前向声明可用时）。"""
        if sym["kind"] == "var_def":
            text = join_decl(sym["def_line"], sym["def_end"])
            eq = re.search(r"(?<![=!<>])=(?!=)", text)
            if eq:
                text = text[:eq.start()].rstrip()
            text = re.sub(r"\s*\{.*$", "", text)
            text = de_static(text)
            # 空维度数组：extern 声明必须带尺寸，否则其他 TU 的 sizeof(arr) 不完整
            if re.search(r"\[\s*\]\s*$", text):
                n = count_array_elems(sym["def_line"], sym["def_end"])
                if n is not None:
                    text = re.sub(r"\[\s*\]\s*$", f"[{n}]", text)
            return "extern " + text + ";"
        # 函数：签名区间到 '{' 前
        text = join_decl(sym["decl_start"], sym["decl_end"])
        brace = text.find("{")
        if brace >= 0:
            text = text[:brace]
        text = de_static(text).rstrip()
        if not text.endswith(";"):
            text += ";"
        return text

    def decl_text_for(sym, keep_static=False):
        """优先复用前向声明块文本；否则从定义合成。keep_static 用于 .c 内部声明。
        数组变量一律合成（前向声明不带尺寸，会让 sizeof 不完整）。"""
        e = fwd_entries.get(sym_name_of(sym))
        if e is not None and sym["kind"] != "var_def":
            text = join_decl(e["start"], e["end"])
            if e["is_static"] and not keep_static:
                text = de_static(text)
            if not text.endswith(";"):
                text += ";"
            return text
        text = synth_decl_for(sym)
        if keep_static:
            text = re.sub(r"^extern\s+", "static ", text)
        return text

    def sym_name_of(sym):
        for k, v in symbols.items():
            if v is sym:
                return k
        return None

    # ---- 分发声明到 .h / .c 头部
    h_decls = {}      # module -> [decl text]
    c_decls = {}      # module -> [decl text]（保持 static 的模块内前向声明）
    for mod, _ in MODULES:
        h_decls[mod] = []
        c_decls[mod] = []

    for name, v in symbols.items():
        if v["kind"] == "inline_func":
            continue
        e = fwd_entries.get(name)
        exported = bool(v["refs"]) or v.get("used_by_inline")
        if v["is_static"]:
            if exported:
                h_decls[v["def_module"]].append(decl_text_for(v))
            else:
                if e is not None:
                    # 模块内部 static 前向声明（使用先于定义时需要）
                    c_decls[v["def_module"]].append(join_decl(e["start"], e["end"]))
        else:
            if v["in_quickjs_h"]:
                continue
            if exported:
                h_decls[v["def_module"]].append(decl_text_for(v))
            else:
                if e is not None:
                    c_decls[v["def_module"]].append(join_decl(e["start"], e["end"]))

    # 声明块里声明了但 symbols 里没有定义的（条件编译未启用的调试函数等）：
    # 定义可能存在于 type_def_block 之外的 #if 内未被识别 —— 保守丢弃（编译器会告诉我们）
    handled = set()
    for name in fwd_entries:
        if name not in symbols:
            handled.add(name)

    # ---- exotic methods tentative 声明分流：
    #      被跨模块引用 → internal.h extern 声明；仅本模块使用 → static 声明放定义模块 .c 头部
    exotic_extern_lines = []
    for ln in range(EXOTIC_REGION[0], EXOTIC_REGION[1] + 1):
        text = lines[ln - 1]
        m2 = re.search(r"([A-Za-z_]\w*)\s*;\s*$", text.strip())
        nm = m2.group(1) if m2 else None
        v = symbols.get(nm)
        if v is not None and not v["refs"] and not v.get("used_by_inline"):
            c_decls[v["def_module"]].append(text.strip())
        else:
            exotic_extern_lines.append(de_static(text))

    # ---- 生成 .c 文件
    os.makedirs(OUT_DIR, exist_ok=True)
    for mod, regions in MODULES:
        out = []
        out.append(f"/* {mod} — extracted from quickjs.c, lines "
                   + ", ".join(f"{a}-{b}" for a, b in regions) + " */")
        out.append(HDR_INCLUDE)
        out.append("")
        if c_decls[mod]:
            out.append("/* 前向声明（源自 quickjs.c 头部声明块） */")
            out.extend(c_decls[mod])
            out.append("")
        for a, b in regions:
            for ln in range(a, b + 1):
                if ln in move_lines:
                    continue
                text = lines[ln - 1]
                if ln in destatic_lines:
                    text = de_static(text)
                out.append(text)
        # 尾部去多余空行
        while out and out[-1] == "":
            out.pop()
        with open(os.path.join(OUT_DIR, mod), "w") as f:
            f.write("\n".join(out) + "\n")

    # ---- 生成模块 .h
    for mod, _ in MODULES:
        base = mod[:-2]
        guard = "QUICKJS_" + base.upper() + "_H"
        decls = h_decls[mod]
        if not decls:
            continue   # 无导出符号的模块不生成 .h
        out = [f"/* {base}.h — {mod} 导出符号声明（自动生成自 quickjs.c） */",
               f"#ifndef {guard}", f"#define {guard}", ""]
        out.extend(sorted(set(decls)))
        out += ["", f"#endif /* {guard} */"]
        with open(os.path.join(OUT_DIR, base + ".h"), "w") as f:
            f.write("\n".join(out) + "\n")

    # ---- 生成 quickjs-internal.h
    parts = []
    parts.append("/* quickjs-internal.h — 内部聚合头（自动生成自 quickjs.c 头部区） */")
    parts.append("#ifndef QUICKJS_INTERNAL_H")
    parts.append("#define QUICKJS_INTERNAL_H")
    parts.append("")
    for a, b in HDR_REGIONS:
        for ln in range(a, b + 1):
            text = lines[ln - 1]
            if ln == TYPED_ARRAY_DECL_LINE:
                text = text.replace("static ", "extern ", 1)
            parts.append(text)
        parts.append("")
    # 声明块区域（1145-1413）内混有的类型定义（typedef enum 等）与 #define 也进头文件
    hdr_type_lines = set()
    for e in entries:
        if e["kind"] in ("type_def_block", "typedef") and 1145 <= e["start"] <= 1413:
            hdr_type_lines.update(range(e["start"], e["end"] + 1))
    hdr_macro_lines = []
    for i in range(1144, min(1413, len(lines))):
        if lines[i].startswith("#define"):
            ln = i + 1
            while ln <= len(lines):
                hdr_macro_lines.append(ln)
                if not lines[ln - 1].rstrip().endswith("\\"):
                    break
                ln += 1
    if hdr_macro_lines:
        parts.append("/* 声明块区混有的宏定义（源自 quickjs.c 1146-1413） */")
        for ln in hdr_macro_lines:
            parts.append(lines[ln - 1])
        parts.append("")
    if hdr_type_lines:
        parts.append("/* 声明块区混有的类型定义（源自 quickjs.c 1146-1413） */")
        for ln in sorted(hdr_type_lines):
            parts.append(lines[ln - 1])
        parts.append("")
    # 4 个 exotic tentative → extern（仅被跨模块引用的；模块私有的已分流到 .c 头部）
    if exotic_extern_lines:
        parts.append("/* 类的 exotic 方法表（定义于各模块，此处声明） */")
        parts.extend(exotic_extern_lines)
        parts.append("")
    # opcode_info extern 声明（synth）
    oi = symbols.get("opcode_info")
    if oi:
        parts.append(synth_decl_for(oi))
        parts.append("")
    # ---- 搬移区（分三层，层内保持原行序）：
    #      1) cross 宏   2) cross 类型定义   3) inline 函数 + 特判函数
    #      （宏前置：类型/inline 可能引用宏；类型前置：inline 引用类型）
    def emit_moved(ln_set, header_comment, to_inline=False):
        parts.append(header_comment)
        for ln in sorted(ln_set):
            text = lines[ln - 1]
            if to_inline and any(a <= ln <= b for a, b in MOVE_TO_HDR):
                m2 = re.match(r"\s*static\s+", text)
                if m2:
                    text = re.sub(r"^(\s*)static\s+", r"\1static inline ", text, count=1)
                elif re.match(r"\s*(void|double|int|uint32_t)\s+\w+\(", text):
                    text = re.sub(r"^(\s*)(void|double|int|uint32_t)", r"\1static inline \2", text, count=1)
            parts.append(text)
        parts.append("")

    if macro_lines:
        emit_moved(macro_lines, "/* 跨模块宏（自 quickjs.c 按原行序搬移） */")
    if type_lines:
        emit_moved(type_lines, "/* 跨模块类型定义（自 quickjs.c 按原行序搬移） */")
    # 模块头聚合（在 inline 区之前：inline 体内调用 extern 函数需先见声明）
    parts.append("/* 模块头 */")
    for mod, _ in MODULES:
        base = mod[:-2]
        if h_decls[mod]:
            parts.append(f'#include "{base}.h"')
    parts.append("")
    if inline_lines_moved:
        emit_moved(inline_lines_moved,
                   "/* static inline 函数（自 quickjs.c 按原行序搬移） */", to_inline=True)
    parts.append("#endif /* QUICKJS_INTERNAL_H */")
    with open(os.path.join(OUT_DIR, "quickjs-internal.h"), "w") as f:
        f.write("\n".join(parts) + "\n")

    # ---- 汇总
    total_out = 0
    for mod, regions in MODULES:
        cnt = sum(1 for a, b in regions for ln in range(a, b + 1) if ln not in move_lines)
        total_out += cnt
        print(f"  {mod:26} {cnt:6} lines ({len(h_decls[mod])} exported)")
    print(f"total .c body lines: {total_out}")
    print(f"move_lines: {len(move_lines)}")

if __name__ == "__main__":
    main()

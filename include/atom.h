/* atom.h — atom.c 导出符号声明（自动生成自 quickjs.c） */
#ifndef QUICKJS_ATOM_H
#define QUICKJS_ATOM_H

BOOL JS_AtomIsArrayIndex(JSContext *ctx, uint32_t *pval, JSAtom atom);
BOOL JS_AtomIsString(JSContext *ctx, JSAtom v);
BOOL JS_AtomSymbolHasDescription(JSContext *ctx, JSAtom v);
JSAtom JS_DupAtomRT(JSRuntime *rt, JSAtom v);
JSAtom JS_NewAtomInt64(JSContext *ctx, int64_t n);
JSAtom JS_NewAtomStr(JSContext *ctx, JSString *p);
JSAtom __JS_FindAtom(JSRuntime *rt, const char *str, size_t len, int atom_type);
JSAtom __JS_NewAtomInit(JSRuntime *rt, const char *str, int len, int atom_type);
JSAtom js_atom_concat_num(JSContext *ctx, JSAtom name, uint32_t n);
JSAtom js_atom_concat_str(JSContext *ctx, JSAtom name, const char *str1);
JSAtom js_get_atom_index(JSRuntime *rt, JSAtomStruct *p);
JSAtomKindEnum JS_AtomGetKind(JSContext *ctx, JSAtom v);
JSValue JS_AtomIsNumericIndex1(JSContext *ctx, JSAtom atom);
JSValue JS_NewSymbol(JSContext *ctx, JSString *p, int atom_type);
JSValue JS_NewSymbolFromAtom(JSContext *ctx, JSAtom descr, int atom_type);
__maybe_unused void JS_DumpAtoms(JSRuntime *rt);
__maybe_unused void JS_DumpString(JSRuntime *rt, const JSString *p);
const char *JS_AtomGetStr(JSContext *ctx, char *buf, int buf_size, JSAtom atom);
const char *JS_AtomGetStrRT(JSRuntime *rt, char *buf, int buf_size, JSAtom atom);
int JS_AtomIsNumericIndex(JSContext *ctx, JSAtom atom);
int JS_InitAtoms(JSRuntime *rt);
size_t count_ascii(const uint8_t *buf, size_t len);
uint32_t hash_string(const JSString *str, uint32_t h);
uint32_t hash_string_rope(JSValueConst val, uint32_t h);
void JS_FreeAtomStruct(JSRuntime *rt, JSAtomStruct *p);

#endif /* QUICKJS_ATOM_H */

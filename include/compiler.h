/* compiler.h — compiler.c 导出符号声明（自动生成自 quickjs.c） */
#ifndef QUICKJS_COMPILER_H
#define QUICKJS_COMPILER_H

JSFunctionDef *js_new_function_def(JSContext *ctx, JSFunctionDef *parent, BOOL is_eval, BOOL is_func_expr, const char *filename, const uint8_t *source_ptr, GetLineColCache *get_line_col_cache);
JSFunctionDef *js_parse_function_class_fields_init(JSParseState *s);
JSValue JS_EvalObject(JSContext *ctx, JSValueConst this_obj, JSValueConst val, int flags, int scope_idx);
JSValue __JS_EvalInternal(JSContext *ctx, JSValueConst this_obj, const char *input, size_t input_len, const char *filename, int flags, int scope_idx);
__exception int js_parse_function_decl(JSParseState *s, JSParseFunctionEnum func_type, JSFunctionKindEnum func_kind, JSAtom func_name, const uint8_t *ptr);
__exception int js_parse_function_decl2(JSParseState *s, JSParseFunctionEnum func_type, JSFunctionKindEnum func_kind, JSAtom func_name, const uint8_t *ptr, JSParseExportEnum export_flag, JSFunctionDef **pfd);
void free_function_bytecode(JSRuntime *rt, JSFunctionBytecode *b);
void js_parse_init(JSContext *ctx, JSParseState *s, const char *input, size_t input_len, const char *filename);

#endif /* QUICKJS_COMPILER_H */

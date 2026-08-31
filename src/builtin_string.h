/* builtin_string.h — builtin_string.c 导出符号声明（自动生成自 quickjs.c） */
#ifndef QUICKJS_BUILTIN_STRING_H
#define QUICKJS_BUILTIN_STRING_H

JSValue js_string_constructor(JSContext *ctx, JSValueConst new_target, int argc, JSValueConst *argv);
double js_fmax(double a, double b);
double js_fmin(double a, double b);
extern const JSCFunctionListEntry js_string_funcs[3];
extern const JSCFunctionListEntry js_string_iterator_proto_funcs[2];
extern const JSCFunctionListEntry js_string_proto_funcs[50];
extern const JSClassExoticMethods js_string_exotic_methods;
int js_string_GetSubstitution(JSContext *ctx, StringBuffer *b, JSValueConst matched, JSString *sp, uint32_t position, JSValueConst captures_val, JSValueConst namedCaptures, JSValueConst rep, uint8_t **captures, uint32_t captures_len);
int js_string_find_invalid_codepoint(JSString *p);
int string_indexof_char(JSString *p, int c, int from);
int64_t string_advance_index(JSString *p, int64_t index, BOOL unicode);

#endif /* QUICKJS_BUILTIN_STRING_H */

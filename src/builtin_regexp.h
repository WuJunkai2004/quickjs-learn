/* builtin_regexp.h — builtin_regexp.c 导出符号声明（自动生成自 quickjs.c） */
#ifndef QUICKJS_BUILTIN_REGEXP_H
#define QUICKJS_BUILTIN_REGEXP_H

JSValue JS_NewRegexp(JSContext *ctx, JSValue pattern, JSValue bc);
int js_is_regexp(JSContext *ctx, JSValueConst obj);
void js_regexp_finalizer(JSRuntime *rt, JSValue val);
void js_regexp_string_iterator_finalizer(JSRuntime *rt, JSValue val);
void js_regexp_string_iterator_mark(JSRuntime *rt, JSValueConst val, JS_MarkFunc *mark_func);

#endif /* QUICKJS_BUILTIN_REGEXP_H */

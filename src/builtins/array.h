/* array.h — array.c 导出符号声明（自动生成自 quickjs.c） */
#ifndef QUICKJS_BUILTINS_ARRAY_H
#define QUICKJS_BUILTINS_ARRAY_H

JSValue js_array_constructor(JSContext *ctx, JSValueConst new_target, int argc, JSValueConst *argv);
JSValue js_array_every(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, int special);
JSValue js_array_includes(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
JSValue js_array_iterator_next(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, BOOL *pdone, int magic);
JSValue js_array_pop(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, int shift);
JSValue js_array_push(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, int unshift);
JSValue js_array_reduce(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, int special);
JSValue js_create_array_iterator(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, int magic);
JSValue js_get_this(JSContext *ctx, JSValueConst this_val);
JSValue js_iterator_constructor(JSContext *ctx, JSValueConst new_target, int argc, JSValueConst *argv);
JSValue js_iterator_constructor_getset(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, int magic, JSValue *func_data);
JSValue js_iterator_proto_iterator(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
JSValue js_number_constructor(JSContext *ctx, JSValueConst new_target, int argc, JSValueConst *argv);
JSValue js_number_isFinite(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
JSValue js_number_isInteger(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
JSValue js_number_isNaN(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
JSValue js_number_isSafeInteger(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
extern const JSCFunctionListEntry js_array_funcs[4];
extern const JSCFunctionListEntry js_array_iterator_proto_funcs[2];
extern const JSCFunctionListEntry js_array_proto_funcs[40];
extern const JSCFunctionListEntry js_iterator_concat_proto_funcs[3];
extern const JSCFunctionListEntry js_iterator_funcs[2];
extern const JSCFunctionListEntry js_iterator_helper_proto_funcs[3];
extern const JSCFunctionListEntry js_iterator_proto_funcs[13];
extern const JSCFunctionListEntry js_iterator_wrap_proto_funcs[2];
void js_array_iterator_finalizer(JSRuntime *rt, JSValue val);
void js_array_iterator_mark(JSRuntime *rt, JSValueConst val, JS_MarkFunc *mark_func);
void js_iterator_concat_finalizer(JSRuntime *rt, JSValue val);
void js_iterator_concat_mark(JSRuntime *rt, JSValueConst val, JS_MarkFunc *mark_func);
void js_iterator_helper_finalizer(JSRuntime *rt, JSValue val);
void js_iterator_helper_mark(JSRuntime *rt, JSValueConst val, JS_MarkFunc *mark_func);
void js_iterator_wrap_finalizer(JSRuntime *rt, JSValue val);
void js_iterator_wrap_mark(JSRuntime *rt, JSValueConst val, JS_MarkFunc *mark_func);

#endif /* QUICKJS_BUILTINS_ARRAY_H */

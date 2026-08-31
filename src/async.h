/* async.h — async.c 导出符号声明（自动生成自 quickjs.c） */
#ifndef QUICKJS_ASYNC_H
#define QUICKJS_ASYNC_H

JSValue js_async_function_call(JSContext *ctx, JSValueConst func_obj, JSValueConst this_obj, int argc, JSValueConst *argv, int flags);
JSValue js_async_function_resolve_call(JSContext *ctx, JSValueConst func_obj, JSValueConst this_obj, int argc, JSValueConst *argv, int flags);
JSValue js_async_generator_function_call(JSContext *ctx, JSValueConst func_obj, JSValueConst this_obj, int argc, JSValueConst *argv, int flags);
JSValue js_async_generator_next(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, int magic);
JSValue js_generator_function_call(JSContext *ctx, JSValueConst func_obj, JSValueConst this_obj, int argc, JSValueConst *argv, int flags);
extern const JSCFunctionListEntry js_generator_function_proto_funcs[1];
extern const JSCFunctionListEntry js_generator_proto_funcs[4];
void js_async_function_resolve_finalizer(JSRuntime *rt, JSValue val);
void js_async_function_resolve_mark(JSRuntime *rt, JSValueConst val, JS_MarkFunc *mark_func);
void js_async_generator_finalizer(JSRuntime *rt, JSValue obj);
void js_async_generator_mark(JSRuntime *rt, JSValueConst val, JS_MarkFunc *mark_func);
void js_generator_finalizer(JSRuntime *rt, JSValue obj);
void js_generator_mark(JSRuntime *rt, JSValueConst val, JS_MarkFunc *mark_func);

#endif /* QUICKJS_ASYNC_H */

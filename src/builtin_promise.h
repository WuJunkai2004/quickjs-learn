/* builtin_promise.h — builtin_promise.c 导出符号声明（自动生成自 quickjs.c） */
#ifndef QUICKJS_BUILTIN_PROMISE_H
#define QUICKJS_BUILTIN_PROMISE_H

JSValue JS_CreateAsyncFromSyncIterator(JSContext *ctx, JSValueConst sync_iter);
JSValue js_promise_resolve(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, int magic);
JSValue js_promise_then(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
__exception int perform_promise_then(JSContext *ctx, JSValueConst promise, JSValueConst *resolve_reject, JSValueConst *cap_resolving_funcs);

#endif /* QUICKJS_BUILTIN_PROMISE_H */

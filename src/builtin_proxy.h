/* builtin_proxy.h — builtin_proxy.c 导出符号声明（自动生成自 quickjs.c） */
#ifndef QUICKJS_BUILTIN_PROXY_H
#define QUICKJS_BUILTIN_PROXY_H

JSValue JS_ThrowTypeErrorRevokedProxy(JSContext *ctx);
int js_resolve_proxy(JSContext *ctx, JSValueConst *pval, int throw_exception);

#endif /* QUICKJS_BUILTIN_PROXY_H */

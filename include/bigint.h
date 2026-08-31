/* bigint.h — bigint.c 导出符号声明（自动生成自 quickjs.c） */
#ifndef QUICKJS_BIGINT_H
#define QUICKJS_BIGINT_H

JSBigInt *JS_ToBigIntBuf(JSContext *ctx, JSBigIntBuf *buf1, JSValue op1);
JSValue JS_CompactBigInt(JSContext *ctx, JSBigInt *p);
JSValue JS_StringToBigInt(JSContext *ctx, JSValue val);
JSValue JS_ToBigIntFree(JSContext *ctx, JSValue val);
JSValue js_atof(JSContext *ctx, const char *str, const char **pp, int radix, int flags);
JSValue js_bigint_to_string1(JSContext *ctx, JSValueConst val, int radix);
__exception int js_post_inc_slow(JSContext *ctx, JSValue *sp, OPCodeEnum op);
int JS_AddIntrinsicBigInt(JSContext *ctx);
int JS_ToBigInt64Free(JSContext *ctx, int64_t *pres, JSValue val);
int js_bigint_cmp(JSContext *ctx, const JSBigInt *a, const JSBigInt *b);
int js_bigint_float64_cmp(JSContext *ctx, const JSBigInt *a, double b);
no_inline __exception int js_add_slow(JSContext *ctx, JSValue *sp);
no_inline __exception int js_binary_arith_slow(JSContext *ctx, JSValue *sp, OPCodeEnum op);
no_inline __exception int js_binary_logic_slow(JSContext *ctx, JSValue *sp, OPCodeEnum op);
no_inline __exception int js_unary_arith_slow(JSContext *ctx, JSValue *sp, OPCodeEnum op);
no_inline int js_not_slow(JSContext *ctx, JSValue *sp);

#endif /* QUICKJS_BIGINT_H */

/* convert.h — convert.c 导出符号声明（自动生成自 quickjs.c） */
#ifndef QUICKJS_CONVERT_H
#define QUICKJS_CONVERT_H

BOOL JS_NumberIsNegativeOrMinusZero(JSContext *ctx, JSValueConst val);
BOOL is_safe_integer(double d);
JSBigInt *js_bigint_add(JSContext *ctx, const JSBigInt *a, const JSBigInt *b, int b_neg);
JSBigInt *js_bigint_divrem(JSContext *ctx, const JSBigInt *a, const JSBigInt *b, BOOL is_rem);
JSBigInt *js_bigint_from_float64(JSContext *ctx, int *pres, double a1);
JSBigInt *js_bigint_logic(JSContext *ctx, const JSBigInt *a, const JSBigInt *b, OPCodeEnum op);
JSBigInt *js_bigint_mul(JSContext *ctx, const JSBigInt *a, const JSBigInt *b);
JSBigInt *js_bigint_neg(JSContext *ctx, const JSBigInt *a);
JSBigInt *js_bigint_new(JSContext *ctx, int len);
JSBigInt *js_bigint_new_di(JSContext *ctx, js_sdlimb_t a);
JSBigInt *js_bigint_new_si64(JSContext *ctx, int64_t a);
JSBigInt *js_bigint_new_ui64(JSContext *ctx, uint64_t a);
JSBigInt *js_bigint_normalize(JSContext *ctx, JSBigInt *a);
JSBigInt *js_bigint_not(JSContext *ctx, const JSBigInt *a);
JSBigInt *js_bigint_pow(JSContext *ctx, const JSBigInt *a, JSBigInt *b);
JSBigInt *js_bigint_set_short(JSBigIntBuf *buf, JSValueConst val);
JSBigInt *js_bigint_set_si(JSBigIntBuf *buf, js_slimb_t a);
JSBigInt *js_bigint_shl(JSContext *ctx, const JSBigInt *a, unsigned int shift1);
JSBigInt *js_bigint_shr(JSContext *ctx, const JSBigInt *a, unsigned int shift1);
JSValue JS_ToLocaleStringFree(JSContext *ctx, JSValue val);
JSValue JS_ToNumber(JSContext *ctx, JSValueConst val);
JSValue JS_ToNumberFree(JSContext *ctx, JSValue val);
JSValue JS_ToNumeric(JSContext *ctx, JSValueConst val);
JSValue JS_ToNumericFree(JSContext *ctx, JSValue val);
JSValue JS_ToPrimitive(JSContext *ctx, JSValueConst val, int hint);
JSValue JS_ToPrimitiveFree(JSContext *ctx, JSValue val, int hint);
JSValue JS_ToStringCheckObject(JSContext *ctx, JSValueConst val);
JSValue JS_ToStringFree(JSContext *ctx, JSValue val);
JSValue js_bigint_to_string(JSContext *ctx, JSValueConst val);
JSValue js_dtoa2(JSContext *ctx, double d, int radix, int n_digits, int flags);
__exception int JS_ToArrayLengthFree(JSContext *ctx, uint32_t *plen, JSValue val, BOOL is_array_ctor);
__exception int JS_ToLengthFree(JSContext *ctx, int64_t *plen, JSValue val);
__exception int __JS_ToFloat64Free(JSContext *ctx, double *pres, JSValue val);
__maybe_unused JSValue JS_ToIntegerFree(JSContext *ctx, JSValue val);
double js_bigint_to_float64(JSContext *ctx, const JSBigInt *a);
int JS_NumberIsInteger(JSContext *ctx, JSValueConst val);
int JS_ToBoolFree(JSContext *ctx, JSValue val);
int JS_ToInt32Clamp(JSContext *ctx, int *pres, JSValueConst val, int min, int max, int min_offset);
int JS_ToInt32Free(JSContext *ctx, int32_t *pres, JSValue val);
int JS_ToInt32Sat(JSContext *ctx, int *pres, JSValueConst val);
int JS_ToInt64Clamp(JSContext *ctx, int64_t *pres, JSValueConst val, int64_t min, int64_t max, int64_t neg_offset);
int JS_ToInt64Free(JSContext *ctx, int64_t *pres, JSValue val);
int JS_ToInt64Sat(JSContext *ctx, int64_t *pres, JSValueConst val);
int JS_ToUint8ClampFree(JSContext *ctx, int32_t *pres, JSValue val);
int skip_spaces(const char *pc);
js_limb_t mp_div1(js_limb_t *tabr, const js_limb_t *taba, js_limb_t n, js_limb_t b, js_limb_t r);
js_limb_t mp_mul1(js_limb_t *tabr, const js_limb_t *taba, js_limb_t n, js_limb_t b, js_limb_t l);
js_slimb_t js_bigint_get_si_sat(const JSBigInt *a);
uint64_t js_bigint_get_mant_exp(JSContext *ctx, int *pexp, const JSBigInt *a);

#endif /* QUICKJS_CONVERT_H */

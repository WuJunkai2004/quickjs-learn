/* convert.c — extracted from quickjs.c, lines 11063-12336, 12941-13676 */
#include "quickjs-internal.h"

JSValue JS_ToPrimitiveFree(JSContext *ctx, JSValue val, int hint)
{
    int i;
    BOOL force_ordinary;

    JSAtom method_name;
    JSValue method, ret;
    if (JS_VALUE_GET_TAG(val) != JS_TAG_OBJECT)
        return val;
    force_ordinary = hint & HINT_FORCE_ORDINARY;
    hint &= ~HINT_FORCE_ORDINARY;
    if (!force_ordinary) {
        method = JS_GetProperty(ctx, val, JS_ATOM_Symbol_toPrimitive);
        if (JS_IsException(method))
            goto exception;
        /* ECMA says *If exoticToPrim is not undefined* but tests in
           test262 use null as a non callable converter */
        if (!JS_IsUndefined(method) && !JS_IsNull(method)) {
            JSAtom atom;
            JSValue arg;
            switch(hint) {
            case HINT_STRING:
                atom = JS_ATOM_string;
                break;
            case HINT_NUMBER:
                atom = JS_ATOM_number;
                break;
            default:
            case HINT_NONE:
                atom = JS_ATOM_default;
                break;
            }
            arg = JS_AtomToString(ctx, atom);
            ret = JS_CallFree(ctx, method, val, 1, (JSValueConst *)&arg);
            JS_FreeValue(ctx, arg);
            if (JS_IsException(ret))
                goto exception;
            JS_FreeValue(ctx, val);
            if (JS_VALUE_GET_TAG(ret) != JS_TAG_OBJECT)
                return ret;
            JS_FreeValue(ctx, ret);
            return JS_ThrowTypeError(ctx, "toPrimitive");
        }
    }
    if (hint != HINT_STRING)
        hint = HINT_NUMBER;
    for(i = 0; i < 2; i++) {
        if ((i ^ hint) == 0) {
            method_name = JS_ATOM_toString;
        } else {
            method_name = JS_ATOM_valueOf;
        }
        method = JS_GetProperty(ctx, val, method_name);
        if (JS_IsException(method))
            goto exception;
        if (JS_IsFunction(ctx, method)) {
            ret = JS_CallFree(ctx, method, val, 0, NULL);
            if (JS_IsException(ret))
                goto exception;
            if (JS_VALUE_GET_TAG(ret) != JS_TAG_OBJECT) {
                JS_FreeValue(ctx, val);
                return ret;
            }
            JS_FreeValue(ctx, ret);
        } else {
            JS_FreeValue(ctx, method);
        }
    }
    JS_ThrowTypeError(ctx, "toPrimitive");
exception:
    JS_FreeValue(ctx, val);
    return JS_EXCEPTION;
}

JSValue JS_ToPrimitive(JSContext *ctx, JSValueConst val, int hint)
{
    return JS_ToPrimitiveFree(ctx, JS_DupValue(ctx, val), hint);
}

void JS_SetIsHTMLDDA(JSContext *ctx, JSValueConst obj)
{
    JSObject *p;
    if (JS_VALUE_GET_TAG(obj) != JS_TAG_OBJECT)
        return;
    p = JS_VALUE_GET_OBJ(obj);
    p->is_HTMLDDA = TRUE;
}


int JS_ToBoolFree(JSContext *ctx, JSValue val)
{
    uint32_t tag = JS_VALUE_GET_TAG(val);
    switch(tag) {
    case JS_TAG_INT:
        return JS_VALUE_GET_INT(val) != 0;
    case JS_TAG_BOOL:
    case JS_TAG_NULL:
    case JS_TAG_UNDEFINED:
        return JS_VALUE_GET_INT(val);
    case JS_TAG_EXCEPTION:
        return -1;
    case JS_TAG_STRING:
        {
            BOOL ret = JS_VALUE_GET_STRING(val)->len != 0;
            JS_FreeValue(ctx, val);
            return ret;
        }
    case JS_TAG_STRING_ROPE:
        {
            BOOL ret = JS_VALUE_GET_STRING_ROPE(val)->len != 0;
            JS_FreeValue(ctx, val);
            return ret;
        }
    case JS_TAG_SHORT_BIG_INT:
        return JS_VALUE_GET_SHORT_BIG_INT(val) != 0;
    case JS_TAG_BIG_INT:
        {
            JSBigInt *p = JS_VALUE_GET_PTR(val);
            BOOL ret;
            int i;
            
            /* fail safe: we assume it is not necessarily
               normalized. Beginning from the MSB ensures that the
               test is fast. */
            ret = FALSE;
            for(i = p->len - 1; i >= 0; i--) {
                if (p->tab[i] != 0) {
                    ret = TRUE;
                    break;
                }
            }
            JS_FreeValue(ctx, val);
            return ret;
        }
    case JS_TAG_OBJECT:
        {
            JSObject *p = JS_VALUE_GET_OBJ(val);
            BOOL ret;
            ret = !p->is_HTMLDDA;
            JS_FreeValue(ctx, val);
            return ret;
        }
        break;
    default:
        if (JS_TAG_IS_FLOAT64(tag)) {
            double d = JS_VALUE_GET_FLOAT64(val);
            return !isnan(d) && d != 0;
        } else {
            JS_FreeValue(ctx, val);
            return TRUE;
        }
    }
}

int JS_ToBool(JSContext *ctx, JSValueConst val)
{
    return JS_ToBoolFree(ctx, JS_DupValue(ctx, val));
}

int skip_spaces(const char *pc)
{
    const uint8_t *p, *p_next, *p_start;
    uint32_t c;

    p = p_start = (const uint8_t *)pc;
    for (;;) {
        c = *p;
        if (c < 128) {
            if (!((c >= 0x09 && c <= 0x0d) || (c == 0x20)))
                break;
            p++;
        } else {
            c = unicode_from_utf8(p, UTF8_CHAR_LEN_MAX, &p_next);
            if (!lre_is_space(c))
                break;
            p = p_next;
        }
    }
    return p - p_start;
}


/* bigint support */


#define ADDC(res, carry_out, op1, op2, carry_in)        \
do {                                                    \
    js_limb_t __v, __a, __k, __k1;                      \
    __v = (op1);                                        \
    __a = __v + (op2);                                  \
    __k1 = __a < __v;                                   \
    __k = (carry_in);                                   \
    __a = __a + __k;                                    \
    carry_out = (__a < __k) | __k1;                     \
    res = __a;                                          \
} while (0)


/* handle a = 0 too */

static js_limb_t mp_add(js_limb_t *res, const js_limb_t *op1, const js_limb_t *op2,
                     js_limb_t n, js_limb_t carry)
{
    int i;
    for(i = 0;i < n; i++) {
        ADDC(res[i], carry, op1[i], op2[i], carry);
    }
    return carry;
}

static js_limb_t mp_sub(js_limb_t *res, const js_limb_t *op1, const js_limb_t *op2,
                        int n, js_limb_t carry)
{
    int i;
    js_limb_t k, a, v, k1;

    k = carry;
    for(i=0;i<n;i++) {
        v = op1[i];
        a = v - op2[i];
        k1 = a > v;
        v = a - k;
        k = (v > a) | k1;
        res[i] = v;
    }
    return k;
}

/* compute 0 - op2. carry = 0 or 1. */
static js_limb_t mp_neg(js_limb_t *res, const js_limb_t *op2, int n)
{
    int i;
    js_limb_t v, carry;

    carry = 1;
    for(i=0;i<n;i++) {
        v = ~op2[i] + carry;
        carry = v < carry;
        res[i] = v;
    }
    return carry;
}

/* tabr[] = taba[] * b + l. Return the high carry */
js_limb_t mp_mul1(js_limb_t *tabr, const js_limb_t *taba, js_limb_t n,
                      js_limb_t b, js_limb_t l)
{
    js_limb_t i;
    js_dlimb_t t;

    for(i = 0; i < n; i++) {
        t = (js_dlimb_t)taba[i] * (js_dlimb_t)b + l;
        tabr[i] = t;
        l = t >> JS_LIMB_BITS;
    }
    return l;
}

js_limb_t mp_div1(js_limb_t *tabr, const js_limb_t *taba, js_limb_t n,
                      js_limb_t b, js_limb_t r)
{
    js_slimb_t i;
    js_dlimb_t a1;
    for(i = n - 1; i >= 0; i--) {
        a1 = ((js_dlimb_t)r << JS_LIMB_BITS) | taba[i];
        tabr[i] = a1 / b;
        r = a1 % b;
    }
    return r;
}

/* tabr[] += taba[] * b, return the high word. */
static js_limb_t mp_add_mul1(js_limb_t *tabr, const js_limb_t *taba, js_limb_t n,
                          js_limb_t b)
{
    js_limb_t i, l;
    js_dlimb_t t;

    l = 0;
    for(i = 0; i < n; i++) {
        t = (js_dlimb_t)taba[i] * (js_dlimb_t)b + l + tabr[i];
        tabr[i] = t;
        l = t >> JS_LIMB_BITS;
    }
    return l;
}

/* size of the result : op1_size + op2_size. */
static void mp_mul_basecase(js_limb_t *result,
                            const js_limb_t *op1, js_limb_t op1_size,
                            const js_limb_t *op2, js_limb_t op2_size)
{
    int i;
    js_limb_t r;
    
    result[op1_size] = mp_mul1(result, op1, op1_size, op2[0], 0);
    for(i=1;i<op2_size;i++) {
        r = mp_add_mul1(result + i, op1, op1_size, op2[i]);
        result[i + op1_size] = r;
    }
}

/* tabr[] -= taba[] * b. Return the value to substract to the high
   word. */
static js_limb_t mp_sub_mul1(js_limb_t *tabr, const js_limb_t *taba, js_limb_t n,
                          js_limb_t b)
{
    js_limb_t i, l;
    js_dlimb_t t;

    l = 0;
    for(i = 0; i < n; i++) {
        t = tabr[i] - (js_dlimb_t)taba[i] * (js_dlimb_t)b - l;
        tabr[i] = t;
        l = -(t >> JS_LIMB_BITS);
    }
    return l;
}

/* WARNING: d must be >= 2^(JS_LIMB_BITS-1) */

/* return the quotient and the remainder in '*pr'of 'a1*2^JS_LIMB_BITS+a0
   / d' with 0 <= a1 < d. */

#define UDIV1NORM_THRESHOLD 3

/* b must be >= 1 << (JS_LIMB_BITS - 1) */
static js_limb_t mp_div1norm(js_limb_t *tabr, const js_limb_t *taba, js_limb_t n,
                          js_limb_t b, js_limb_t r)
{
    js_slimb_t i;

    if (n >= UDIV1NORM_THRESHOLD) {
        js_limb_t b_inv;
        b_inv = udiv1norm_init(b);
        for(i = n - 1; i >= 0; i--) {
            tabr[i] = udiv1norm(&r, r, taba[i], b, b_inv);
        }
    } else {
        js_dlimb_t a1;
        for(i = n - 1; i >= 0; i--) {
            a1 = ((js_dlimb_t)r << JS_LIMB_BITS) | taba[i];
            tabr[i] = a1 / b;
            r = a1 % b;
        }
    }
    return r;
}

/* base case division: divides taba[0..na-1] by tabb[0..nb-1]. tabb[nb
   - 1] must be >= 1 << (JS_LIMB_BITS - 1). na - nb must be >= 0. 'taba'
   is modified and contains the remainder (nb limbs). tabq[0..na-nb]
   contains the quotient with tabq[na - nb] <= 1. */
static void mp_divnorm(js_limb_t *tabq, js_limb_t *taba, js_limb_t na,
                       const js_limb_t *tabb, js_limb_t nb)
{
    js_limb_t r, a, c, q, v, b1, b1_inv, n, dummy_r;
    int i, j;

    b1 = tabb[nb - 1];
    if (nb == 1) {
        taba[0] = mp_div1norm(tabq, taba, na, b1, 0);
        return;
    }
    n = na - nb;

    if (n >= UDIV1NORM_THRESHOLD)
        b1_inv = udiv1norm_init(b1);
    else
        b1_inv = 0;

    /* first iteration: the quotient is only 0 or 1 */
    q = 1;
    for(j = nb - 1; j >= 0; j--) {
        if (taba[n + j] != tabb[j]) {
            if (taba[n + j] < tabb[j])
                q = 0;
            break;
        }
    }
    tabq[n] = q;
    if (q) {
        mp_sub(taba + n, taba + n, tabb, nb, 0);
    }

    for(i = n - 1; i >= 0; i--) {
        if (unlikely(taba[i + nb] >= b1)) {
            q = -1;
        } else if (b1_inv) {
            q = udiv1norm(&dummy_r, taba[i + nb], taba[i + nb - 1], b1, b1_inv);
        } else {
            js_dlimb_t al;
            al = ((js_dlimb_t)taba[i + nb] << JS_LIMB_BITS) | taba[i + nb - 1];
            q = al / b1;
            r = al % b1;
        }
        r = mp_sub_mul1(taba + i, tabb, nb, q);

        v = taba[i + nb];
        a = v - r;
        c = (a > v);
        taba[i + nb] = a;

        if (c != 0) {
            /* negative result */
            for(;;) {
                q--;
                c = mp_add(taba + i, taba + i, tabb, nb, 0);
                /* propagate carry and test if positive result */
                if (c != 0) {
                    if (++taba[i + nb] == 0) {
                        break;
                    }
                }
            }
        }
        tabq[i] = q;
    }
}

/* 1 <= shift <= JS_LIMB_BITS - 1 */
static js_limb_t mp_shl(js_limb_t *tabr, const js_limb_t *taba, int n,
                        int shift)
{
    int i;
    js_limb_t l, v;
    l = 0;
    for(i = 0; i < n; i++) {
        v = taba[i];
        tabr[i] = (v << shift) | l;
        l = v >> (JS_LIMB_BITS - shift);
    }
    return l;
}

/* r = (a + high*B^n) >> shift. Return the remainder r (0 <= r < 2^shift). 
   1 <= shift <= LIMB_BITS - 1 */
static js_limb_t mp_shr(js_limb_t *tab_r, const js_limb_t *tab, int n,
                        int shift, js_limb_t high)
{
    int i;
    js_limb_t l, a;

    l = high;
    for(i = n - 1; i >= 0; i--) {
        a = tab[i];
        tab_r[i] = (a >> shift) | (l << (JS_LIMB_BITS - shift));
        l = a;
    }
    return l & (((js_limb_t)1 << shift) - 1);
}

JSBigInt *js_bigint_new(JSContext *ctx, int len)
{
    JSBigInt *r;
    if (len > JS_BIGINT_MAX_SIZE) {
        JS_ThrowRangeError(ctx, "BigInt is too large to allocate");
        return NULL;
    }
    r = js_malloc(ctx, sizeof(JSBigInt) + len * sizeof(js_limb_t));
    if (!r)
        return NULL;
    js_rc(r)->ref_count = 1;
    r->len = len;
    return r;
}

JSBigInt *js_bigint_set_si(JSBigIntBuf *buf, js_slimb_t a)
{
    JSBigInt *r = (JSBigInt *)buf->big_int_buf;
    r->len = 1;
    r->tab[0] = a;
    return r;
}

static JSBigInt *js_bigint_set_si64(JSBigIntBuf *buf, int64_t a)
{
#if JS_LIMB_BITS == 64
    return js_bigint_set_si(buf, a);
#else
    JSBigInt *r = (JSBigInt *)buf->big_int_buf;
    if (a >= INT32_MIN && a <= INT32_MAX) {
        r->len = 1;
        r->tab[0] = a;
    } else {
        r->len = 2;
        r->tab[0] = a;
        r->tab[1] = a >> JS_LIMB_BITS;
    }
    return r;
#endif
}

/* val must be a short big int */
JSBigInt *js_bigint_set_short(JSBigIntBuf *buf, JSValueConst val)
{
    return js_bigint_set_si(buf, JS_VALUE_GET_SHORT_BIG_INT(val));
}

static __maybe_unused void js_bigint_dump1(JSContext *ctx, const char *str,
                                           const js_limb_t *tab, int len)
{
    int i;
    printf("%s: ", str);
    for(i = len - 1; i >= 0; i--) {
#if JS_LIMB_BITS == 32
        printf(" %08x", tab[i]);
#else
        printf(" %016" PRIx64, tab[i]);
#endif
    }
    printf("\n");
}

static __maybe_unused void js_bigint_dump(JSContext *ctx, const char *str,
                                          const JSBigInt *p)
{
    js_bigint_dump1(ctx, str, p->tab, p->len);
}

static JSBigInt *js_bigint_new_si(JSContext *ctx, js_slimb_t a)
{
    JSBigInt *r;
    r = js_bigint_new(ctx, 1);
    if (!r)
        return NULL;
    r->tab[0] = a;
    return r;
}

JSBigInt *js_bigint_new_si64(JSContext *ctx, int64_t a)
{
#if JS_LIMB_BITS == 64
    return js_bigint_new_si(ctx, a);
#else
    if (a >= INT32_MIN && a <= INT32_MAX) {
        return js_bigint_new_si(ctx, a);
    } else {
        JSBigInt *r;
        r = js_bigint_new(ctx, 2);
        if (!r)
            return NULL;
        r->tab[0] = a;
        r->tab[1] = a >> 32;
        return r;
    }
#endif
}

JSBigInt *js_bigint_new_ui64(JSContext *ctx, uint64_t a)
{
    if (a <= INT64_MAX) {
        return js_bigint_new_si64(ctx, a);
    } else {
        JSBigInt *r;
        r = js_bigint_new(ctx, (65 + JS_LIMB_BITS - 1) / JS_LIMB_BITS);
        if (!r)
            return NULL;
#if JS_LIMB_BITS == 64
        r->tab[0] = a;
        r->tab[1] = 0;
#else
        r->tab[0] = a;
        r->tab[1] = a >> 32;
        r->tab[2] = 0;
#endif
        return r;
    }
}

JSBigInt *js_bigint_new_di(JSContext *ctx, js_sdlimb_t a)
{
    JSBigInt *r;
    if (a == (js_slimb_t)a) {
        r = js_bigint_new(ctx, 1);
        if (!r)
            return NULL;
        r->tab[0] = a;
    } else {
        r = js_bigint_new(ctx, 2);
        if (!r)
            return NULL;
        r->tab[0] = a;
        r->tab[1] = a >> JS_LIMB_BITS;
    }
    return r;
}

/* Remove redundant high order limbs. Warning: 'a' may be
   reallocated. Can never fail.
*/
static JSBigInt *js_bigint_normalize1(JSContext *ctx, JSBigInt *a, int l)
{
    js_limb_t v;

    assert(js_rc(a)->ref_count == 1);
    while (l > 1) {
        v = a->tab[l - 1];
        if ((v != 0 && v != -1) ||
            (v & 1) != (a->tab[l - 2] >> (JS_LIMB_BITS - 1))) {
            break;
        }
        l--;
    }
    if (l != a->len) {
        JSBigInt *a1;
        /* realloc to reduce the size */
        a->len = l;
        a1 = js_realloc(ctx, a, sizeof(JSBigInt) + l * sizeof(js_limb_t));
        if (a1)
            a = a1;
    }
    return a;
}

JSBigInt *js_bigint_normalize(JSContext *ctx, JSBigInt *a)
{
    return js_bigint_normalize1(ctx, a, a->len);
}

/* return 0 or 1 depending on the sign */

js_slimb_t js_bigint_get_si_sat(const JSBigInt *a)
{
    if (a->len == 1) {
        return a->tab[0];
    } else {
#if JS_LIMB_BITS == 32
        if (js_bigint_sign(a))
            return INT32_MIN;
        else
            return INT32_MAX;
#else
        if (js_bigint_sign(a))
            return INT64_MIN;
        else
            return INT64_MAX;
#endif
    }
}

/* add the op1 limb */
static JSBigInt *js_bigint_extend(JSContext *ctx, JSBigInt *r,
                                  js_limb_t op1)
{
    int n2 = r->len;
    if ((op1 != 0 && op1 != -1) ||
        (op1 & 1) != r->tab[n2 - 1] >> (JS_LIMB_BITS - 1)) {
        JSBigInt *r1;
        r1 = js_realloc(ctx, r,
                        sizeof(JSBigInt) + (n2 + 1) * sizeof(js_limb_t));
        if (!r1) {
            js_free(ctx, r);
            return NULL;
        }
        r = r1;
        r->len = n2 + 1;
        r->tab[n2] = op1;
    } else {
        /* otherwise still need to normalize the result */
        r = js_bigint_normalize(ctx, r);
    }
    return r;
}

/* return NULL in case of error. Compute a + b (b_neg = 0) or a - b
   (b_neg = 1) */
/* XXX: optimize */
JSBigInt *js_bigint_add(JSContext *ctx, const JSBigInt *a,
                               const JSBigInt *b, int b_neg)
{
    JSBigInt *r;
    int n1, n2, i;
    js_limb_t carry, op1, op2, a_sign, b_sign;
    
    n2 = max_int(a->len, b->len);
    n1 = min_int(a->len, b->len);
    r = js_bigint_new(ctx, n2);
    if (!r)
        return NULL;
    /* XXX: optimize */
    /* common part */
    carry = b_neg;
    for(i = 0; i < n1; i++) {
        op1 = a->tab[i];
        op2 = b->tab[i] ^ (-b_neg);
        ADDC(r->tab[i], carry, op1, op2, carry);
    }
    a_sign = -js_bigint_sign(a);
    b_sign = (-js_bigint_sign(b)) ^ (-b_neg);
    /* part with sign extension of one operand  */
    if (a->len > b->len) {
        for(i = n1; i < n2; i++) {
            op1 = a->tab[i];
            ADDC(r->tab[i], carry, op1, b_sign, carry);
        }
    } else if (a->len < b->len) {
        for(i = n1; i < n2; i++) {
            op2 = b->tab[i] ^ (-b_neg);
            ADDC(r->tab[i], carry, a_sign, op2, carry);
        }
    }

    /* part with sign extension for both operands. Extend the result
       if necessary */
    return js_bigint_extend(ctx, r, a_sign + b_sign + carry);
}

/* XXX: optimize */
JSBigInt *js_bigint_neg(JSContext *ctx, const JSBigInt *a)
{
    JSBigIntBuf buf;
    JSBigInt *b;
    b = js_bigint_set_si(&buf, 0);
    return js_bigint_add(ctx, b, a, 1);
}

JSBigInt *js_bigint_mul(JSContext *ctx, const JSBigInt *a,
                               const JSBigInt *b)
{
    JSBigInt *r;
    
    r = js_bigint_new(ctx, a->len + b->len);
    if (!r)
        return NULL;
    mp_mul_basecase(r->tab, a->tab, a->len, b->tab, b->len);
    /* correct the result if negative operands (no overflow is
       possible) */
    if (js_bigint_sign(a))
        mp_sub(r->tab + a->len, r->tab + a->len, b->tab, b->len, 0);
    if (js_bigint_sign(b))
        mp_sub(r->tab + b->len, r->tab + b->len, a->tab, a->len, 0);
    return js_bigint_normalize(ctx, r);
}

/* return the division or the remainder. 'b' must be != 0. return NULL
   in case of exception (division by zero or memory error) */
JSBigInt *js_bigint_divrem(JSContext *ctx, const JSBigInt *a,
                                  const JSBigInt *b, BOOL is_rem)
{
    JSBigInt *r, *q;
    js_limb_t *tabb, h;
    int na, nb, a_sign, b_sign, shift;
    
    if (b->len == 1 && b->tab[0] == 0) {
        JS_ThrowRangeError(ctx, "BigInt division by zero");
        return NULL;
    }
    
    a_sign = js_bigint_sign(a);
    b_sign = js_bigint_sign(b);
    na = a->len;
    nb = b->len;

    r = js_bigint_new(ctx, na + 2); 
    if (!r)
        return NULL;
    if (a_sign) {
        mp_neg(r->tab, a->tab, na);
    } else {
        memcpy(r->tab, a->tab, na * sizeof(a->tab[0]));
    }
    /* normalize */
    while (na > 1 && r->tab[na - 1] == 0)
        na--;

    tabb = js_malloc(ctx, nb * sizeof(tabb[0]));
    if (!tabb) {
        js_free(ctx, r);
        return NULL;
    }
    if (b_sign) {
        mp_neg(tabb, b->tab, nb);
    } else {
        memcpy(tabb, b->tab, nb * sizeof(tabb[0]));
    }
    /* normalize */
    while (nb > 1 && tabb[nb - 1] == 0)
        nb--;

    /* trivial case if 'a' is small */
    if (na < nb) {
        js_free(ctx, r);
        js_free(ctx, tabb);
        if (is_rem) {
            /* r = a */
            r = js_bigint_new(ctx, a->len);
            if (!r)
                return NULL;
            memcpy(r->tab, a->tab, a->len * sizeof(a->tab[0])); 
            return r;
        } else {
            /* q = 0 */
            return js_bigint_new_si(ctx, 0);
        }
    }

    /* normalize 'b' */
    shift = js_limb_clz(tabb[nb - 1]);
    if (shift != 0) {
        mp_shl(tabb, tabb, nb, shift);
        h = mp_shl(r->tab, r->tab, na, shift);
        if (h != 0)
            r->tab[na++] = h;
    }

    q = js_bigint_new(ctx, na - nb + 2); /* one more limb for the sign */
    if (!q) {
        js_free(ctx, r);
        js_free(ctx, tabb);
        return NULL;
    }

    //    js_bigint_dump1(ctx, "a", r->tab, na);
    //    js_bigint_dump1(ctx, "b", tabb, nb);
    mp_divnorm(q->tab, r->tab, na, tabb, nb);
    js_free(ctx, tabb);

    if (is_rem) {
        js_free(ctx, q);
        if (shift != 0)
            mp_shr(r->tab, r->tab, nb, shift, 0);
        r->tab[nb++] = 0;
        if (a_sign)
            mp_neg(r->tab, r->tab, nb);
        r = js_bigint_normalize1(ctx, r, nb);
        return r;
    } else {
        js_free(ctx, r);
        q->tab[na - nb + 1] = 0;
        if (a_sign ^ b_sign) {
            mp_neg(q->tab, q->tab, q->len);
        }
        q = js_bigint_normalize(ctx, q);
        return q;
    }
}

/* and, or, xor */
JSBigInt *js_bigint_logic(JSContext *ctx, const JSBigInt *a,
                                 const JSBigInt *b, OPCodeEnum op)
{
    JSBigInt *r;
    js_limb_t b_sign;
    int a_len, b_len, i;

    if (a->len < b->len) {
        const JSBigInt *tmp;
        tmp = a;
        a = b;
        b = tmp;
    }
    /* a_len >= b_len */
    a_len = a->len;
    b_len = b->len;
    b_sign = -js_bigint_sign(b);

    r = js_bigint_new(ctx, a_len);
    if (!r)
        return NULL;
    switch(op) {
    case OP_or:
        for(i = 0; i < b_len; i++) {
            r->tab[i] = a->tab[i] | b->tab[i];
        }
        for(i = b_len; i < a_len; i++) {
            r->tab[i] = a->tab[i] | b_sign;
        }
        break;
    case OP_and:
        for(i = 0; i < b_len; i++) {
            r->tab[i] = a->tab[i] & b->tab[i];
        }
        for(i = b_len; i < a_len; i++) {
            r->tab[i] = a->tab[i] & b_sign;
        }
        break;
    case OP_xor:
        for(i = 0; i < b_len; i++) {
            r->tab[i] = a->tab[i] ^ b->tab[i];
        }
        for(i = b_len; i < a_len; i++) {
            r->tab[i] = a->tab[i] ^ b_sign;
        }
        break;
    default:
        abort();
    }
    return js_bigint_normalize(ctx, r);
}

JSBigInt *js_bigint_not(JSContext *ctx, const JSBigInt *a)
{
    JSBigInt *r;
    int i;
    
    r = js_bigint_new(ctx, a->len);
    if (!r)
        return NULL;
    for(i = 0; i < a->len; i++) {
        r->tab[i] = ~a->tab[i];
    }
    /* no normalization is needed */
    return r;
}

JSBigInt *js_bigint_shl(JSContext *ctx, const JSBigInt *a,
                               unsigned int shift1)
{
    int d, i, shift;
    JSBigInt *r;
    js_limb_t l;

    if (a->len == 1 && a->tab[0] == 0)
        return js_bigint_new_si(ctx, 0); /* zero case */
    d = shift1 / JS_LIMB_BITS;
    shift = shift1 % JS_LIMB_BITS;
    r = js_bigint_new(ctx, a->len + d);
    if (!r)
        return NULL;
    for(i = 0; i < d; i++)
        r->tab[i] = 0;
    if (shift == 0) {
        for(i = 0; i < a->len; i++) {
            r->tab[i + d] = a->tab[i];
        }
    } else {
        l = mp_shl(r->tab + d, a->tab, a->len, shift);
        if (js_bigint_sign(a))
            l |= (js_limb_t)(-1) << shift;
        r = js_bigint_extend(ctx, r, l);
    }
    return r;
}

JSBigInt *js_bigint_shr(JSContext *ctx, const JSBigInt *a,
                               unsigned int shift1)
{
    int d, i, shift, a_sign, n1;
    JSBigInt *r;

    d = shift1 / JS_LIMB_BITS;
    shift = shift1 % JS_LIMB_BITS;
    a_sign = js_bigint_sign(a);
    if (d >= a->len)
        return js_bigint_new_si(ctx, -a_sign);
    n1 = a->len - d;
    r = js_bigint_new(ctx, n1);
    if (!r)
        return NULL;
    if (shift == 0) {
        for(i = 0; i < n1; i++) {
            r->tab[i] = a->tab[i + d];
        }
        /* no normalization is needed */
    } else {
        mp_shr(r->tab, a->tab + d, n1, shift, -a_sign);
        r = js_bigint_normalize(ctx, r);
    }
    return r;
}

JSBigInt *js_bigint_pow(JSContext *ctx, const JSBigInt *a, JSBigInt *b)
{
    uint32_t e;
    int n_bits, i;
    JSBigInt *r, *r1;
    
    /* b must be >= 0 */
    if (js_bigint_sign(b)) {
        JS_ThrowRangeError(ctx, "BigInt negative exponent");
        return NULL;
    }
    if (b->len == 1 && b->tab[0] == 0) {
        /* a^0 = 1 */
        return js_bigint_new_si(ctx, 1);
    } else if (a->len == 1) {
        js_limb_t v;
        BOOL is_neg;

        v = a->tab[0];
        if (v <= 1)
            return js_bigint_new_si(ctx, v);
        else if (v == -1)
            return js_bigint_new_si(ctx, 1 - 2 * (b->tab[0] & 1));
        is_neg = (js_slimb_t)v < 0;
        if (is_neg)
            v = -v;
        if ((v & (v - 1)) == 0) {
            uint64_t e1;
            int n;
            /* v = 2^n */
            n = JS_LIMB_BITS - 1 - js_limb_clz(v);
            if (b->len > 1)
                goto overflow;
            if (b->tab[0] > INT32_MAX)
                goto overflow;
            e = b->tab[0];
            e1 = (uint64_t)e * n;
            if (e1 > JS_BIGINT_MAX_SIZE * JS_LIMB_BITS)
                goto overflow;
            e = e1;
            if (is_neg)
                is_neg = b->tab[0] & 1;
            r = js_bigint_new(ctx,
                              (e + JS_LIMB_BITS + 1 - is_neg) / JS_LIMB_BITS);
            if (!r)
                return NULL;
            memset(r->tab, 0, sizeof(r->tab[0]) * r->len);
            r->tab[e / JS_LIMB_BITS] =
                (js_limb_t)(1 - 2 * is_neg) << (e % JS_LIMB_BITS);
            return r;
        }
    }
    if (b->len > 1)
        goto overflow;
    if (b->tab[0] > INT32_MAX)
        goto overflow;
    e = b->tab[0];
    n_bits = 32 - clz32(e);

    r = js_bigint_new(ctx, a->len);
    if (!r)
        return NULL;
    memcpy(r->tab, a->tab, a->len * sizeof(a->tab[0]));
    for(i = n_bits - 2; i >= 0; i--) {
        r1 = js_bigint_mul(ctx, r, r);
        if (!r1)
            return NULL;
        js_free(ctx, r);
        r = r1;
        if ((e >> i) & 1) {
            r1 = js_bigint_mul(ctx, r, a);
            if (!r1)
                return NULL;
            js_free(ctx, r);
            r = r1;
        }
    }
    return r;
 overflow:
    JS_ThrowRangeError(ctx, "BigInt is too large");
    return NULL;
}

/* return (mant, exp) so that abs(a) ~ mant*2^(exp - (limb_bits -
   1). a must be != 0. */
uint64_t js_bigint_get_mant_exp(JSContext *ctx,
                                       int *pexp, const JSBigInt *a)
{
    js_limb_t t[4 - JS_LIMB_BITS / 32], carry, v, low_bits;
    int n1, n2, sgn, shift, i, j, e;
    uint64_t a1, a0;

    n2 = 4 - JS_LIMB_BITS / 32;
    n1 = a->len - n2;
    sgn = js_bigint_sign(a);

    /* low_bits != 0 if there are a non zero low bit in abs(a) */
    low_bits = 0;
    carry = sgn;
    for(i = 0; i < n1; i++) {
        v = (a->tab[i] ^ (-sgn)) + carry;
        carry = v < carry;
        low_bits |= v;
    }
    /* get the n2 high limbs of abs(a) */
    for(j = 0; j < n2; j++) {
        i = j + n1;
        if (i < 0) {
            v = 0;
        } else {
            v = (a->tab[i] ^ (-sgn)) + carry;
            carry = v < carry;
        }
        t[j] = v;
    }
    
#if JS_LIMB_BITS == 32
    a1 = ((uint64_t)t[2] << 32) | t[1];
    a0 = (uint64_t)t[0] << 32;
#else
    a1 = t[1];
    a0 = t[0];
#endif
    a0 |= (low_bits != 0);
    /* normalize */
    if (a1 == 0) {
        /* JS_LIMB_BITS = 64 bit only */
        shift = 64;
        a1 = a0;
        a0 = 0;
    } else {
        shift = clz64(a1);
        if (shift != 0) {
            a1 = (a1 << shift) | (a0 >> (64 - shift));
            a0 <<= shift;
        }
    }
    a1 |= (a0 != 0); /* keep the bits for the final rounding */
    /* compute the exponent */
    e = a->len * JS_LIMB_BITS - shift - 1;
    *pexp = e;
    return a1;
}

/* shift left with round to nearest, ties to even. n >= 1 */
static uint64_t shr_rndn(uint64_t a, int n)
{
    uint64_t addend = ((a >> n) & 1) + ((1 << (n - 1)) - 1);
    return (a + addend) >> n;
}

/* convert to float64 with round to nearest, ties to even. Return
   +/-infinity if too large. */
double js_bigint_to_float64(JSContext *ctx, const JSBigInt *a)
{
    int sgn, e;
    uint64_t mant;

    if (a->len == 1) {
        /* fast case, including zero */
        return (double)(js_slimb_t)a->tab[0];
    }

    sgn = js_bigint_sign(a);
    mant = js_bigint_get_mant_exp(ctx, &e, a);
    if (e > 1023) {
        /* overflow: return infinity */
        mant = 0;
        e = 1024;
    } else {
        mant = (mant >> 1) | (mant & 1); /* avoid overflow in rounding */
        mant = shr_rndn(mant, 10);
        /* rounding can cause an overflow */
        if (mant >= ((uint64_t)1 << 53)) {
            mant >>= 1;
            e++;
        }
        mant &= (((uint64_t)1 << 52) - 1);
    }
    return uint64_as_float64(((uint64_t)sgn << 63) |
                             ((uint64_t)(e + 1023) << 52) |
                             mant);
}

/* return (1, NULL) if not an integer, (2, NULL) if NaN or Infinity,
   (0, n) if an integer, (0, NULL) in case of memory error */
JSBigInt *js_bigint_from_float64(JSContext *ctx, int *pres, double a1)
{
    uint64_t a = float64_as_uint64(a1);
    int sgn, e, shift;
    uint64_t mant;
    JSBigIntBuf buf;
    JSBigInt *r;
    
    sgn = a >> 63;
    e = (a >> 52) & ((1 << 11) - 1);
    mant = a & (((uint64_t)1 << 52) - 1);
    if (e == 2047) {
        /* NaN, Infinity */
        *pres = 2;
        return NULL;
    }
    if (e == 0 && mant == 0) {
        /* zero */
        *pres = 0;
        return js_bigint_new_si(ctx, 0);
    }
    e -= 1023;
    /* 0 < a < 1 : not an integer */
    if (e < 0)
        goto not_an_integer;
    mant |= (uint64_t)1 << 52;
    if (e < 52) {
        shift = 52 - e;
        /* check that there is no fractional part */
        if (mant & (((uint64_t)1 << shift) - 1)) {
        not_an_integer:
            *pres = 1;
            return NULL;
        }
        mant >>= shift;
        e = 0;
    } else {
        e -= 52;
    }
    if (sgn)
        mant = -mant;
    /* the integer is mant*2^e */
    r = js_bigint_set_si64(&buf, (int64_t)mant);
    *pres = 0;
    return js_bigint_shl(ctx, r, e);
}
typedef enum JSToNumberHintEnum {
    TON_FLAG_NUMBER,
    TON_FLAG_NUMERIC,
} JSToNumberHintEnum;

static JSValue JS_ToNumberHintFree(JSContext *ctx, JSValue val,
                                   JSToNumberHintEnum flag)
{
    uint32_t tag;
    JSValue ret;

 redo:
    tag = JS_VALUE_GET_NORM_TAG(val);
    switch(tag) {
    case JS_TAG_BIG_INT:
    case JS_TAG_SHORT_BIG_INT:
        if (flag != TON_FLAG_NUMERIC) {
            JS_FreeValue(ctx, val);
            return JS_ThrowTypeError(ctx, "cannot convert bigint to number");
        }
        ret = val;
        break;
    case JS_TAG_FLOAT64:
    case JS_TAG_INT:
    case JS_TAG_EXCEPTION:
        ret = val;
        break;
    case JS_TAG_BOOL:
    case JS_TAG_NULL:
        ret = JS_NewInt32(ctx, JS_VALUE_GET_INT(val));
        break;
    case JS_TAG_UNDEFINED:
        ret = JS_NAN;
        break;
    case JS_TAG_OBJECT:
        val = JS_ToPrimitiveFree(ctx, val, HINT_NUMBER);
        if (JS_IsException(val))
            return JS_EXCEPTION;
        goto redo;
    case JS_TAG_STRING:
    case JS_TAG_STRING_ROPE:
        {
            const char *str;
            const char *p;
            size_t len;

            str = JS_ToCStringLen(ctx, &len, val);
            JS_FreeValue(ctx, val);
            if (!str)
                return JS_EXCEPTION;
            p = str;
            p += skip_spaces(p);
            if ((p - str) == len) {
                ret = JS_NewInt32(ctx, 0);
            } else {
                int flags = ATOD_ACCEPT_BIN_OCT;
                ret = js_atof(ctx, p, &p, 0, flags);
                if (!JS_IsException(ret)) {
                    p += skip_spaces(p);
                    if ((p - str) != len) {
                        JS_FreeValue(ctx, ret);
                        ret = JS_NAN;
                    }
                }
            }
            JS_FreeCString(ctx, str);
        }
        break;
    case JS_TAG_SYMBOL:
        JS_FreeValue(ctx, val);
        return JS_ThrowTypeError(ctx, "cannot convert symbol to number");
    default:
        JS_FreeValue(ctx, val);
        ret = JS_NAN;
        break;
    }
    return ret;
}

JSValue JS_ToNumberFree(JSContext *ctx, JSValue val)
{
    return JS_ToNumberHintFree(ctx, val, TON_FLAG_NUMBER);
}

JSValue JS_ToNumericFree(JSContext *ctx, JSValue val)
{
    return JS_ToNumberHintFree(ctx, val, TON_FLAG_NUMERIC);
}

JSValue JS_ToNumeric(JSContext *ctx, JSValueConst val)
{
    return JS_ToNumericFree(ctx, JS_DupValue(ctx, val));
}

__exception int __JS_ToFloat64Free(JSContext *ctx, double *pres,
                                          JSValue val)
{
    double d;
    uint32_t tag;
    
    val = JS_ToNumberFree(ctx, val);
    if (JS_IsException(val))
        goto fail;
    tag = JS_VALUE_GET_NORM_TAG(val);
    switch(tag) {
    case JS_TAG_INT:
        d = JS_VALUE_GET_INT(val);
        break;
    case JS_TAG_FLOAT64:
        d = JS_VALUE_GET_FLOAT64(val);
        break;
    default:
        abort();
    }
    *pres = d;
    return 0;
 fail:
    *pres = JS_FLOAT64_NAN;
    return -1;
}


int JS_ToFloat64(JSContext *ctx, double *pres, JSValueConst val)
{
    return JS_ToFloat64Free(ctx, pres, JS_DupValue(ctx, val));
}

JSValue JS_ToNumber(JSContext *ctx, JSValueConst val)
{
    return JS_ToNumberFree(ctx, JS_DupValue(ctx, val));
}

/* same as JS_ToNumber() but return 0 in case of NaN/Undefined */
__maybe_unused JSValue JS_ToIntegerFree(JSContext *ctx, JSValue val)
{
    uint32_t tag;
    JSValue ret;

 redo:
    tag = JS_VALUE_GET_NORM_TAG(val);
    switch(tag) {
    case JS_TAG_INT:
    case JS_TAG_BOOL:
    case JS_TAG_NULL:
    case JS_TAG_UNDEFINED:
        ret = JS_NewInt32(ctx, JS_VALUE_GET_INT(val));
        break;
    case JS_TAG_FLOAT64:
        {
            double d = JS_VALUE_GET_FLOAT64(val);
            if (isnan(d)) {
                ret = JS_NewInt32(ctx, 0);
            } else {
                /* convert -0 to +0 */
                d = trunc(d) + 0.0;
                ret = JS_NewFloat64(ctx, d);
            }
        }
        break;
    default:
        val = JS_ToNumberFree(ctx, val);
        if (JS_IsException(val))
            return val;
        goto redo;
    }
    return ret;
}

/* Note: the integer value is satured to 32 bits */
static int JS_ToInt32SatFree(JSContext *ctx, int *pres, JSValue val)
{
    uint32_t tag;
    int ret;

 redo:
    tag = JS_VALUE_GET_NORM_TAG(val);
    switch(tag) {
    case JS_TAG_INT:
    case JS_TAG_BOOL:
    case JS_TAG_NULL:
    case JS_TAG_UNDEFINED:
        ret = JS_VALUE_GET_INT(val);
        break;
    case JS_TAG_EXCEPTION:
        *pres = 0;
        return -1;
    case JS_TAG_FLOAT64:
        {
            double d = JS_VALUE_GET_FLOAT64(val);
            if (isnan(d)) {
                ret = 0;
            } else {
                if (d < INT32_MIN)
                    ret = INT32_MIN;
                else if (d > INT32_MAX)
                    ret = INT32_MAX;
                else
                    ret = (int)d;
            }
        }
        break;
    default:
        val = JS_ToNumberFree(ctx, val);
        if (JS_IsException(val)) {
            *pres = 0;
            return -1;
        }
        goto redo;
    }
    *pres = ret;
    return 0;
}

int JS_ToInt32Sat(JSContext *ctx, int *pres, JSValueConst val)
{
    return JS_ToInt32SatFree(ctx, pres, JS_DupValue(ctx, val));
}

int JS_ToInt32Clamp(JSContext *ctx, int *pres, JSValueConst val,
                    int min, int max, int min_offset)
{
    int res = JS_ToInt32SatFree(ctx, pres, JS_DupValue(ctx, val));
    if (res == 0) {
        if (*pres < min) {
            *pres += min_offset;
            if (*pres < min)
                *pres = min;
        } else {
            if (*pres > max)
                *pres = max;
        }
    }
    return res;
}

static int JS_ToInt64SatFree(JSContext *ctx, int64_t *pres, JSValue val)
{
    uint32_t tag;

 redo:
    tag = JS_VALUE_GET_NORM_TAG(val);
    switch(tag) {
    case JS_TAG_INT:
    case JS_TAG_BOOL:
    case JS_TAG_NULL:
    case JS_TAG_UNDEFINED:
        *pres = JS_VALUE_GET_INT(val);
        return 0;
    case JS_TAG_EXCEPTION:
        *pres = 0;
        return -1;
    case JS_TAG_FLOAT64:
        {
            double d = JS_VALUE_GET_FLOAT64(val);
            if (isnan(d)) {
                *pres = 0;
            } else {
                if (d < INT64_MIN)
                    *pres = INT64_MIN;
                else if (d >= 0x1p63) /* must use INT64_MAX + 1 because INT64_MAX cannot be exactly represented as a double */
                    *pres = INT64_MAX;
                else
                    *pres = (int64_t)d;
            }
        }
        return 0;
    default:
        val = JS_ToNumberFree(ctx, val);
        if (JS_IsException(val)) {
            *pres = 0;
            return -1;
        }
        goto redo;
    }
}

int JS_ToInt64Sat(JSContext *ctx, int64_t *pres, JSValueConst val)
{
    return JS_ToInt64SatFree(ctx, pres, JS_DupValue(ctx, val));
}

int JS_ToInt64Clamp(JSContext *ctx, int64_t *pres, JSValueConst val,
                    int64_t min, int64_t max, int64_t neg_offset)
{
    int res = JS_ToInt64SatFree(ctx, pres, JS_DupValue(ctx, val));
    if (res == 0) {
        if (*pres < 0)
            *pres += neg_offset;
        if (*pres < min)
            *pres = min;
        else if (*pres > max)
            *pres = max;
    }
    return res;
}

/* Same as JS_ToInt32Free() but with a 64 bit result. Return (<0, 0)
   in case of exception */
int JS_ToInt64Free(JSContext *ctx, int64_t *pres, JSValue val)
{
    uint32_t tag;
    int64_t ret;

 redo:
    tag = JS_VALUE_GET_NORM_TAG(val);
    switch(tag) {
    case JS_TAG_INT:
    case JS_TAG_BOOL:
    case JS_TAG_NULL:
    case JS_TAG_UNDEFINED:
        ret = JS_VALUE_GET_INT(val);
        break;
    case JS_TAG_FLOAT64:
        {
            JSFloat64Union u;
            double d;
            int e;
            d = JS_VALUE_GET_FLOAT64(val);
            u.d = d;
            /* we avoid doing fmod(x, 2^64) */
            e = (u.u64 >> 52) & 0x7ff;
            if (likely(e <= (1023 + 62))) {
                /* fast case */
                ret = (int64_t)d;
            } else if (e <= (1023 + 62 + 53)) {
                uint64_t v;
                /* remainder modulo 2^64 */
                v = (u.u64 & (((uint64_t)1 << 52) - 1)) | ((uint64_t)1 << 52);
                ret = v << ((e - 1023) - 52);
                /* take the sign into account */
                if (u.u64 >> 63)
                    ret = -ret;
            } else {
                ret = 0; /* also handles NaN and +inf */
            }
        }
        break;
    default:
        val = JS_ToNumberFree(ctx, val);
        if (JS_IsException(val)) {
            *pres = 0;
            return -1;
        }
        goto redo;
    }
    *pres = ret;
    return 0;
}

int JS_ToInt64(JSContext *ctx, int64_t *pres, JSValueConst val)
{
    return JS_ToInt64Free(ctx, pres, JS_DupValue(ctx, val));
}

int JS_ToInt64Ext(JSContext *ctx, int64_t *pres, JSValueConst val)
{
    if (JS_IsBigInt(ctx, val))
        return JS_ToBigInt64(ctx, pres, val);
    else
        return JS_ToInt64(ctx, pres, val);
}

/* return (<0, 0) in case of exception */
int JS_ToInt32Free(JSContext *ctx, int32_t *pres, JSValue val)
{
    uint32_t tag;
    int32_t ret;

 redo:
    tag = JS_VALUE_GET_NORM_TAG(val);
    switch(tag) {
    case JS_TAG_INT:
    case JS_TAG_BOOL:
    case JS_TAG_NULL:
    case JS_TAG_UNDEFINED:
        ret = JS_VALUE_GET_INT(val);
        break;
    case JS_TAG_FLOAT64:
        {
            JSFloat64Union u;
            double d;
            int e;
            d = JS_VALUE_GET_FLOAT64(val);
            u.d = d;
            /* we avoid doing fmod(x, 2^32) */
            e = (u.u64 >> 52) & 0x7ff;
            if (likely(e <= (1023 + 30))) {
                /* fast case */
                ret = (int32_t)d;
            } else if (e <= (1023 + 30 + 53)) {
                uint64_t v;
                /* remainder modulo 2^32 */
                v = (u.u64 & (((uint64_t)1 << 52) - 1)) | ((uint64_t)1 << 52);
                v = v << ((e - 1023) - 52 + 32);
                ret = v >> 32;
                /* take the sign into account */
                if (u.u64 >> 63)
                    ret = -ret;
            } else {
                ret = 0; /* also handles NaN and +inf */
            }
        }
        break;
    default:
        val = JS_ToNumberFree(ctx, val);
        if (JS_IsException(val)) {
            *pres = 0;
            return -1;
        }
        goto redo;
    }
    *pres = ret;
    return 0;
}

int JS_ToInt32(JSContext *ctx, int32_t *pres, JSValueConst val)
{
    return JS_ToInt32Free(ctx, pres, JS_DupValue(ctx, val));
}


int JS_ToUint8ClampFree(JSContext *ctx, int32_t *pres, JSValue val)
{
    uint32_t tag;
    int res;

 redo:
    tag = JS_VALUE_GET_NORM_TAG(val);
    switch(tag) {
    case JS_TAG_INT:
    case JS_TAG_BOOL:
    case JS_TAG_NULL:
    case JS_TAG_UNDEFINED:
        res = JS_VALUE_GET_INT(val);
        res = max_int(0, min_int(255, res));
        break;
    case JS_TAG_FLOAT64:
        {
            double d = JS_VALUE_GET_FLOAT64(val);
            if (isnan(d)) {
                res = 0;
            } else {
                if (d < 0)
                    res = 0;
                else if (d > 255)
                    res = 255;
                else
                    res = lrint(d);
            }
        }
        break;
    default:
        val = JS_ToNumberFree(ctx, val);
        if (JS_IsException(val)) {
            *pres = 0;
            return -1;
        }
        goto redo;
    }
    *pres = res;
    return 0;
}

__exception int JS_ToArrayLengthFree(JSContext *ctx, uint32_t *plen,
                                            JSValue val, BOOL is_array_ctor)
{
    uint32_t tag, len;

    tag = JS_VALUE_GET_TAG(val);
    switch(tag) {
    case JS_TAG_INT:
    case JS_TAG_BOOL:
    case JS_TAG_NULL:
        {
            int v;
            v = JS_VALUE_GET_INT(val);
            if (v < 0)
                goto fail;
            len = v;
        }
        break;
    default:
        if (JS_TAG_IS_FLOAT64(tag)) {
            double d;
            d = JS_VALUE_GET_FLOAT64(val);
            if (!(d >= 0 && d <= UINT32_MAX))
                goto fail;
            len = (uint32_t)d;
            if (len != d)
                goto fail;
        } else {
            uint32_t len1;

            if (is_array_ctor) {
                val = JS_ToNumberFree(ctx, val);
                if (JS_IsException(val))
                    return -1;
                /* cannot recurse because val is a number */
                if (JS_ToArrayLengthFree(ctx, &len, val, TRUE))
                    return -1;
            } else {
                /* legacy behavior: must do the conversion twice and compare */
                if (JS_ToUint32(ctx, &len, val)) {
                    JS_FreeValue(ctx, val);
                    return -1;
                }
                val = JS_ToNumberFree(ctx, val);
                if (JS_IsException(val))
                    return -1;
                /* cannot recurse because val is a number */
                if (JS_ToArrayLengthFree(ctx, &len1, val, FALSE))
                    return -1;
                if (len1 != len) {
                fail:
                    JS_ThrowRangeError(ctx, "invalid array length");
                    return -1;
                }
            }
        }
        break;
    }
    *plen = len;
    return 0;
}


BOOL is_safe_integer(double d)
{
    return isfinite(d) && floor(d) == d &&
        fabs(d) <= (double)MAX_SAFE_INTEGER;
}

int JS_ToIndex(JSContext *ctx, uint64_t *plen, JSValueConst val)
{
    int64_t v;
    if (JS_ToInt64Sat(ctx, &v, val))
        return -1;
    if (v < 0 || v > MAX_SAFE_INTEGER) {
        JS_ThrowRangeError(ctx, "invalid array index");
        *plen = 0;
        return -1;
    }
    *plen = v;
    return 0;
}

/* convert a value to a length between 0 and MAX_SAFE_INTEGER.
   return -1 for exception */
__exception int JS_ToLengthFree(JSContext *ctx, int64_t *plen,
                                       JSValue val)
{
    int res = JS_ToInt64Clamp(ctx, plen, val, 0, MAX_SAFE_INTEGER, 0);
    JS_FreeValue(ctx, val);
    return res;
}

/* Note: can return an exception */
int JS_NumberIsInteger(JSContext *ctx, JSValueConst val)
{
    double d;
    if (!JS_IsNumber(val))
        return FALSE;
    if (unlikely(JS_ToFloat64(ctx, &d, val)))
        return -1;
    return isfinite(d) && floor(d) == d;
}

BOOL JS_NumberIsNegativeOrMinusZero(JSContext *ctx, JSValueConst val)
{
    uint32_t tag;

    tag = JS_VALUE_GET_NORM_TAG(val);
    switch(tag) {
    case JS_TAG_INT:
        {
            int v;
            v = JS_VALUE_GET_INT(val);
            return (v < 0);
        }
    case JS_TAG_FLOAT64:
        {
            JSFloat64Union u;
            u.d = JS_VALUE_GET_FLOAT64(val);
            return (u.u64 >> 63);
        }
    case JS_TAG_SHORT_BIG_INT:
        return (JS_VALUE_GET_SHORT_BIG_INT(val) < 0);
    case JS_TAG_BIG_INT:
        {
            JSBigInt *p = JS_VALUE_GET_PTR(val);
            return js_bigint_sign(p);
        }
    default:
        return FALSE;
    }
}

JSValue js_bigint_to_string(JSContext *ctx, JSValueConst val)
{
    return js_bigint_to_string1(ctx, val, 10);
}

JSValue js_dtoa2(JSContext *ctx,
                        double d, int radix, int n_digits, int flags)
{
    char static_buf[128], *buf, *tmp_buf;
    int len, len_max;
    JSValue res;
    JSDTOATempMem dtoa_mem;
    len_max = js_dtoa_max_len(d, radix, n_digits, flags);
    
    /* longer buffer may be used if radix != 10 */
    if (len_max > sizeof(static_buf) - 1) {
        tmp_buf = js_malloc(ctx, len_max + 1);
        if (!tmp_buf)
            return JS_EXCEPTION;
        buf = tmp_buf;
    } else {
        tmp_buf = NULL;
        buf = static_buf;
    }
    len = js_dtoa(buf, d, radix, n_digits, flags, &dtoa_mem);
    res = js_new_string8_len(ctx, buf, len);
    js_free(ctx, tmp_buf);
    return res;
}

static JSValue JS_ToStringInternal(JSContext *ctx, JSValueConst val, BOOL is_ToPropertyKey)
{
    uint32_t tag;
    char buf[32];

    tag = JS_VALUE_GET_NORM_TAG(val);
    switch(tag) {
    case JS_TAG_STRING:
        return JS_DupValue(ctx, val);
    case JS_TAG_STRING_ROPE:
        return js_linearize_string_rope(ctx, JS_DupValue(ctx, val));
    case JS_TAG_INT:
        {
            size_t len;
            len = i32toa(buf, JS_VALUE_GET_INT(val));
            return js_new_string8_len(ctx, buf, len);
        }
        break;
    case JS_TAG_BOOL:
        return JS_AtomToString(ctx, JS_VALUE_GET_BOOL(val) ?
                          JS_ATOM_true : JS_ATOM_false);
    case JS_TAG_NULL:
        return JS_AtomToString(ctx, JS_ATOM_null);
    case JS_TAG_UNDEFINED:
        return JS_AtomToString(ctx, JS_ATOM_undefined);
    case JS_TAG_EXCEPTION:
        return JS_EXCEPTION;
    case JS_TAG_OBJECT:
        {
            JSValue val1, ret;
            val1 = JS_ToPrimitive(ctx, val, HINT_STRING);
            if (JS_IsException(val1))
                return val1;
            ret = JS_ToStringInternal(ctx, val1, is_ToPropertyKey);
            JS_FreeValue(ctx, val1);
            return ret;
        }
        break;
    case JS_TAG_FUNCTION_BYTECODE:
        return js_new_string8(ctx, "[function bytecode]");
    case JS_TAG_SYMBOL:
        if (is_ToPropertyKey) {
            return JS_DupValue(ctx, val);
        } else {
            return JS_ThrowTypeError(ctx, "cannot convert symbol to string");
        }
    case JS_TAG_FLOAT64:
        return js_dtoa2(ctx, JS_VALUE_GET_FLOAT64(val), 10, 0,
                        JS_DTOA_FORMAT_FREE);
    case JS_TAG_SHORT_BIG_INT:
    case JS_TAG_BIG_INT:
        return js_bigint_to_string(ctx, val);
    default:
        return js_new_string8(ctx, "[unsupported type]");
    }
}

JSValue JS_ToString(JSContext *ctx, JSValueConst val)
{
    return JS_ToStringInternal(ctx, val, FALSE);
}

JSValue JS_ToStringFree(JSContext *ctx, JSValue val)
{
    JSValue ret;
    ret = JS_ToString(ctx, val);
    JS_FreeValue(ctx, val);
    return ret;
}

JSValue JS_ToLocaleStringFree(JSContext *ctx, JSValue val)
{
    if (JS_IsUndefined(val) || JS_IsNull(val))
        return JS_ToStringFree(ctx, val);
    return JS_InvokeFree(ctx, val, JS_ATOM_toLocaleString, 0, NULL);
}

JSValue JS_ToPropertyKey(JSContext *ctx, JSValueConst val)
{
    return JS_ToStringInternal(ctx, val, TRUE);
}

JSValue JS_ToStringCheckObject(JSContext *ctx, JSValueConst val)
{
    uint32_t tag = JS_VALUE_GET_TAG(val);
    if (tag == JS_TAG_NULL || tag == JS_TAG_UNDEFINED)
        return JS_ThrowTypeError(ctx, "null or undefined are forbidden");
    return JS_ToString(ctx, val);
}

/* string.h — string.c 导出符号声明（自动生成自 quickjs.c） */
#ifndef QUICKJS_STRING_H
#define QUICKJS_STRING_H

BOOL JS_ConcatStringInPlace(JSContext *ctx, JSString *p1, JSValueConst op2);
BOOL js_string_eq(JSContext *ctx, const JSString *p1, const JSString *p2);
JSValue JS_ConcatString(JSContext *ctx, JSValue op1, JSValue op2);
JSValue JS_ConcatString3(JSContext *ctx, const char *str1, JSValue str2, const char *str3);
JSValue js_linearize_string_rope(JSContext *ctx, JSValue rope);
JSValue js_new_string16_len(JSContext *ctx, const uint16_t *buf, int len);
JSValue js_new_string8(JSContext *ctx, const char *buf);
JSValue js_new_string8_len(JSContext *ctx, const char *buf, int len);
JSValue js_new_string_char(JSContext *ctx, uint16_t c);
JSValue js_sub_string(JSContext *ctx, JSString *p, int start, int end);
JSValue string_buffer_end(StringBuffer *s);
int js_string_compare(JSContext *ctx, const JSString *p1, const JSString *p2);
int js_string_memcmp(const JSString *p1, int pos1, const JSString *p2, int pos2, int len);
int js_string_rope_compare(JSContext *ctx, JSValueConst op1, JSValueConst op2, BOOL eq_only);
int string_buffer_concat(StringBuffer *s, const JSString *p, uint32_t from, uint32_t to);
int string_buffer_concat_value(StringBuffer *s, JSValueConst v);
int string_buffer_concat_value_free(StringBuffer *s, JSValue v);
int string_buffer_fill(StringBuffer *s, int c, int count);
int string_buffer_init2(JSContext *ctx, StringBuffer *s, int size, int is_wide);
int string_buffer_putc16(StringBuffer *s, uint32_t c);
int string_buffer_putc8(StringBuffer *s, uint32_t c);
int string_buffer_putc_slow(StringBuffer *s, uint32_t c);
int string_buffer_puts8(StringBuffer *s, const char *str);
int string_buffer_write8(StringBuffer *s, const uint8_t *p, int len);
int string_getc(const JSString *p, int *pidx);
int string_rope_get(JSValueConst val, uint32_t idx);
void string_buffer_free(StringBuffer *s);

#endif /* QUICKJS_STRING_H */

/* runtime.h — runtime.c 导出符号声明（自动生成自 quickjs.c） */
#ifndef QUICKJS_RUNTIME_H
#define QUICKJS_RUNTIME_H

BOOL is_backtrace_needed(JSContext *ctx, JSValueConst obj);
JSString *js_alloc_string(JSContext *ctx, int max_len, int is_wide_char);
JSString *js_alloc_string_rt(JSRuntime *rt, int max_len, int is_wide_char);
JSValue JS_ThrowError(JSContext *ctx, JSErrorEnum error_num, const char *fmt, va_list ap);
JSValue JS_ThrowError2(JSContext *ctx, JSErrorEnum error_num, const char *fmt, va_list ap, BOOL add_backtrace);
JSValue JS_ThrowReferenceErrorNotDefined(JSContext *ctx, JSAtom name);
JSValue JS_ThrowReferenceErrorUninitialized(JSContext *ctx, JSAtom name);
JSValue JS_ThrowReferenceErrorUninitialized2(JSContext *ctx, JSFunctionBytecode *b, int idx, BOOL is_ref);
JSValue JS_ThrowStackOverflow(JSContext *ctx);
JSValue JS_ThrowTypeErrorInvalidClass(JSContext *ctx, int class_id);
JSValue JS_ThrowTypeErrorNotAConstructor(JSContext *ctx, JSValueConst func_obj);
JSValue JS_ThrowTypeErrorNotASymbol(JSContext *ctx);
JSValue JS_ThrowTypeErrorNotAnObject(JSContext *ctx);
JSValue __attribute__((format(printf, 3, 4))) __JS_ThrowSyntaxErrorAtom(JSContext *ctx, JSAtom atom, const char *fmt, ...);
JSValue __attribute__((format(printf, 3, 4))) __JS_ThrowTypeErrorAtom(JSContext *ctx, JSAtom atom, const char *fmt, ...);
const char *get_prop_string(JSContext *ctx, JSValueConst obj, JSAtom prop);
extern JSClassID js_class_id_alloc;
int JS_EnqueueJob2(JSContext *ctx, JSJobFunc *job_func, int argc, JSValueConst *argv, BOOL no_exception);
int JS_ThrowTypeErrorReadOnly(JSContext *ctx, int flags, JSAtom atom);
int __attribute__((format(printf, 3, 4))) JS_ThrowTypeErrorOrFalse(JSContext *ctx, int flags, const char *fmt, ...);
int find_line_num(JSContext *ctx, JSFunctionBytecode *b, uint32_t pc_value, int *pcol_num);
int get_leb128(uint32_t *pval, const uint8_t *buf, const uint8_t *buf_end);
int get_sleb128(int32_t *pval, const uint8_t *buf, const uint8_t *buf_end);
int init_class_range(JSRuntime *rt, JSClassShortDef const *tab, int start, int count);
no_inline __exception int __js_poll_interrupts(JSContext *ctx);
no_inline int js_realloc_array(JSContext *ctx, void **parray, int elem_size, int *psize, int req_size);
void *js_realloc_bytecode_rt(void *opaque, void *ptr, size_t size);
void JS_MarkContext(JSRuntime *rt, JSContext *ctx, JS_MarkFunc *mark_func);
void JS_SetImmutablePrototype(JSContext *ctx, JSValueConst obj);
void JS_ThrowInterrupted(JSContext *ctx);
void build_backtrace(JSContext *ctx, JSValueConst error_obj, const char *filename, int line_num, int col_num, int backtrace_flags);
void js_free_modules(JSContext *ctx, JSFreeModuleEnum flag);
void js_trigger_gc(JSRuntime *rt, size_t size);

#endif /* QUICKJS_RUNTIME_H */

/* builtin_typedarray.h — builtin_typedarray.c 导出符号声明（自动生成自 quickjs.c） */
#ifndef QUICKJS_BUILTIN_TYPEDARRAY_H
#define QUICKJS_BUILTIN_TYPEDARRAY_H

BOOL array_buffer_is_resizable(const JSArrayBuffer *abuf);
BOOL typed_array_is_oob(JSObject *p);
JSArrayBuffer *js_get_array_buffer(JSContext *ctx, JSValueConst obj);
JSValue JS_ThrowTypeErrorArrayBufferOOB(JSContext *ctx);
JSValue JS_ThrowTypeErrorDetachedArrayBuffer(JSContext *ctx);
JSValue js_array_buffer_constructor3(JSContext *ctx, JSValueConst new_target, uint64_t len, uint64_t *max_len, JSClassID class_id, uint8_t *buf, JSFreeArrayBufferDataFunc *free_func, void *opaque, BOOL alloc_flag);
JSValue js_typed_array___speciesCreate(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
JSValue js_typed_array_constructor(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, int classid);
extern uint8_t const typed_array_size_log2[JS_TYPED_ARRAY_COUNT];
int js_typed_array_get_length_unsafe(JSContext *ctx, JSValueConst obj);
void js_array_buffer_finalizer(JSRuntime *rt, JSValue val);
void js_array_buffer_free(JSRuntime *rt, void *opaque, void *ptr);
void js_typed_array_finalizer(JSRuntime *rt, JSValue val);
void js_typed_array_mark(JSRuntime *rt, JSValueConst val, JS_MarkFunc *mark_func);

#endif /* QUICKJS_BUILTIN_TYPEDARRAY_H */

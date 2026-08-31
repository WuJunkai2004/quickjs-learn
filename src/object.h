/* object.h — object.c 导出符号声明（自动生成自 quickjs.c） */
#ifndef QUICKJS_OBJECT_H
#define QUICKJS_OBJECT_H

BOOL js_class_has_bytecode(JSClassID class_id);
JSAutoInitIDEnum js_autoinit_get_id(JSProperty *pr);
JSContext *js_autoinit_get_realm(JSProperty *pr);
JSFunctionBytecode *JS_GetFunctionBytecode(JSValueConst val);
JSObject *get_proto_obj(JSValueConst proto_val);
JSShape *find_hashed_shape_prop(JSRuntime *rt, JSShape *sh, JSAtom atom, int prop_flags);
JSShape *js_clone_shape(JSContext *ctx, JSShape *sh1);
JSShape *js_dup_shape(JSShape *sh);
JSValue JS_NewCFunction3(JSContext *ctx, JSCFunction *func, const char *name, int length, JSCFunctionEnum cproto, int magic, JSValueConst proto_val, int n_fields);
JSValue JS_NewObjectFromShape(JSContext *ctx, JSShape *sh, JSClassID class_id, JSProperty *props);
JSValue JS_NewObjectProtoClassAlloc(JSContext *ctx, JSValueConst proto_val, JSClassID class_id, int n_alloc_props);
JSValue js_c_function_data_call(JSContext *ctx, JSValueConst func_obj, JSValueConst this_val, int argc, JSValueConst *argv, int flags);
JSValue js_get_function_name(JSContext *ctx, JSAtom name);
__maybe_unused void JS_DumpShapes(JSRuntime *rt);
int JS_SetObjectData(JSContext *ctx, JSValueConst obj, JSValue val);
int add_shape_property(JSContext *ctx, JSShape **psh, JSObject *p, JSAtom atom, int prop_flags);
int compact_properties(JSContext *ctx, JSObject *p);
int init_shape_hash(JSRuntime *rt);
int js_method_set_properties(JSContext *ctx, JSValueConst func_obj, JSAtom name, int flags, JSValueConst home_obj);
no_inline JSShape *js_new_shape2(JSContext *ctx, JSObject *proto, int hash_size, int prop_size);
no_inline int resize_properties(JSContext *ctx, JSShape **psh, JSObject *p, uint32_t count);
uint32_t shape_hash(uint32_t h, uint32_t val);
void free_property(JSRuntime *rt, JSProperty *pr, int prop_flags);
void js_autoinit_free(JSRuntime *rt, JSProperty *pr);
void js_autoinit_mark(JSRuntime *rt, JSProperty *pr, JS_MarkFunc *mark_func);
void js_c_function_data_finalizer(JSRuntime *rt, JSValue val);
void js_c_function_data_mark(JSRuntime *rt, JSValueConst val, JS_MarkFunc *mark_func);
void js_free_shape(JSRuntime *rt, JSShape *sh);
void js_free_shape_null(JSRuntime *rt, JSShape *sh);
void js_function_set_properties(JSContext *ctx, JSValueConst func_obj, JSAtom name, int len);
void js_method_set_home_object(JSContext *ctx, JSValueConst func_obj, JSValueConst home_obj);
void js_shape_hash_link(JSRuntime *rt, JSShape *sh);
void js_shape_hash_unlink(JSRuntime *rt, JSShape *sh);
void set_cycle_flag(JSContext *ctx, JSValueConst obj);

#endif /* QUICKJS_OBJECT_H */

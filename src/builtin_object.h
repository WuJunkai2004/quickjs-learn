/* builtin_object.h — builtin_object.c 导出符号声明（自动生成自 quickjs.c） */
#ifndef QUICKJS_BUILTIN_OBJECT_H
#define QUICKJS_BUILTIN_OBJECT_H

BOOL js_weakref_is_live(JSValueConst val);
BOOL js_weakref_is_target(JSValueConst val);
JSValue *build_arg_list(JSContext *ctx, uint32_t *plen, JSValueConst array_arg);
JSValue JS_SpeciesConstructor(JSContext *ctx, JSValueConst obj, JSValueConst defaultConstructor);
JSValue js_aggregate_error_constructor(JSContext *ctx, JSValueConst errors);
JSValue js_error_constructor(JSContext *ctx, JSValueConst new_target, int argc, JSValueConst *argv, int magic);
JSValue js_function_apply(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, int magic);
JSValue js_function_constructor(JSContext *ctx, JSValueConst new_target, int argc, JSValueConst *argv, int magic);
JSValue js_function_proto(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
JSValue js_map_constructor(JSContext *ctx, JSValueConst new_target, int argc, JSValueConst *argv, int magic);
JSValue js_object_constructor(JSContext *ctx, JSValueConst new_target, int argc, JSValueConst *argv);
JSValue js_object_keys(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, int kind);
JSValue js_object_seal(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, int freeze_flag);
JSValue js_object_toString(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
JSValue js_symbol_constructor(JSContext *ctx, JSValueConst new_target, int argc, JSValueConst *argv);
JSValue js_weakref_new(JSContext *ctx, JSValueConst val);
JSValue map_normalize_key(JSContext *ctx, JSValue key);
JSValueConst map_normalize_key_const(JSContext *ctx, JSValueConst key);
__exception int js_get_length32(JSContext *ctx, uint32_t *pres, JSValueConst obj);
__exception int js_get_length64(JSContext *ctx, int64_t *pres, JSValueConst obj);
extern const JSCFunctionListEntry js_error_funcs[1];
extern const JSCFunctionListEntry js_error_proto_funcs[3];
extern const JSCFunctionListEntry js_function_proto_funcs[8];
extern const JSCFunctionListEntry js_native_error_proto_funcs[];
extern const JSCFunctionListEntry js_object_funcs[23];
extern const JSCFunctionListEntry js_object_proto_funcs[11];
extern const JSCFunctionListEntry js_reflect_obj[1];
extern const JSCFunctionListEntry js_symbol_funcs[15];
extern const JSCFunctionListEntry js_symbol_proto_funcs[5];
int JS_CopySubArray(JSContext *ctx, JSValueConst obj, int64_t to_pos, int64_t from_pos, int64_t count, int dir);
int js_obj_to_desc(JSContext *ctx, JSPropertyDescriptor *d, JSValueConst desc);
void free_arg_list(JSContext *ctx, JSValue *tab, uint32_t len);
void js_weakref_free(JSRuntime *rt, JSValue val);

#endif /* QUICKJS_BUILTIN_OBJECT_H */

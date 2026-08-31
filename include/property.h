/* property.h — property.c 导出符号声明（自动生成自 quickjs.c） */
#ifndef QUICKJS_PROPERTY_H
#define QUICKJS_PROPERTY_H

BOOL JS_IsCFunction(JSContext *ctx, JSValueConst val, JSCFunction *func, int magic);
BOOL check_define_prop_flags(int prop_flags, int flags);
JSProperty *add_property(JSContext *ctx, JSObject *p, JSAtom prop, int prop_flags);
JSValue JS_GetPrivateField(JSContext *ctx, JSValueConst obj, JSValueConst name);
JSValue JS_GetPropertyInt64(JSContext *ctx, JSValueConst obj, int64_t idx);
JSValue JS_GetPropertyValue(JSContext *ctx, JSValueConst this_obj, JSValue prop);
JSValue JS_GetPrototypeFree(JSContext *ctx, JSValue obj);
JSValue JS_ThrowSyntaxErrorVarRedeclaration(JSContext *ctx, JSAtom prop);
JSValue js_allocate_fast_array(JSContext *ctx, int64_t len);
JSValue js_create_array(JSContext *ctx, int len, JSValueConst *tab);
JSValue js_create_array_free(JSContext *ctx, int len, JSValue *tab);
int JS_AddBrand(JSContext *ctx, JSValueConst obj, JSValueConst home_obj);
int JS_AutoInitProperty(JSContext *ctx, JSObject *p, JSAtom prop, JSProperty *pr, JSShapeProperty *prs);
int JS_CheckBrand(JSContext *ctx, JSValueConst obj, JSValueConst func);
int JS_CheckDefineGlobalVar(JSContext *ctx, JSAtom prop, int flags);
int JS_CreateDataPropertyUint32(JSContext *ctx, JSValueConst this_obj, int64_t idx, JSValue val, int flags);
int JS_DefineAutoInitProperty(JSContext *ctx, JSValueConst this_obj, JSAtom prop, JSAutoInitIDEnum id, void *opaque, int flags);
int JS_DefineObjectName(JSContext *ctx, JSValueConst obj, JSAtom name, int flags);
int JS_DefineObjectNameComputed(JSContext *ctx, JSValueConst obj, JSValueConst str, int flags);
int JS_DefinePrivateField(JSContext *ctx, JSValueConst obj, JSValueConst name, JSValue val);
int JS_DefinePropertyValueInt64(JSContext *ctx, JSValueConst this_obj, int64_t idx, JSValue val, int flags);
int JS_DefinePropertyValueValue(JSContext *ctx, JSValueConst this_obj, JSValue prop, JSValue val, int flags);
int JS_DeleteGlobalVar(JSContext *ctx, JSAtom prop);
int JS_DeletePropertyInt64(JSContext *ctx, JSValueConst obj, int64_t idx, int flags);
int JS_GetGlobalVarRef(JSContext *ctx, JSAtom prop, JSValue *sp);
int JS_GetOwnPropertyInternal(JSContext *ctx, JSPropertyDescriptor *desc, JSObject *p, JSAtom prop);
int JS_OrdinaryIsInstanceOf(JSContext *ctx, JSValueConst val, JSValueConst obj);
int JS_SetPrivateField(JSContext *ctx, JSValueConst obj, JSValueConst name, JSValue val);
int JS_SetPropertyValue(JSContext *ctx, JSValueConst this_obj, JSValue prop, JSValue val, int flags);
int JS_SetPrototypeInternal(JSContext *ctx, JSValueConst obj, JSValueConst proto_val, BOOL throw_flag);
int JS_TryGetPropertyInt64(JSContext *ctx, JSValueConst obj, int64_t idx, JSValue *pval);
int __exception JS_GetOwnPropertyNamesInternal(JSContext *ctx, JSPropertyEnum **ptab, uint32_t *plen, JSObject *p, int flags);
int delete_property(JSContext *ctx, JSObject *p, JSAtom atom);
int expand_fast_array(JSContext *ctx, JSObject *p, uint32_t new_len);
int js_update_property_flags(JSContext *ctx, JSObject *p, JSShapeProperty **pprs, int flags);
no_inline __exception int convert_fast_array_to_array(JSContext *ctx, JSObject *p);
uint32_t js_string_obj_get_length(JSContext *ctx, JSValueConst obj);
void js_free_desc(JSContext *ctx, JSPropertyDescriptor *desc);

#endif /* QUICKJS_PROPERTY_H */

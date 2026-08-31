/* serialize.h — serialize.c 导出符号声明（自动生成自 quickjs.c） */
#ifndef QUICKJS_SERIALIZE_H
#define QUICKJS_SERIALIZE_H

JSValue JS_InstantiateFunctionListItem2(JSContext *ctx, JSObject *p, JSAtom atom, void *opaque);
JSValue JS_NewCConstructor(JSContext *ctx, int class_id, const char *name, JSCFunction *func, int length, JSCFunctionEnum cproto, int magic, JSValueConst parent_ctor, const JSCFunctionListEntry *ctor_fields, int n_ctor_fields, const JSCFunctionListEntry *proto_fields, int n_proto_fields, int flags);
JSValue JS_NewObjectProtoList(JSContext *ctx, JSValueConst proto, const JSCFunctionListEntry *fields, int n_fields);
JSValue JS_ToObject(JSContext *ctx, JSValueConst val);
JSValue JS_ToObjectFree(JSContext *ctx, JSValue val);
JSValue js_global_eval(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
JSValue js_global_isFinite(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
JSValue js_global_isNaN(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
int JS_SetConstructor2(JSContext *ctx, JSValueConst func_obj, JSValueConst proto, int proto_flags, int ctor_flags);
int check_exception_free(JSContext *ctx, JSValue obj);
int check_function(JSContext *ctx, JSValueConst obj);
void __attribute__((format(printf, 2, 3))) bc_read_trace(BCReaderState *s, const char *fmt, ...);

#endif /* QUICKJS_SERIALIZE_H */

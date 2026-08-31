/* interp.h — interp.c 导出符号声明（自动生成自 quickjs.c） */
#ifndef QUICKJS_INTERP_H
#define QUICKJS_INTERP_H

BOOL js_get_fast_array(JSContext *ctx, JSValueConst obj, JSValue **arrpp, uint32_t *countp);
BOOL js_same_value(JSContext *ctx, JSValueConst op1, JSValueConst op2);
BOOL js_same_value_zero(JSContext *ctx, JSValueConst op1, JSValueConst op2);
BOOL js_strict_eq2(JSContext *ctx, JSValueConst op1, JSValueConst op2, JSStrictEqModeEnum eq_mode);
JSAsyncFunctionState *async_func_init(JSContext *ctx, JSValueConst func_obj, JSValueConst this_obj, int argc, JSValueConst *argv);
JSContext *JS_GetFunctionRealm(JSContext *ctx, JSValueConst func_obj);
JSValue JS_CallFree(JSContext *ctx, JSValue func_obj, JSValueConst this_obj, int argc, JSValueConst *argv);
JSValue JS_GetIterator(JSContext *ctx, JSValueConst obj, BOOL is_async);
JSValue JS_GetIterator2(JSContext *ctx, JSValueConst obj, JSValueConst method);
JSValue JS_InvokeFree(JSContext *ctx, JSValue this_val, JSAtom atom, int argc, JSValueConst *argv);
JSValue JS_IteratorGetCompleteValue(JSContext *ctx, JSValueConst obj, BOOL *pdone);
JSValue JS_IteratorNext(JSContext *ctx, JSValueConst enum_obj, JSValueConst method, int argc, JSValueConst *argv, BOOL *pdone);
JSValue JS_IteratorNext2(JSContext *ctx, JSValueConst enum_obj, JSValueConst method, int argc, JSValueConst *argv, int *pdone);
JSValue async_func_resume(JSContext *ctx, JSAsyncFunctionState *s);
JSValue js_call_bound_function(JSContext *ctx, JSValueConst func_obj, JSValueConst this_obj, int argc, JSValueConst *argv, int flags);
JSValue js_call_c_function(JSContext *ctx, JSValueConst func_obj, JSValueConst this_obj, int argc, JSValueConst *argv, int flags);
JSValue js_closure(JSContext *ctx, JSValue bfunc, JSVarRef **cur_var_refs, JSStackFrame *sf, BOOL is_eval);
JSValue js_closure2(JSContext *ctx, JSValue func_obj, JSFunctionBytecode *b, JSVarRef **cur_var_refs, JSStackFrame *sf, BOOL is_eval, JSModuleDef *m);
JSValue js_create_from_ctor(JSContext *ctx, JSValueConst ctor, int class_id);
JSValue js_create_iterator_result(JSContext *ctx, JSValue val, BOOL done);
JSValue js_function_proto_fileName(JSContext *ctx, JSValueConst this_val);
JSValue js_function_proto_lineNumber(JSContext *ctx, JSValueConst this_val, int is_col);
JSValue js_instantiate_prototype(JSContext *ctx, JSObject *p, JSAtom atom, void *opaque);
JSValue js_throw_type_error(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
JSValueConst JS_GetActiveFunction(JSContext *ctx);
JSVarRef *js_create_var_ref(JSContext *ctx, BOOL is_lexical);
JSVarRef *js_global_object_find_uninitialized_var(JSContext *ctx, JSObject *p, JSAtom atom, BOOL is_lexical);
__exception int JS_CopyDataProperties(JSContext *ctx, JSValueConst target, JSValueConst source, JSValueConst excluded, BOOL setprop);
extern const JSClassExoticMethods js_arguments_exotic_methods;
extern const uint16_t func_kind_to_class_id[4];
int JS_IteratorClose(JSContext *ctx, JSValueConst enum_obj, BOOL is_exception_pending);
no_inline __exception int js_eq_slow(JSContext *ctx, JSValue *sp, BOOL is_neq);
void __async_func_free(JSRuntime *rt, JSAsyncFunctionState *s);
void async_func_free(JSRuntime *rt, JSAsyncFunctionState *s);
void free_generator_stack_rt(JSRuntime *rt, JSGeneratorData *s);
void js_global_object_finalizer(JSRuntime *rt, JSValue obj);
void js_global_object_mark(JSRuntime *rt, JSValueConst val, JS_MarkFunc *mark_func);
void js_mapped_arguments_finalizer(JSRuntime *rt, JSValue val);
void js_mapped_arguments_mark(JSRuntime *rt, JSValueConst val, JS_MarkFunc *mark_func);

#endif /* QUICKJS_INTERP_H */

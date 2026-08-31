/* gc.h — gc.c 导出符号声明（自动生成自 quickjs.c） */
#ifndef QUICKJS_GC_H
#define QUICKJS_GC_H

void JS_RunGCInternal(JSRuntime *rt, BOOL remove_weak_objects);
void add_gc_object(JSRuntime *rt, JSGCObjectHeader *h, JSGCObjectTypeEnum type);
void free_var_ref(JSRuntime *rt, JSVarRef *var_ref);
void free_zero_refcount(JSRuntime *rt);
void gc_decref(JSRuntime *rt);
void js_array_finalizer(JSRuntime *rt, JSValue val);
void js_array_mark(JSRuntime *rt, JSValueConst val, JS_MarkFunc *mark_func);
void js_bound_function_finalizer(JSRuntime *rt, JSValue val);
void js_bound_function_mark(JSRuntime *rt, JSValueConst val, JS_MarkFunc *mark_func);
void js_bytecode_function_finalizer(JSRuntime *rt, JSValue val);
void js_bytecode_function_mark(JSRuntime *rt, JSValueConst val, JS_MarkFunc *mark_func);
void js_c_function_finalizer(JSRuntime *rt, JSValue val);
void js_c_function_mark(JSRuntime *rt, JSValueConst val, JS_MarkFunc *mark_func);
void js_for_in_iterator_finalizer(JSRuntime *rt, JSValue val);
void js_for_in_iterator_mark(JSRuntime *rt, JSValueConst val, JS_MarkFunc *mark_func);
void js_object_data_finalizer(JSRuntime *rt, JSValue val);
void js_object_data_mark(JSRuntime *rt, JSValueConst val, JS_MarkFunc *mark_func);
void remove_gc_object(JSGCObjectHeader *h);

#endif /* QUICKJS_GC_H */

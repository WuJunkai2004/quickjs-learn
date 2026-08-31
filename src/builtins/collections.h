/* collections.h — collections.c 导出符号声明（自动生成自 quickjs.c） */
#ifndef QUICKJS_BUILTINS_COLLECTIONS_H
#define QUICKJS_BUILTINS_COLLECTIONS_H

JSValue js_object_groupBy(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, int is_map);
void finrec_delete_weakref(JSRuntime *rt, JSWeakRefHeader *wh);
void js_map_finalizer(JSRuntime *rt, JSValue val);
void js_map_iterator_finalizer(JSRuntime *rt, JSValue val);
void js_map_iterator_mark(JSRuntime *rt, JSValueConst val, JS_MarkFunc *mark_func);
void js_map_mark(JSRuntime *rt, JSValueConst val, JS_MarkFunc *mark_func);
void map_delete_weakrefs(JSRuntime *rt, JSWeakRefHeader *wh);
void weakref_delete_weakref(JSRuntime *rt, JSWeakRefHeader *wh);

#endif /* QUICKJS_BUILTINS_COLLECTIONS_H */

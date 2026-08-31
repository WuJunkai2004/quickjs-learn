/* module.h — module.c 导出符号声明（自动生成自 quickjs.c） */
#ifndef QUICKJS_MODULE_H
#define QUICKJS_MODULE_H

JSExportEntry *add_export_entry(JSParseState *s, JSModuleDef *m, JSAtom local_name, JSAtom export_name, JSExportTypeEnum export_type);
JSModuleDef *js_new_module_def(JSContext *ctx, JSAtom name);
JSValue JS_NewModuleValue(JSContext *ctx, JSModuleDef *m);
JSValue js_dynamic_import(JSContext *ctx, JSValueConst specifier, JSValueConst options);
JSValue js_evaluate_module(JSContext *ctx, JSModuleDef *m);
JSValue js_import_meta(JSContext *ctx);
JSValue js_module_ns_autoinit(JSContext *ctx, JSObject *p, JSAtom atom, void *opaque);
__exception int js_parse_export(JSParseState *s);
__exception int js_parse_from_clause(JSParseState *s, JSModuleDef *m);
__exception int js_parse_with_clause(JSParseState *s, JSReqModuleEntry *rme);
extern const JSClassExoticMethods js_module_ns_exotic_methods;
int add_req_module_entry(JSContext *ctx, JSModuleDef *m, JSAtom module_name);
int js_create_module_function(JSContext *ctx, JSModuleDef *m);
int js_link_module(JSContext *ctx, JSModuleDef *m);
int js_resolve_module(JSContext *ctx, JSModuleDef *m);
void js_free_module_def(JSRuntime *rt, JSModuleDef *m);
void js_mark_module_def(JSRuntime *rt, JSModuleDef *m, JS_MarkFunc *mark_func);

#endif /* QUICKJS_MODULE_H */

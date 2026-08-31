/* dump.h — dump.c 导出符号声明（自动生成自 quickjs.c） */
#ifndef QUICKJS_DUMP_H
#define QUICKJS_DUMP_H

__maybe_unused void JS_DumpGCObject(JSRuntime *rt, JSGCObjectHeader *p);
__maybe_unused void JS_DumpObject(JSRuntime *rt, JSObject *p);
__maybe_unused void JS_DumpObjectHeader(JSRuntime *rt);
__maybe_unused void JS_DumpValue(JSContext *ctx, const char *str, JSValueConst val);
__maybe_unused void print_atom(JSContext *ctx, JSAtom atom);
void js_dump_value_write(void *opaque, const char *buf, size_t len);

#endif /* QUICKJS_DUMP_H */

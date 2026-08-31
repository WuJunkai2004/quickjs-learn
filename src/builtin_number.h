/* builtin_number.h — builtin_number.c 导出符号声明（自动生成自 quickjs.c） */
#ifndef QUICKJS_BUILTIN_NUMBER_H
#define QUICKJS_BUILTIN_NUMBER_H

JSValue js_boolean_constructor(JSContext *ctx, JSValueConst new_target, int argc, JSValueConst *argv);
JSValue js_parseFloat(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
JSValue js_parseInt(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
extern const JSCFunctionListEntry js_boolean_proto_funcs[2];
extern const JSCFunctionListEntry js_math_obj[1];
extern const JSCFunctionListEntry js_number_funcs[14];
extern const JSCFunctionListEntry js_number_proto_funcs[6];
int js_get_radix(JSContext *ctx, JSValueConst val);
void js_random_init(JSContext *ctx);

#endif /* QUICKJS_BUILTIN_NUMBER_H */

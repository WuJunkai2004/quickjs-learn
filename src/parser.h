/* parser.h — parser.c 导出符号声明（自动生成自 quickjs.c） */
#ifndef QUICKJS_PARSER_H
#define QUICKJS_PARSER_H

BOOL js_is_live_code(JSParseState *s);
BOOL token_is_ident(int tok);
JSAtom get_private_setter_name(JSContext *ctx, JSAtom name);
JSGlobalVar *add_global_var(JSContext *ctx, JSFunctionDef *s, JSAtom name);
JSGlobalVar *find_global_var(JSFunctionDef *fd, JSAtom name);
__attribute__((format(printf, 2, 3))) int js_parse_error(JSParseState *s, const char *fmt, ...);
__attribute__((format(printf, 3, 4))) int js_parse_error_pos(JSParseState *s, const uint8_t *ptr, const char *fmt, ...);
__exception int js_parse_assign_expr(JSParseState *s);
__exception int js_parse_class(JSParseState *s, BOOL is_class_expr, JSParseExportEnum export_flag);
__exception int js_parse_seek_token(JSParseState *s, const JSParsePos *sp);
__exception int js_parse_statement_or_decl(JSParseState *s, int decl_mask);
__exception int js_parse_var(JSParseState *s, int parse_flags, int tok, BOOL export_flag);
__exception int json_next_token(JSParseState *s);
__exception int next_token(JSParseState *s);
extern const JSOpCode opcode_info[OP_COUNT + (OP_TEMP_END - OP_TEMP_START)];
int add_arg(JSContext *ctx, JSFunctionDef *fd, JSAtom name);
int add_arguments_arg(JSContext *ctx, JSFunctionDef *fd);
int add_arguments_var(JSContext *ctx, JSFunctionDef *fd);
int add_func_var(JSContext *ctx, JSFunctionDef *fd, JSAtom name);
int add_var(JSContext *ctx, JSFunctionDef *fd, JSAtom name);
int cpool_add(JSParseState *s, JSValue val);
int define_var(JSParseState *s, JSFunctionDef *fd, JSAtom name, JSVarDefEnum var_def_type);
int emit_goto(JSParseState *s, int opcode, int label);
int emit_label(JSParseState *s, int label);
int find_lexical_decl(JSContext *ctx, JSFunctionDef *fd, JSAtom name, int scope_idx, BOOL check_catch_var);
int find_var(JSContext *ctx, JSFunctionDef *fd, JSAtom name);
int get_line_col_cached(GetLineColCache *s, int *pcol_num, const uint8_t *ptr);
int js_parse_check_duplicate_parameter(JSParseState *s, JSAtom name);
int js_parse_destructuring_element(JSParseState *s, int tok, int is_arg, int hasval, int has_ellipsis, BOOL allow_initializer, BOOL export_flag);
int js_parse_error_reserved_identifier(JSParseState *s);
int js_parse_expect(JSParseState *s, int tok);
int js_parse_expect_semi(JSParseState *s);
int js_parse_get_pos(JSParseState *s, JSParsePos *sp);
int js_parse_skip_parens_token(JSParseState *s, int *pbits, BOOL no_line_terminator);
int new_label(JSParseState *s);
int new_label_fd(JSFunctionDef *fd);
int peek_token(JSParseState *s, BOOL no_line_terminator);
int push_scope(JSParseState *s);
int update_label(JSFunctionDef *s, int label, int delta);
void emit_atom(JSParseState *s, JSAtom name);
void emit_class_field_init(JSParseState *s);
void emit_op(JSParseState *s, uint8_t val);
void emit_return(JSParseState *s, BOOL hasval);
void emit_u16(JSParseState *s, uint16_t val);
void emit_u32(JSParseState *s, uint32_t val);
void free_token(JSParseState *s, JSToken *token);
void reparse_ident_token(JSParseState *s);
void set_object_name(JSParseState *s, JSAtom name);
void skip_shebang(const uint8_t **pp, const uint8_t *buf_end);

#endif /* QUICKJS_PARSER_H */

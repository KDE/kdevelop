#ifndef INC_AdaTreeParserSuper_hpp_
#define INC_AdaTreeParserSuper_hpp_

#include <antlr/config.hpp>
#include "AdaTreeParserSuperTokenTypes.hpp"
/* $ANTLR 2.7.2: "ada.tree.g" -> "AdaTreeParserSuper.hpp"$ */
#include <antlr/TreeParser.hpp>

class AdaTreeParserSuper : public ANTLR_USE_NAMESPACE(antlr)TreeParser, public AdaTreeParserSuperTokenTypes
{
#line 1 "ada.tree.g"
#line 13 "AdaTreeParserSuper.hpp"
public:
	AdaTreeParserSuper();
	void initializeASTFactory( ANTLR_USE_NAMESPACE(antlr)ASTFactory& factory );
	int getNumTokens() const
	{
		return AdaTreeParserSuper::NUM_TOKENS;
	}
	const char* getTokenName( int type ) const
	{
		if( type > getNumTokens() ) return 0;
		return AdaTreeParserSuper::tokenNames[type];
	}
	public: void compilation_unit(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void context_items_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void library_item(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void subunit(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void pragma(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void pragma_arg(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void expression(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void with_clause(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void use_clause(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void compound_name(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void subtype_mark(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void attribute_id(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void modifiers(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void subprog_decl_or_rename_or_inst_or_body(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void def_id(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void pkg_body_part(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void generic_inst(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void pkg_spec_part(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void renames(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void generic_decl(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void subprog_decl(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void formal_part_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void def_designator(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void function_tail(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void value_s(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void value(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void ranged_expr_s(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void ranged_expr(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void simple_expression(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void range(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void range_constraint(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void range_dots(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void range_attrib_ref(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void prefix(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void parameter_specification(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void defining_identifier_list(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void init_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void name(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void definable_operator_symbol(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void parenthesized_primary(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void extension_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void spec_decl_part(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void basic_declarative_items_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void basic_decl_item(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void task_type_or_single_decl(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void prot_type_or_single_decl(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void decl_common(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void discrim_part_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void task_definition_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void task_items_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void private_task_items_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void discriminant_specifications(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void discriminant_specification(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void entrydecls_repspecs_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void entry_declaration(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void rep_spec(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void discrete_subtype_def_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void discrete_subtype_definition(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void subtype_ind(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void align_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void comp_loc_s(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void local_enum_name(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void enumeration_aggregate(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void protected_definition(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void prot_op_decl_s(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void prot_member_decl_s(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void prot_op_decl(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void comp_decl(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void component_subtype_def(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void enum_id_s(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void range_constraint_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void array_type_declaration(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void access_type_declaration(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void id_and_discrim(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void record_definition(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void array_type_definition(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void enumeration_literal_specification(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void index_or_discrete_range_s(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void index_or_discrete_range(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void constraint_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void digits_constraint(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void delta_constraint(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void index_constraint(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void discriminant_constraint(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void discrete_range(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void discriminant_association(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void selector_names_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void selector_name(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void component_list(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void component_items(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void variant_part(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void discriminant_direct_name(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void variant_s(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void variant(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void choice_s(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void choice(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void discrete_with_range(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void mark_with_constraint(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void generic_formal_part_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void generic_formal_parameter(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void formal_array_type_declaration(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void formal_access_type_declaration(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void id_part(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void subprogram_default_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void formal_package_actual_part_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void procedure_body(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void function_body(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void body_part(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void declarative_part(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void block_body(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void declarative_item(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void prot_op_bodies_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void block_body_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void handled_stmt_s(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void entry_body(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void subprog_decl_or_body(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void statements(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void except_handler_part_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void statement(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void def_label_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void null_stmt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void exit_stmt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void return_stmt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void goto_stmt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void delay_stmt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void abort_stmt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void raise_stmt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void requeue_stmt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void accept_stmt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void select_stmt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void if_stmt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void case_stmt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void loop_stmt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void block(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void call_or_assignment(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void cond_clause(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void elsifs_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void else_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void condition(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void alternative_s(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void case_statement_alternative(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void iteration_scheme_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void declare_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void label_name(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void entry_body_formal_part(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void entry_barrier(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void entry_index_spec_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void entry_call_stmt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void entry_index_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void triggering_alternative(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void abortable_part(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void selective_accept(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void entry_call_alternative(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void delay_alternative(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void stmts_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void guard_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void select_alternative(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void or_select_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void accept_alternative(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void exception_handler(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void identifier_colon_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void except_choice_s(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void exception_choice(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void operator_call(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void relation(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void range_or_mark(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void signed_term(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void term(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void factor(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void primary(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void name_or_qualified(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void allocator(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void subprogram_body(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void package_body(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void task_body(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void protected_body(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
private:
	static const char* tokenNames[];
#ifndef NO_STATIC_CONSTS
	static const int NUM_TOKENS = 305;
#else
	enum {
		NUM_TOKENS = 305
	};
#endif
	
	static const unsigned long _tokenSet_0_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_0;
	static const unsigned long _tokenSet_1_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_1;
	static const unsigned long _tokenSet_2_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_2;
	static const unsigned long _tokenSet_3_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_3;
	static const unsigned long _tokenSet_4_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_4;
};

#endif /*INC_AdaTreeParserSuper_hpp_*/

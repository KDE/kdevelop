#ifndef INC_AdaTreeParserSuper_hpp_
#define INC_AdaTreeParserSuper_hpp_

#include <antlr/config.hpp>
#include "AdaTreeParserSuperTokenTypes.hpp"
/* $ANTLR 2.7.2: "ada.tree.g" -> "AdaTreeParserSuper.hpp"$ */
#include <antlr/TreeParser.hpp>

class AdaTreeParserSuper : public antlr::TreeParser, public AdaTreeParserSuperTokenTypes
{
#line 1 "ada.tree.g"
#line 13 "AdaTreeParserSuper.hpp"
public:
	AdaTreeParserSuper();
	void initializeASTFactory( antlr::ASTFactory& factory );
	int getNumTokens() const
	{
		return AdaTreeParserSuper::NUM_TOKENS;
	}
	const char* getTokenName( int type ) const
	{
		if( type > getNumTokens() ) return 0;
		return AdaTreeParserSuper::tokenNames[type];
	}
	public: void compilation_unit(antlr::RefAST _t);
	public: void context_items_opt(antlr::RefAST _t);
	public: void library_item(antlr::RefAST _t);
	public: void subunit(antlr::RefAST _t);
	public: void pragma(antlr::RefAST _t);
	public: void pragma_arg(antlr::RefAST _t);
	public: void expression(antlr::RefAST _t);
	public: void with_clause(antlr::RefAST _t);
	public: void use_clause(antlr::RefAST _t);
	public: void compound_name(antlr::RefAST _t);
	public: void subtype_mark(antlr::RefAST _t);
	public: void attribute_id(antlr::RefAST _t);
	public: void modifiers(antlr::RefAST _t);
	public: void subprog_decl_or_rename_or_inst_or_body(antlr::RefAST _t);
	public: void def_id(antlr::RefAST _t);
	public: void pkg_body_part(antlr::RefAST _t);
	public: void generic_inst(antlr::RefAST _t);
	public: void pkg_spec_part(antlr::RefAST _t);
	public: void renames(antlr::RefAST _t);
	public: void generic_decl(antlr::RefAST _t);
	public: void subprog_decl(antlr::RefAST _t);
	public: void formal_part_opt(antlr::RefAST _t);
	public: void def_designator(antlr::RefAST _t);
	public: void function_tail(antlr::RefAST _t);
	public: void value_s(antlr::RefAST _t);
	public: void value(antlr::RefAST _t);
	public: void ranged_expr_s(antlr::RefAST _t);
	public: void ranged_expr(antlr::RefAST _t);
	public: void simple_expression(antlr::RefAST _t);
	public: void range(antlr::RefAST _t);
	public: void range_constraint(antlr::RefAST _t);
	public: void range_dots(antlr::RefAST _t);
	public: void range_attrib_ref(antlr::RefAST _t);
	public: void prefix(antlr::RefAST _t);
	public: void parameter_specification(antlr::RefAST _t);
	public: void defining_identifier_list(antlr::RefAST _t);
	public: void init_opt(antlr::RefAST _t);
	public: void name(antlr::RefAST _t);
	public: void definable_operator_symbol(antlr::RefAST _t);
	public: void parenthesized_primary(antlr::RefAST _t);
	public: void extension_opt(antlr::RefAST _t);
	public: void spec_decl_part(antlr::RefAST _t);
	public: void basic_declarative_items_opt(antlr::RefAST _t);
	public: void basic_decl_item(antlr::RefAST _t);
	public: void task_type_or_single_decl(antlr::RefAST _t);
	public: void prot_type_or_single_decl(antlr::RefAST _t);
	public: void decl_common(antlr::RefAST _t);
	public: void discrim_part_opt(antlr::RefAST _t);
	public: void task_definition_opt(antlr::RefAST _t);
	public: void task_items_opt(antlr::RefAST _t);
	public: void private_task_items_opt(antlr::RefAST _t);
	public: void discriminant_specifications(antlr::RefAST _t);
	public: void discriminant_specification(antlr::RefAST _t);
	public: void entrydecls_repspecs_opt(antlr::RefAST _t);
	public: void entry_declaration(antlr::RefAST _t);
	public: void rep_spec(antlr::RefAST _t);
	public: void discrete_subtype_def_opt(antlr::RefAST _t);
	public: void discrete_subtype_definition(antlr::RefAST _t);
	public: void subtype_ind(antlr::RefAST _t);
	public: void align_opt(antlr::RefAST _t);
	public: void comp_loc_s(antlr::RefAST _t);
	public: void local_enum_name(antlr::RefAST _t);
	public: void enumeration_aggregate(antlr::RefAST _t);
	public: void protected_definition(antlr::RefAST _t);
	public: void prot_op_decl_s(antlr::RefAST _t);
	public: void prot_member_decl_s(antlr::RefAST _t);
	public: void prot_op_decl(antlr::RefAST _t);
	public: void comp_decl(antlr::RefAST _t);
	public: void component_subtype_def(antlr::RefAST _t);
	public: void enum_id_s(antlr::RefAST _t);
	public: void range_constraint_opt(antlr::RefAST _t);
	public: void array_type_declaration(antlr::RefAST _t);
	public: void access_type_declaration(antlr::RefAST _t);
	public: void id_and_discrim(antlr::RefAST _t);
	public: void record_definition(antlr::RefAST _t);
	public: void array_type_definition(antlr::RefAST _t);
	public: void enumeration_literal_specification(antlr::RefAST _t);
	public: void index_or_discrete_range_s(antlr::RefAST _t);
	public: void index_or_discrete_range(antlr::RefAST _t);
	public: void constraint_opt(antlr::RefAST _t);
	public: void digits_constraint(antlr::RefAST _t);
	public: void delta_constraint(antlr::RefAST _t);
	public: void index_constraint(antlr::RefAST _t);
	public: void discriminant_constraint(antlr::RefAST _t);
	public: void discrete_range(antlr::RefAST _t);
	public: void discriminant_association(antlr::RefAST _t);
	public: void selector_names_opt(antlr::RefAST _t);
	public: void selector_name(antlr::RefAST _t);
	public: void component_list(antlr::RefAST _t);
	public: void component_items(antlr::RefAST _t);
	public: void variant_part(antlr::RefAST _t);
	public: void discriminant_direct_name(antlr::RefAST _t);
	public: void variant_s(antlr::RefAST _t);
	public: void variant(antlr::RefAST _t);
	public: void choice_s(antlr::RefAST _t);
	public: void choice(antlr::RefAST _t);
	public: void discrete_with_range(antlr::RefAST _t);
	public: void mark_with_constraint(antlr::RefAST _t);
	public: void generic_formal_part_opt(antlr::RefAST _t);
	public: void generic_formal_parameter(antlr::RefAST _t);
	public: void formal_array_type_declaration(antlr::RefAST _t);
	public: void formal_access_type_declaration(antlr::RefAST _t);
	public: void id_part(antlr::RefAST _t);
	public: void subprogram_default_opt(antlr::RefAST _t);
	public: void formal_package_actual_part_opt(antlr::RefAST _t);
	public: void procedure_body(antlr::RefAST _t);
	public: void function_body(antlr::RefAST _t);
	public: void body_part(antlr::RefAST _t);
	public: void declarative_part(antlr::RefAST _t);
	public: void block_body(antlr::RefAST _t);
	public: void declarative_item(antlr::RefAST _t);
	public: void prot_op_bodies_opt(antlr::RefAST _t);
	public: void block_body_opt(antlr::RefAST _t);
	public: void handled_stmt_s(antlr::RefAST _t);
	public: void entry_body(antlr::RefAST _t);
	public: void subprog_decl_or_body(antlr::RefAST _t);
	public: void statements(antlr::RefAST _t);
	public: void except_handler_part_opt(antlr::RefAST _t);
	public: void statement(antlr::RefAST _t);
	public: void def_label_opt(antlr::RefAST _t);
	public: void null_stmt(antlr::RefAST _t);
	public: void exit_stmt(antlr::RefAST _t);
	public: void return_stmt(antlr::RefAST _t);
	public: void goto_stmt(antlr::RefAST _t);
	public: void delay_stmt(antlr::RefAST _t);
	public: void abort_stmt(antlr::RefAST _t);
	public: void raise_stmt(antlr::RefAST _t);
	public: void requeue_stmt(antlr::RefAST _t);
	public: void accept_stmt(antlr::RefAST _t);
	public: void select_stmt(antlr::RefAST _t);
	public: void if_stmt(antlr::RefAST _t);
	public: void case_stmt(antlr::RefAST _t);
	public: void loop_stmt(antlr::RefAST _t);
	public: void block(antlr::RefAST _t);
	public: void call_or_assignment(antlr::RefAST _t);
	public: void cond_clause(antlr::RefAST _t);
	public: void elsifs_opt(antlr::RefAST _t);
	public: void else_opt(antlr::RefAST _t);
	public: void condition(antlr::RefAST _t);
	public: void alternative_s(antlr::RefAST _t);
	public: void case_statement_alternative(antlr::RefAST _t);
	public: void iteration_scheme_opt(antlr::RefAST _t);
	public: void declare_opt(antlr::RefAST _t);
	public: void label_name(antlr::RefAST _t);
	public: void entry_body_formal_part(antlr::RefAST _t);
	public: void entry_barrier(antlr::RefAST _t);
	public: void entry_index_spec_opt(antlr::RefAST _t);
	public: void entry_call_stmt(antlr::RefAST _t);
	public: void entry_index_opt(antlr::RefAST _t);
	public: void triggering_alternative(antlr::RefAST _t);
	public: void abortable_part(antlr::RefAST _t);
	public: void selective_accept(antlr::RefAST _t);
	public: void entry_call_alternative(antlr::RefAST _t);
	public: void delay_alternative(antlr::RefAST _t);
	public: void stmts_opt(antlr::RefAST _t);
	public: void guard_opt(antlr::RefAST _t);
	public: void select_alternative(antlr::RefAST _t);
	public: void or_select_opt(antlr::RefAST _t);
	public: void accept_alternative(antlr::RefAST _t);
	public: void exception_handler(antlr::RefAST _t);
	public: void identifier_colon_opt(antlr::RefAST _t);
	public: void except_choice_s(antlr::RefAST _t);
	public: void exception_choice(antlr::RefAST _t);
	public: void operator_call(antlr::RefAST _t);
	public: void relation(antlr::RefAST _t);
	public: void range_or_mark(antlr::RefAST _t);
	public: void signed_term(antlr::RefAST _t);
	public: void term(antlr::RefAST _t);
	public: void factor(antlr::RefAST _t);
	public: void primary(antlr::RefAST _t);
	public: void name_or_qualified(antlr::RefAST _t);
	public: void allocator(antlr::RefAST _t);
	public: void subprogram_body(antlr::RefAST _t);
	public: void package_body(antlr::RefAST _t);
	public: void task_body(antlr::RefAST _t);
	public: void protected_body(antlr::RefAST _t);
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
	static const antlr::BitSet _tokenSet_0;
	static const unsigned long _tokenSet_1_data_[];
	static const antlr::BitSet _tokenSet_1;
	static const unsigned long _tokenSet_2_data_[];
	static const antlr::BitSet _tokenSet_2;
	static const unsigned long _tokenSet_3_data_[];
	static const antlr::BitSet _tokenSet_3;
	static const unsigned long _tokenSet_4_data_[];
	static const antlr::BitSet _tokenSet_4;
};

#endif /*INC_AdaTreeParserSuper_hpp_*/

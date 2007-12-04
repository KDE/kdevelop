#ifndef INC_AdaParser_hpp_
#define INC_AdaParser_hpp_

#line 29 "ada.g"

#include <antlr/SemanticException.hpp>  // antlr wants this
#include "AdaAST.hpp"
#include "preambles.h"

#line 11 "AdaParser.hpp"
#include <antlr/config.hpp>
/* $ANTLR 2.7.7 (20070609): "ada.g" -> "AdaParser.hpp"$ */
#include <antlr/TokenStream.hpp>
#include <antlr/TokenBuffer.hpp>
#include "AdaTokenTypes.hpp"
#include <antlr/LLkParser.hpp>

class CUSTOM_API AdaParser : public ANTLR_USE_NAMESPACE(antlr)LLkParser, public AdaTokenTypes
{
#line 53 "ada.g"

  ANTLR_PARSER_PREAMBLE

public:
  // Ada support stuff
  void push_def_id (const RefAdaAST& defid);
  const RefAdaAST& pop_def_id ();
  bool end_id_matches_def_id (const RefAdaAST& endid);
  bool definable_operator (const char *string);  // operator_symbol sans "/="
  bool is_operator_symbol (const char *string);
#line 22 "AdaParser.hpp"
public:
	void initializeASTFactory( ANTLR_USE_NAMESPACE(antlr)ASTFactory& factory );
protected:
	AdaParser(ANTLR_USE_NAMESPACE(antlr)TokenBuffer& tokenBuf, int k);
public:
	AdaParser(ANTLR_USE_NAMESPACE(antlr)TokenBuffer& tokenBuf);
protected:
	AdaParser(ANTLR_USE_NAMESPACE(antlr)TokenStream& lexer, int k);
public:
	AdaParser(ANTLR_USE_NAMESPACE(antlr)TokenStream& lexer);
	AdaParser(const ANTLR_USE_NAMESPACE(antlr)ParserSharedInputState& state);
	int getNumTokens() const
	{
		return AdaParser::NUM_TOKENS;
	}
	const char* getTokenName( int type ) const
	{
		if( type > getNumTokens() ) return 0;
		return AdaParser::tokenNames[type];
	}
	const char* const* getTokenNames() const
	{
		return AdaParser::tokenNames;
	}
	public: void compilation_unit();
	public: void context_items_opt();
	public: void library_item();
	public: void subunit();
	public: void pragma();
	public: void pragma_args_opt();
	public: void pragma_arg();
	public: void expression();
	public: void with_clause();
	public: void use_clause();
	public: void c_name_list();
	public: void compound_name();
	public: void subtype_mark();
	public: void attribute_id();
	public: void private_opt();
	public: void lib_pkg_spec_or_body();
	public: void subprog_decl_or_rename_or_inst_or_body(
		boolean lib_level
	);
	public: void generic_decl(
		boolean lib_level
	);
	public: void def_id(
		boolean lib_level
	);
	public: void pkg_body_part();
	public: void spec_decl_part(
		RefAdaAST pkg
	);
	public: void subprog_decl(
		boolean lib_level
	);
	public: void generic_subp_inst();
	public: void formal_part_opt();
	public: void renames();
	public: void is_separate_or_abstract_or_decl(
		RefAdaAST t
	);
	public: void def_designator(
		boolean lib_level
	);
	public: void function_tail();
	public: void generic_inst();
	public: void value_s();
	public: void parenth_values();
	public: void value();
	public: void ranged_expr_s();
	public: void ranged_expr();
	public: void simple_expression();
	public: void range();
	public: void range_constraint();
	public: void range_dots();
	public: void range_attrib_ref();
	public: void prefix();
	public: void parameter_specification();
	public: void def_ids_colon();
	public: void mode_opt();
	public: void init_opt();
	public: void defining_identifier_list();
	public: void name();
	public: RefAdaAST  definable_operator_symbol();
	public: RefAdaAST  is_operator();
	public: void parenthesized_primary();
	public: void extension_opt();
	public: void separate_or_abstract(
		RefAdaAST t
	);
	public: RefAdaAST  designator();
	public: void func_formal_part_opt();
	public: void func_param();
	public: void in_access_opt();
	public: void pkg_spec_part();
	public: void basic_declarative_items_opt();
	public: void private_declarative_items_opt();
	public: void end_id_opt();
	public: void basic_decl_item();
	public: void basic_declarative_items();
	public: void task_type_or_single_decl(
		RefAdaAST tsk
	);
	public: void prot_type_or_single_decl(
		RefAdaAST pro
	);
	public: void decl_common();
	public: void discrim_part_opt();
	public: void task_definition_opt();
	public: void task_items_opt();
	public: void private_task_items_opt();
	public: void discrim_part_text();
	public: void discriminant_specifications();
	public: void known_discrim_part();
	public: void empty_discrim_opt();
	public: void discrim_part();
	public: void discriminant_specification();
	public: void access_opt();
	public: void entrydecls_repspecs_opt();
	public: void entry_declaration();
	public: void rep_spec();
	public: void discrete_subtype_def_opt();
	public: void discrete_subtype_definition();
	public: void subtype_ind();
	public: void rep_spec_part(
		RefAdaAST t
	);
	public: void align_opt();
	public: void comp_loc_s();
	public: void protected_definition();
	public: void prot_private_opt();
	public: void prot_op_decl();
	public: void comp_decl();
	public: void prot_op_decl_s();
	public: void prot_member_decl_s();
	public: void component_subtype_def();
	public: void type_def(
		RefAdaAST t
	);
	public: void derived_or_private_or_record(
		RefAdaAST t, boolean has_discrim
	);
	public: void local_enum_name();
	public: void enumeration_aggregate();
	public: void aliased_constant_opt();
	public: void array_type_definition(
		RefAdaAST t
	);
	public: void enum_id_s();
	public: void range_constraint_opt();
	public: void access_type_definition(
		RefAdaAST t
	);
	public: void enumeration_literal_specification();
	public: void index_or_discrete_range_s();
	public: void index_or_discrete_range();
	public: void aliased_opt();
	public: void constraint_opt();
	public: void digits_constraint();
	public: void delta_constraint();
	public: void index_constraint();
	public: void discriminant_constraint();
	public: void discrete_range();
	public: void discriminant_association();
	public: void selector_names_opt();
	public: void association_head();
	public: void selector_name();
	public: void protected_opt();
	public: void constant_all_opt();
	public: void abstract_opt();
	public: void record_definition(
		boolean has_discrim
	);
	public: void abstract_tagged_limited_opt();
	public: void component_list(
		boolean has_discrim
	);
	public: void component_items();
	public: void variant_part();
	public: void empty_component_items();
	public: void discriminant_direct_name();
	public: void variant_s();
	public: void variant();
	public: void choice_s();
	public: void choice();
	public: void discrete_with_range();
	public: void mark_with_constraint();
	public: void generic_formal_part_opt();
	public: void generic_formal_parameter();
	public: void discriminable_type_definition(
		RefAdaAST t
	);
	public: void subprogram_default_opt();
	public: void formal_package_actual_part_opt();
	public: void body_part();
	public: void declarative_part();
	public: void block_body();
	public: void declarative_item();
	public: void body_is();
	public: void separate();
	public: void prot_op_bodies_opt();
	public: void block_body_opt();
	public: void handled_stmt_s();
	public: void entry_body();
	public: void subprog_decl_or_body();
	public: void statements();
	public: void except_handler_part_opt();
	public: void handled_stmts_opt();
	public: void statement();
	public: void def_label_opt();
	public: void null_stmt();
	public: void exit_stmt();
	public: void return_stmt();
	public: void goto_stmt();
	public: void delay_stmt();
	public: void abort_stmt();
	public: void raise_stmt();
	public: void requeue_stmt();
	public: void accept_stmt();
	public: void select_stmt();
	public: void if_stmt();
	public: void case_stmt();
	public: void loop_stmt();
	public: void block();
	public: void statement_identifier();
	public: void id_opt();
	public: void call_or_assignment();
	public: void cond_clause();
	public: void elsifs_opt();
	public: void else_opt();
	public: void condition();
	public: void alternative_s();
	public: void case_statement_alternative();
	public: void iteration_scheme_opt();
	public: void reverse_opt();
	public: void id_opt_aux();
	public: void declare_opt();
	public: void label_name();
	public: void entry_body_formal_part();
	public: void entry_barrier();
	public: void entry_index_spec_opt();
	public: void entry_call_stmt();
	public: void entry_index_opt();
	public: void until_opt();
	public: void triggering_alternative();
	public: void abortable_part();
	public: void selective_accept();
	public: void entry_call_alternative();
	public: void delay_alternative();
	public: void stmts_opt();
	public: void guard_opt();
	public: void select_alternative();
	public: void or_select_opt();
	public: void accept_alternative();
	public: void exception_handler();
	public: void identifier_colon_opt();
	public: void except_choice_s();
	public: void exception_choice();
	public: void operator_call();
	public: void operator_call_tail(
		RefAdaAST opstr
	);
	public: void relation();
	public: void range_or_mark();
	public: void signed_term();
	public: void term();
	public: void factor();
	public: void primary();
	public: void name_or_qualified();
	public: void allocator();
	public: void subprogram_body();
	public: void package_body();
	public: void task_body();
	public: void protected_body();
public:
	ANTLR_USE_NAMESPACE(antlr)RefAST getAST()
	{
		return ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST);
	}
	
protected:
	RefAdaAST returnAST;
private:
	static const char* tokenNames[];
#ifndef NO_STATIC_CONSTS
	static const int NUM_TOKENS = 290;
#else
	enum {
		NUM_TOKENS = 290
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
	static const unsigned long _tokenSet_5_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_5;
	static const unsigned long _tokenSet_6_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_6;
	static const unsigned long _tokenSet_7_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_7;
	static const unsigned long _tokenSet_8_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_8;
	static const unsigned long _tokenSet_9_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_9;
	static const unsigned long _tokenSet_10_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_10;
	static const unsigned long _tokenSet_11_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_11;
	static const unsigned long _tokenSet_12_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_12;
	static const unsigned long _tokenSet_13_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_13;
	static const unsigned long _tokenSet_14_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_14;
	static const unsigned long _tokenSet_15_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_15;
	static const unsigned long _tokenSet_16_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_16;
	static const unsigned long _tokenSet_17_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_17;
	static const unsigned long _tokenSet_18_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_18;
	static const unsigned long _tokenSet_19_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_19;
	static const unsigned long _tokenSet_20_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_20;
	static const unsigned long _tokenSet_21_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_21;
	static const unsigned long _tokenSet_22_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_22;
	static const unsigned long _tokenSet_23_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_23;
	static const unsigned long _tokenSet_24_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_24;
	static const unsigned long _tokenSet_25_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_25;
	static const unsigned long _tokenSet_26_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_26;
	static const unsigned long _tokenSet_27_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_27;
	static const unsigned long _tokenSet_28_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_28;
	static const unsigned long _tokenSet_29_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_29;
	static const unsigned long _tokenSet_30_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_30;
};

#endif /*INC_AdaParser_hpp_*/

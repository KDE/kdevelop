/*
 * ANTLR Ada95 tree super grammar (base class)
 *
 * Oliver M. Kellogg  <okellogg@users.sourceforge.net>
 *
 * $Id$
 *
 */


options {
  language="Cpp";
}

class AdaTreeParserSuper extends TreeParser;
options {
  importVocab=Ada;
}

// Compilation Unit:  This is the start rule.

compilation_unit
	: context_items_opt ( library_item | subunit ) ( pragma )*
	;

pragma  : #(PRAGMA IDENTIFIER ( pragma_arg )* )
	;

pragma_arg
	: #(RIGHT_SHAFT IDENTIFIER expression)
	| expression
	;

context_items_opt
	: #(CONTEXT_CLAUSE ( pragma | with_clause | use_clause )* )
	;

with_clause
	: #(WITH_CLAUSE ( compound_name )+ )
	;

compound_name
	: IDENTIFIER
	| #(DOT compound_name IDENTIFIER)
	;

use_clause
	: #(USE_TYPE_CLAUSE ( subtype_mark )+ )
	| #(USE_CLAUSE ( compound_name )+ )
	;

subtype_mark
	: compound_name
	| #(TIC compound_name attribute_id)
	;

attribute_id : RANGE
	| DIGITS
	| DELTA
	| ACCESS
	| IDENTIFIER
	;

library_item :
	#(LIBRARY_ITEM modifiers
		( subprog_decl_or_rename_or_inst_or_body
		| #(PACKAGE_BODY def_id pkg_body_part)
		| #(GENERIC_PACKAGE_INSTANTIATION def_id generic_inst)
		| #(PACKAGE_SPECIFICATION def_id pkg_spec_part)
		| #(PACKAGE_RENAMING_DECLARATION def_id renames)
		| generic_decl
		)
	)
	;

modifiers : #(MODIFIERS
		( ABSTRACT
		| ACCESS
		| ALIASED
		| ALL
		| CONSTANT
		| IN
                | LIMITED
		| OUT
		| PRIVATE
		| PROTECTED
		| REVERSE
		| TAGGED
		)*
		// We can afford to be this loose because the parser
		// has already fended off illegal usages, i.e. we can
		// be sure that we are only dealing with permissible
		// values for each usage context here.
		// Only exception: PRIVATE as the prefix of private
		// {generic|package|subprogram} library-level decl.
		// The parser has let this in for bodies as well
		// (which is not legal), so we need a semantic check.
	     )
	;

subprog_decl
	: #(GENERIC_PROCEDURE_INSTANTIATION def_id generic_inst)
	| #(PROCEDURE_RENAMING_DECLARATION  def_id formal_part_opt renames)
	| #(PROCEDURE_DECLARATION           def_id formal_part_opt)
	| #(PROCEDURE_BODY_STUB             def_id formal_part_opt)
	| #(ABSTRACT_PROCEDURE_DECLARATION  def_id formal_part_opt)
	| #(GENERIC_FUNCTION_INSTANTIATION  def_designator generic_inst)
	| #(FUNCTION_RENAMING_DECLARATION   def_designator function_tail renames)
	| #(FUNCTION_DECLARATION            def_designator function_tail)
	| #(FUNCTION_BODY_STUB              def_designator function_tail)
	| #(ABSTRACT_FUNCTION_DECLARATION   def_id function_tail)
	;

def_id : compound_name  // Can afford looseness because parser is tight.
	;

generic_inst : compound_name ( value_s )?
	;

value   : #(OTHERS expression)
	| #(RIGHT_SHAFT ranged_expr_s expression)
	| ranged_expr_s
	;

ranged_expr_s
	: #(PIPE ranged_expr_s ranged_expr)
	| ranged_expr
	;

ranged_expr
	: #(DOT_DOT expression simple_expression)
	| #(RANGE expression range)
	| expression
	;

range_constraint : range
	;

range : ( range_dots
	| range_attrib_ref
	)
	;

range_dots : #(DOT_DOT simple_expression simple_expression)
	;

range_attrib_ref : #(RANGE_ATTRIBUTE_REFERENCE prefix ( expression )? )
	;

prefix  : IDENTIFIER
	| #(DOT prefix ( ALL | IDENTIFIER ) )
	| #(INDEXED_COMPONENT prefix value_s )
	;

formal_part_opt : #(FORMAL_PART_OPT ( parameter_specification )* )
	;

parameter_specification
	: #(PARAMETER_SPECIFICATION defining_identifier_list modifiers
		subtype_mark init_opt)
	;

defining_identifier_list : #(DEFINING_IDENTIFIER_LIST ( IDENTIFIER )+ )
	;

renames : CHARACTER_STRING  // CHARACTER_STRING should not really be there.
	| OPERATOR_SYMBOL   // OPERATOR_SYMBOL should be used instead.
	| name
	;

name    : IDENTIFIER
	| #(DOT name
		( ALL
		| IDENTIFIER
		| CHARACTER_LITERAL
		| OPERATOR_SYMBOL
		)
	   )
	| #(INDEXED_COMPONENT name value_s)
	| #(TIC name attribute_id)
	;

definable_operator_symbol : OPERATOR_SYMBOL
	;

parenthesized_primary
	: #(PARENTHESIZED_PRIMARY ( NuLL | value_s extension_opt ) )
	;

extension_opt :  #(EXTENSION_OPT ( NuLL | value_s )? )
	;

def_designator
	: compound_name
	| definable_operator_symbol
	;

function_tail : formal_part_opt subtype_mark
	;

spec_decl_part
	: #(GENERIC_PACKAGE_INSTANTIATION def_id generic_inst)
	| #(PACKAGE_SPECIFICATION def_id pkg_spec_part)
	| #(PACKAGE_RENAMING_DECLARATION def_id renames)
	;

pkg_spec_part
	: basic_declarative_items_opt
	  ( basic_declarative_items_opt )?   // optional private part
	;

basic_declarative_items_opt
	: #(BASIC_DECLARATIVE_ITEMS_OPT ( basic_decl_item )* )
	;

basic_decl_item
	: spec_decl_part
	| task_type_or_single_decl
	| prot_type_or_single_decl
	| subprog_decl
	| decl_common
	;

task_type_or_single_decl
	: #(TASK_TYPE_DECLARATION def_id discrim_part_opt task_definition_opt)
	| #(SINGLE_TASK_DECLARATION def_id task_definition_opt)
	;

task_definition_opt
	: ( task_items_opt private_task_items_opt )?
	;

discrim_part_opt
	: #(DISCRIM_PART_OPT ( BOX | discriminant_specifications )? )
	;

discriminant_specifications
	: #(DISCRIMINANT_SPECIFICATIONS ( discriminant_specification )* )
	;

discriminant_specification
	: #(DISCRIMINANT_SPECIFICATION defining_identifier_list
		modifiers subtype_mark init_opt)
	;

init_opt : #(INIT_OPT ( expression )? )
	;  // `expression' is of course much too loose;
	   // semantic checks are required in the usage contexts.

task_items_opt
	: #(TASK_ITEMS_OPT entrydecls_repspecs_opt)
	;

entrydecls_repspecs_opt
	: ( entry_declaration | pragma | rep_spec )*
	;

entry_declaration
	: #(ENTRY_DECLARATION IDENTIFIER
		discrete_subtype_def_opt formal_part_opt)
	;

discrete_subtype_def_opt
	: #(DISCRETE_SUBTYPE_DEF_OPT ( discrete_subtype_definition )? )
	;

discrete_subtype_definition  // TBD
	: range
	| subtype_ind
	;

rep_spec : #(RECORD_REPRESENTATION_CLAUSE subtype_mark align_opt comp_loc_s)
	| #(AT_CLAUSE subtype_mark expression)
	| #(ATTRIBUTE_DEFINITION_CLAUSE subtype_mark expression)
	        // attrib_def. Semantic check must ensure that the
		// subtype_mark contains an attribute reference.
	| #(ENUMERATION_REPESENTATION_CLAUSE local_enum_name enumeration_aggregate)
	;

align_opt : #(MOD_CLAUSE_OPT ( expression )? )
	;

comp_loc_s
	: #(COMPONENT_CLAUSES_OPT
		( pragma
		| subtype_mark expression range
		)*
	   )
	;

private_task_items_opt
	: #(PRIVATE_TASK_ITEMS_OPT entrydecls_repspecs_opt )
	;

prot_type_or_single_decl
	: #(PROTECTED_TYPE_DECLARATION def_id discrim_part_opt protected_definition)
	| #(SINGLE_PROTECTED_DECLARATION def_id protected_definition)
	;

protected_definition
	: prot_op_decl_s ( prot_member_decl_s )?
	;

prot_op_decl_s
	: #(PROT_OP_DECLARATIONS ( prot_op_decl )* )
	;

prot_op_decl
	: entry_declaration
	| #(PROCEDURE_DECLARATION def_id formal_part_opt)
	| #(FUNCTION_DECLARATION def_designator function_tail)
	| rep_spec
	| pragma
	;

prot_member_decl_s
	: #(PROT_MEMBER_DECLARATIONS ( prot_op_decl | comp_decl )* )
	;

comp_decl
	: #(COMPONENT_DECLARATION defining_identifier_list component_subtype_def init_opt)
	;

// decl_common is shared between declarative_item and basic_decl_item.
// decl_common only contains specifications.
decl_common
	: // type_def:
	  #(ENUMERATION_TYPE_DECLARATION IDENTIFIER enum_id_s)
	| #(SIGNED_INTEGER_TYPE_DECLARATION IDENTIFIER range)
	| #(MODULAR_TYPE_DECLARATION IDENTIFIER expression)
	| #(FLOATING_POINT_DECLARATION IDENTIFIER expression range_constraint_opt)
	| #(ORDINARY_FIXED_POINT_DECLARATION IDENTIFIER expression range)
	| #(DECIMAL_FIXED_POINT_DECLARATION IDENTIFIER expression expression range_constraint_opt)
	| array_type_declaration
	| access_type_declaration
	//
	| #(INCOMPLETE_TYPE_DECLARATION IDENTIFIER discrim_part_opt)
	// derived_or_private_or_record
	| #(PRIVATE_EXTENSION_DECLARATION id_and_discrim modifiers subtype_ind)
	| #(DERIVED_RECORD_EXTENSION id_and_discrim modifiers subtype_ind record_definition)
	| #(ORDINARY_DERIVED_TYPE_DECLARATION id_and_discrim subtype_ind)
	| #(PRIVATE_TYPE_DECLARATION id_and_discrim modifiers)
	| #(RECORD_TYPE_DECLARATION id_and_discrim modifiers record_definition)
	//
	| #(SUBTYPE_DECLARATION IDENTIFIER subtype_ind)
	| generic_decl
	| use_clause
	| rep_spec  // enumeration_representation_clause only
	| #(EXCEPTION_RENAMING_DECLARATION def_id compound_name)
	| #(OBJECT_RENAMING_DECLARATION def_id subtype_mark name)
	| #(EXCEPTION_DECLARATION defining_identifier_list)
	| #(NUMBER_DECLARATION defining_identifier_list expression)
	| #(ARRAY_OBJECT_DECLARATION defining_identifier_list modifiers
		array_type_definition init_opt)
	| #(OBJECT_DECLARATION defining_identifier_list modifiers
		subtype_ind init_opt)
	;

id_and_discrim
	: IDENTIFIER discrim_part_opt
	;

enum_id_s : ( enumeration_literal_specification )+
	;

enumeration_literal_specification : IDENTIFIER | CHARACTER_LITERAL
	;

array_type_definition
	: index_or_discrete_range_s component_subtype_def
	;

range_constraint_opt : ( range_constraint )?
	;

array_type_declaration
	: #(ARRAY_TYPE_DECLARATION IDENTIFIER array_type_definition)
	;

index_or_discrete_range_s
	: index_or_discrete_range
	| #(COMMA index_or_discrete_range_s index_or_discrete_range)
	;

index_or_discrete_range
	: #(DOT_DOT simple_expression simple_expression)
	| #(RANGE simple_expression ( BOX | range) )
	| simple_expression
	;

component_subtype_def : modifiers subtype_ind  // mod=aliased_opt
	;

subtype_ind
	: #(SUBTYPE_INDICATION subtype_mark constraint_opt)
	;

constraint_opt
	:	( range_constraint
		| digits_constraint
		| delta_constraint
		| index_constraint
		| discriminant_constraint
		)?
	;

digits_constraint
	: #(DIGITS_CONSTRAINT expression range_constraint_opt)
	;

delta_constraint
	: #(DELTA_CONSTRAINT expression range_constraint_opt)
	;

index_constraint
	: #(INDEX_CONSTRAINT ( discrete_range )+ )
	;

discrete_range
	: range
	| subtype_ind
	;

discriminant_constraint
	: #(DISCRIMINANT_CONSTRAINT ( discriminant_association )+ )
	;

discriminant_association
	: #(DISCRIMINANT_ASSOCIATION selector_names_opt expression)
	;

selector_names_opt
	: #(SELECTOR_NAMES_OPT ( selector_name )* )
	;

selector_name : IDENTIFIER  // TBD: sem pred
	;

access_type_declaration
	: #(ACCESS_TO_PROCEDURE_DECLARATION IDENTIFIER modifiers formal_part_opt)
	| #(ACCESS_TO_FUNCTION_DECLARATION IDENTIFIER modifiers function_tail)
	| #(ACCESS_TO_OBJECT_DECLARATION IDENTIFIER modifiers subtype_ind)
	;

record_definition : ( component_list )?
	;

component_list
	: component_items ( variant_part )?
	;

component_items
	: #(COMPONENT_ITEMS ( pragma | comp_decl )* )
	;

variant_part
	: #(VARIANT_PART discriminant_direct_name variant_s)
	;

discriminant_direct_name : IDENTIFIER  // TBD: symtab lookup.
	;

variant_s : #(VARIANTS ( variant )+ )
	;

variant : #(VARIANT choice_s ( component_list )? )
	;

choice_s : #(PIPE choice_s choice )
	| choice
	;

choice  : OTHERS
	| discrete_with_range
	| expression
	;

discrete_with_range : mark_with_constraint | range
	;

mark_with_constraint
	: #(MARK_WITH_CONSTRAINT subtype_mark range_constraint)
	;

local_enum_name : IDENTIFIER  // to be refined: do a symbol table lookup
	;

enumeration_aggregate : ( value )*
	;

generic_decl
	: #(GENERIC_PACKAGE_RENAMING generic_formal_part_opt def_id renames)
	| #(GENERIC_PACKAGE_DECLARATION generic_formal_part_opt def_id
		pkg_spec_part)
	| #(GENERIC_PROCEDURE_RENAMING generic_formal_part_opt def_id
		formal_part_opt renames)
	| #(GENERIC_PROCEDURE_DECLARATION generic_formal_part_opt def_id
		formal_part_opt)
	| #(GENERIC_FUNCTION_RENAMING generic_formal_part_opt def_designator
		function_tail renames)
	| #(GENERIC_FUNCTION_DECLARATION generic_formal_part_opt def_id
		function_tail)
	;

generic_formal_part_opt
	: #(GENERIC_FORMAL_PART
		( pragma | use_clause | generic_formal_parameter )*
	   )
	;

generic_formal_parameter
	: // FORMAL_TYPE_DECLARATIONs:
	  #(FORMAL_DISCRETE_TYPE_DECLARATION def_id)
	| #(FORMAL_SIGNED_INTEGER_TYPE_DECLARATION def_id)
	| #(FORMAL_MODULAR_TYPE_DECLARATION def_id)
	| #(FORMAL_DECIMAL_FIXED_POINT_DECLARATION def_id)
	| #(FORMAL_ORDINARY_FIXED_POINT_DECLARATION def_id)
	| #(FORMAL_FLOATING_POINT_DECLARATION def_id)
	| formal_array_type_declaration
	| formal_access_type_declaration
	| #(FORMAL_PRIVATE_TYPE_DECLARATION id_part modifiers)
	| #(FORMAL_ORDINARY_DERIVED_TYPE_DECLARATION id_part subtype_ind)
	| #(FORMAL_PRIVATE_EXTENSION_DECLARATION id_part modifiers subtype_ind)
	| #(FORMAL_PROCEDURE_DECLARATION def_id formal_part_opt
		subprogram_default_opt)
	| #(FORMAL_FUNCTION_DECLARATION def_designator function_tail
		subprogram_default_opt)
	| #(FORMAL_PACKAGE_DECLARATION def_id compound_name formal_package_actual_part_opt)
	| parameter_specification
	;


id_part : def_id discrim_part_opt
	;

formal_array_type_declaration : array_type_declaration
	;

formal_access_type_declaration : access_type_declaration
	;

subprogram_default_opt : ( BOX | name )?
                      // #(SUBPROGRAM_DEFAULT_OPT ( BOX | name )? )
	;

formal_package_actual_part_opt
	: ( BOX | defining_identifier_list )?
	;

subprog_decl_or_rename_or_inst_or_body
	: subprog_decl
	| procedure_body
	| function_body
	;

procedure_body
	: #(PROCEDURE_BODY def_id formal_part_opt body_part)
	;

function_body
	: #(FUNCTION_BODY def_designator function_tail body_part)
	;

body_part : declarative_part block_body
	;

declarative_part
	: #(DECLARATIVE_PART ( pragma | declarative_item )* )
	;

// A declarative_item may appear in the declarative part of any body.
declarative_item
	: #(PACKAGE_BODY_STUB def_id)
	| #(PACKAGE_BODY def_id pkg_body_part)
	| spec_decl_part
	| #(TASK_BODY_STUB def_id)
	| #(TASK_BODY def_id body_part)
	| task_type_or_single_decl
	| #(PROTECTED_BODY_STUB def_id)
	| #(PROTECTED_BODY def_id prot_op_bodies_opt)
	| prot_type_or_single_decl
	| subprog_decl_or_rename_or_inst_or_body
	| decl_common
	;

pkg_body_part : declarative_part block_body_opt
	;

block_body_opt
	: #(BLOCK_BODY_OPT ( handled_stmt_s )? )
	;

prot_op_bodies_opt
	: #(PROT_OP_BODIES_OPT
		( pragma | entry_body | subprog_decl_or_body )*
	   )
	;

subprog_decl_or_body
	: procedure_body
	| #(PROCEDURE_DECLARATION def_id formal_part_opt)
	| function_body
	| #(FUNCTION_DECLARATION def_designator function_tail)
	;

block_body : #(BLOCK_BODY handled_stmt_s)
	;

handled_stmt_s
	: #(HANDLED_SEQUENCE_OF_STATEMENTS statements except_handler_part_opt)
	;

statements
	: #(SEQUENCE_OF_STATEMENTS ( pragma | statement )+ )
	;

statement : #(STATEMENT def_label_opt
		( null_stmt
		| exit_stmt
		| return_stmt
		| goto_stmt
		| delay_stmt
		| abort_stmt
		| raise_stmt
		| requeue_stmt
		| accept_stmt
		| select_stmt
		| if_stmt
		| case_stmt
		| loop_stmt
		| block
		| call_or_assignment
		// | code_stmt  // not yet implemented in parser
		)
	     )
	;

def_label_opt : #(LABEL_OPT ( IDENTIFIER )? )
	;

null_stmt : NULL_STATEMENT
	;

if_stmt : #(IF_STATEMENT cond_clause elsifs_opt else_opt)
	;

cond_clause : #(COND_CLAUSE condition statements)
	;

condition : expression
	;

elsifs_opt : #(ELSIFS_OPT ( cond_clause )* )
	;

else_opt : #(ELSE_OPT ( statements )? )
	;

case_stmt : #(CASE_STATEMENT expression alternative_s)
	;

alternative_s : ( case_statement_alternative )+
	;

case_statement_alternative
	: #(CASE_STATEMENT_ALTERNATIVE choice_s statements)
	;

loop_stmt
	: #(LOOP_STATEMENT iteration_scheme_opt statements)
        ;

iteration_scheme_opt
	: #(ITERATION_SCHEME_OPT
		( #(WHILE condition)
		| #(FOR IDENTIFIER modifiers discrete_subtype_definition)
		)?                // mod=reverse_opt
	   )
	;

block   : #(BLOCK_STATEMENT declare_opt block_body)
	;

declare_opt
	: #(DECLARE_OPT ( declarative_part )? )
	;

exit_stmt
	: #(EXIT_STATEMENT ( label_name )? ( WHEN condition )? ) // TBD !!!
	;

label_name : IDENTIFIER
	;

return_stmt
	: #(RETURN_STATEMENT ( expression )? )
	;

goto_stmt : #(GOTO_STATEMENT label_name)
	;

call_or_assignment
	: #(ASSIGNMENT_STATEMENT name expression)
	| #(CALL_STATEMENT name)
	;

entry_body
	: #(ENTRY_BODY def_id entry_body_formal_part entry_barrier body_part)
	;

entry_body_formal_part : entry_index_spec_opt formal_part_opt
	;

entry_index_spec_opt
	: #(ENTRY_INDEX_SPECIFICATION ( def_id discrete_subtype_definition )? )
	;

entry_barrier : condition
	;

entry_call_stmt
	: #(ENTRY_CALL_STATEMENT name)
	;

accept_stmt
	: #(ACCEPT_STATEMENT def_id entry_index_opt formal_part_opt
		( handled_stmt_s )? )
	;

entry_index_opt
	: #(ENTRY_INDEX_OPT ( expression )? )
	;

delay_stmt
	: #(DELAY_STATEMENT modifiers expression)  //mod=until_opt
	;

// SELECT_STATEMENT itself is not modeled since it is trivially
// reconstructed:
//   select_statement ::= selective_accept | timed_entry_call
//             | conditional_entry_call | asynchronous_select
//
select_stmt
	: #(ASYNCHRONOUS_SELECT triggering_alternative abortable_part)
	| #(SELECTIVE_ACCEPT selective_accept)
	| #(TIMED_ENTRY_CALL entry_call_alternative delay_alternative)
	| #(CONDITIONAL_ENTRY_CALL entry_call_alternative statements)
	;

triggering_alternative
	: #(TRIGGERING_ALTERNATIVE
		( delay_stmt | entry_call_stmt ) stmts_opt
	   )
	;

abortable_part
	: #(ABORTABLE_PART stmts_opt)
	;

entry_call_alternative
	: #(ENTRY_CALL_ALTERNATIVE entry_call_stmt stmts_opt)
	;

selective_accept : guard_opt select_alternative or_select_opt else_opt
	;

guard_opt : #(GUARD_OPT ( condition ( pragma )* )? )
	;

select_alternative
	: accept_alternative
        | delay_alternative
	| TERMINATE_ALTERNATIVE
	;

accept_alternative
	: #(ACCEPT_ALTERNATIVE accept_stmt stmts_opt)
	;

delay_alternative
	: #(DELAY_ALTERNATIVE delay_stmt stmts_opt)
	;

stmts_opt : ( pragma | statement )*
	;

or_select_opt
	: #(OR_SELECT_OPT ( guard_opt select_alternative )* )
	;

abort_stmt
	: #(ABORT_STATEMENT ( name )+ )
	;

except_handler_part_opt
	: #(EXCEPT_HANDLER_PART_OPT ( exception_handler )* )
	;

exception_handler
	: #(EXCEPTION_HANDLER identifier_colon_opt except_choice_s
		statements
	   )
	;

identifier_colon_opt
	: #(IDENTIFIER_COLON_OPT ( IDENTIFIER )? )
	;

except_choice_s
	: #(PIPE except_choice_s exception_choice)
	| exception_choice
	;

exception_choice : compound_name
	| OTHERS
	;

raise_stmt
	: #(RAISE_STATEMENT ( compound_name )? )
	;

requeue_stmt
	: #(REQUEUE_STATEMENT name ( ABORT )? )
	;

operator_call : #(OPERATOR_SYMBOL value_s)
	;

value_s : #(VALUES ( value )+ )
	;

expression
	: #(AND expression relation)
	| #(AND_THEN expression relation)
	| #(OR expression relation)
	| #(OR_ELSE expression relation)
	| #(XOR expression relation)
	| relation
	;

relation
	: #(IN simple_expression range_or_mark)
	| #(NOT_IN simple_expression range_or_mark)
	| #(EQ simple_expression simple_expression)
	| #(NE simple_expression simple_expression)
	| #(LT_ simple_expression simple_expression)
	| #(LE simple_expression simple_expression)
	| #(GT simple_expression simple_expression)
	| #(GE simple_expression simple_expression)
	| simple_expression
	;

range_or_mark
	: range
	| subtype_mark
	;

simple_expression
	: #(PLUS simple_expression signed_term)
	| #(MINUS simple_expression signed_term)
	| #(CONCAT simple_expression signed_term)
	| signed_term
	;

signed_term
	: #(UNARY_PLUS term)
	| #(UNARY_MINUS term)
	| term
	;

term    : #(STAR term factor)
	| #(DIV term factor)
	| #(MOD term factor)
	| #(REM term factor)
	| factor
	;

factor  : #(NOT primary)
	| #(ABS primary)
	| #(EXPON primary primary)
	| primary
	;

primary :
	( name_or_qualified
	| parenthesized_primary
	| allocator
	| NuLL
	| NUMERIC_LIT
	| CHARACTER_LITERAL
	| CHAR_STRING
	| operator_call
	)
	;

// Temporary, to be turned into just `qualified'.
// We get away with it because `qualified' is always mentioned
// together with `name'.
// Only exception: `code_stmt', which is not yet implemented.
name_or_qualified
	: IDENTIFIER
	| #(DOT name_or_qualified
		( ALL
		| IDENTIFIER
		| CHARACTER_LITERAL
		| OPERATOR_SYMBOL
		)
	   )
	| #(INDEXED_COMPONENT name_or_qualified value_s)
	| #(TIC name_or_qualified
		( parenthesized_primary
		| attribute_id
		)
	   )
	;

allocator : #(ALLOCATOR name_or_qualified)
	;

subunit : #(SUBUNIT compound_name
		( subprogram_body
		| package_body
		| task_body
		| protected_body
		)
	   )
	;

subprogram_body
	: procedure_body
	| function_body
	;

package_body : #(PACKAGE_BODY def_id pkg_body_part)
	;

task_body : #(TASK_BODY def_id body_part)
	;
 
protected_body : #(PROTECTED_BODY def_id prot_op_bodies_opt)
	;

// TBD
// code_stmt : #(CODE_STATEMENT qualified)
//  	;


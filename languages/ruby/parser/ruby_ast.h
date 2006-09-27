// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#ifndef ruby_AST_H_INCLUDED
#define ruby_AST_H_INCLUDED

#include "kdev-pg-list.h"

#include <kdevast.h>


namespace ruby
  {

  struct additiveExpression_ast;
  struct aliasParameter_ast;
  struct andExpression_ast;
  struct andorExpression_ast;
  struct arrayAccess_ast;
  struct arrayExpression_ast;
  struct arrayReferenceArgument_ast;
  struct assignmentExpression_ast;
  struct blockContent_ast;
  struct blockMethodDefinitionArgument_ast;
  struct blockMethodInvocationArgument_ast;
  struct block_var_ast;
  struct block_vars_ast;
  struct bodyStatement_ast;
  struct caseExpression_ast;
  struct classDefinition_ast;
  struct className_ast;
  struct codeBlock_ast;
  struct colonAccess_ast;
  struct command_ast;
  struct commandOutput_ast;
  struct compoundStatement_ast;
  struct doOrTerminalOrColon_ast;
  struct dotAccess_ast;
  struct elementReference_ast;
  struct equalityExpression_ast;
  struct exceptionHandlingExpression_ast;
  struct exceptionList_ast;
  struct expression_ast;
  struct expressionSubstitution_ast;
  struct forExpression_ast;
  struct hashExpression_ast;
  struct ifExpression_ast;
  struct keyValuePair_ast;
  struct keyword_ast;
  struct keywordAsMethodName_ast;
  struct literal_ast;
  struct logicalAndExpression_ast;
  struct logicalOrExpression_ast;
  struct methodCall_ast;
  struct methodDefinition_ast;
  struct methodDefinitionArgument_ast;
  struct methodDefinitionArgumentWithoutParen_ast;
  struct methodInvocationArgumentWithParen_ast;
  struct methodInvocationArgumentWithoutParen_ast;
  struct methodName_ast;
  struct methodNameSupplement_ast;
  struct mlhs_item_ast;
  struct moduleDefinition_ast;
  struct moduleName_ast;
  struct mrhs_ast;
  struct multiplicativeExpression_ast;
  struct normalMethodDefinitionArgument_ast;
  struct normalMethodInvocationArgument_ast;
  struct notExpression_ast;
  struct numeric_ast;
  struct operatorAsMethodname_ast;
  struct orExpression_ast;
  struct parallelAssignmentLeftOver_ast;
  struct powerExpression_ast;
  struct predefinedValue_ast;
  struct primaryExpression_ast;
  struct program_ast;
  struct rangeExpression_ast;
  struct regex_ast;
  struct relationalExpression_ast;
  struct restMethodDefinitionArgument_ast;
  struct restMethodInvocationArgument_ast;
  struct shiftExpression_ast;
  struct statement_ast;
  struct statementWithoutModifier_ast;
  struct statements_ast;
  struct string_ast;
  struct symbol_ast;
  struct terminal_ast;
  struct ternaryIfThenElseExpression_ast;
  struct thenOrTerminalOrColon_ast;
  struct unaryExpression_ast;
  struct undefParameter_ast;
  struct unlessExpression_ast;
  struct untilExpression_ast;
  struct variable_ast;
  struct whileExpression_ast;


  struct ast_node: public KDevAST
    {
      enum ast_node_kind_enum {
        Kind_additiveExpression = 1000,
        Kind_aliasParameter = 1001,
        Kind_andExpression = 1002,
        Kind_andorExpression = 1003,
        Kind_arrayAccess = 1004,
        Kind_arrayExpression = 1005,
        Kind_arrayReferenceArgument = 1006,
        Kind_assignmentExpression = 1007,
        Kind_blockContent = 1008,
        Kind_blockMethodDefinitionArgument = 1009,
        Kind_blockMethodInvocationArgument = 1010,
        Kind_block_var = 1011,
        Kind_block_vars = 1012,
        Kind_bodyStatement = 1013,
        Kind_caseExpression = 1014,
        Kind_classDefinition = 1015,
        Kind_className = 1016,
        Kind_codeBlock = 1017,
        Kind_colonAccess = 1018,
        Kind_command = 1019,
        Kind_commandOutput = 1020,
        Kind_compoundStatement = 1021,
        Kind_doOrTerminalOrColon = 1022,
        Kind_dotAccess = 1023,
        Kind_elementReference = 1024,
        Kind_equalityExpression = 1025,
        Kind_exceptionHandlingExpression = 1026,
        Kind_exceptionList = 1027,
        Kind_expression = 1028,
        Kind_expressionSubstitution = 1029,
        Kind_forExpression = 1030,
        Kind_hashExpression = 1031,
        Kind_ifExpression = 1032,
        Kind_keyValuePair = 1033,
        Kind_keyword = 1034,
        Kind_keywordAsMethodName = 1035,
        Kind_literal = 1036,
        Kind_logicalAndExpression = 1037,
        Kind_logicalOrExpression = 1038,
        Kind_methodCall = 1039,
        Kind_methodDefinition = 1040,
        Kind_methodDefinitionArgument = 1041,
        Kind_methodDefinitionArgumentWithoutParen = 1042,
        Kind_methodInvocationArgumentWithParen = 1043,
        Kind_methodInvocationArgumentWithoutParen = 1044,
        Kind_methodName = 1045,
        Kind_methodNameSupplement = 1046,
        Kind_mlhs_item = 1047,
        Kind_moduleDefinition = 1048,
        Kind_moduleName = 1049,
        Kind_mrhs = 1050,
        Kind_multiplicativeExpression = 1051,
        Kind_normalMethodDefinitionArgument = 1052,
        Kind_normalMethodInvocationArgument = 1053,
        Kind_notExpression = 1054,
        Kind_numeric = 1055,
        Kind_operatorAsMethodname = 1056,
        Kind_orExpression = 1057,
        Kind_parallelAssignmentLeftOver = 1058,
        Kind_powerExpression = 1059,
        Kind_predefinedValue = 1060,
        Kind_primaryExpression = 1061,
        Kind_program = 1062,
        Kind_rangeExpression = 1063,
        Kind_regex = 1064,
        Kind_relationalExpression = 1065,
        Kind_restMethodDefinitionArgument = 1066,
        Kind_restMethodInvocationArgument = 1067,
        Kind_shiftExpression = 1068,
        Kind_statement = 1069,
        Kind_statementWithoutModifier = 1070,
        Kind_statements = 1071,
        Kind_string = 1072,
        Kind_symbol = 1073,
        Kind_terminal = 1074,
        Kind_ternaryIfThenElseExpression = 1075,
        Kind_thenOrTerminalOrColon = 1076,
        Kind_unaryExpression = 1077,
        Kind_undefParameter = 1078,
        Kind_unlessExpression = 1079,
        Kind_untilExpression = 1080,
        Kind_variable = 1081,
        Kind_whileExpression = 1082,
        AST_NODE_KIND_COUNT
      };

      int kind;
      std::size_t start_token;
      std::size_t end_token;
    };

  struct additiveExpression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_additiveExpression
      };

    };

  struct aliasParameter_ast: public ast_node
    {
      enum
      {
        KIND = Kind_aliasParameter
      };

    };

  struct andExpression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_andExpression
      };

    };

  struct andorExpression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_andorExpression
      };

    };

  struct arrayAccess_ast: public ast_node
    {
      enum
      {
        KIND = Kind_arrayAccess
      };

    };

  struct arrayExpression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_arrayExpression
      };

    };

  struct arrayReferenceArgument_ast: public ast_node
    {
      enum
      {
        KIND = Kind_arrayReferenceArgument
      };

    };

  struct assignmentExpression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_assignmentExpression
      };

    };

  struct blockContent_ast: public ast_node
    {
      enum
      {
        KIND = Kind_blockContent
      };

    };

  struct blockMethodDefinitionArgument_ast: public ast_node
    {
      enum
      {
        KIND = Kind_blockMethodDefinitionArgument
      };

    };

  struct blockMethodInvocationArgument_ast: public ast_node
    {
      enum
      {
        KIND = Kind_blockMethodInvocationArgument
      };

    };

  struct block_var_ast: public ast_node
    {
      enum
      {
        KIND = Kind_block_var
      };

    };

  struct block_vars_ast: public ast_node
    {
      enum
      {
        KIND = Kind_block_vars
      };

    };

  struct bodyStatement_ast: public ast_node
    {
      enum
      {
        KIND = Kind_bodyStatement
      };

    };

  struct caseExpression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_caseExpression
      };

    };

  struct classDefinition_ast: public ast_node
    {
      enum
      {
        KIND = Kind_classDefinition
      };

    };

  struct className_ast: public ast_node
    {
      enum
      {
        KIND = Kind_className
      };

    };

  struct codeBlock_ast: public ast_node
    {
      enum
      {
        KIND = Kind_codeBlock
      };

    };

  struct colonAccess_ast: public ast_node
    {
      enum
      {
        KIND = Kind_colonAccess
      };

    };

  struct command_ast: public ast_node
    {
      enum
      {
        KIND = Kind_command
      };

    };

  struct commandOutput_ast: public ast_node
    {
      enum
      {
        KIND = Kind_commandOutput
      };

    };

  struct compoundStatement_ast: public ast_node
    {
      enum
      {
        KIND = Kind_compoundStatement
      };

    };

  struct doOrTerminalOrColon_ast: public ast_node
    {
      enum
      {
        KIND = Kind_doOrTerminalOrColon
      };

    };

  struct dotAccess_ast: public ast_node
    {
      enum
      {
        KIND = Kind_dotAccess
      };

    };

  struct elementReference_ast: public ast_node
    {
      enum
      {
        KIND = Kind_elementReference
      };

    };

  struct equalityExpression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_equalityExpression
      };

    };

  struct exceptionHandlingExpression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_exceptionHandlingExpression
      };

    };

  struct exceptionList_ast: public ast_node
    {
      enum
      {
        KIND = Kind_exceptionList
      };

    };

  struct expression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_expression
      };

    };

  struct expressionSubstitution_ast: public ast_node
    {
      enum
      {
        KIND = Kind_expressionSubstitution
      };

    };

  struct forExpression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_forExpression
      };

    };

  struct hashExpression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_hashExpression
      };

    };

  struct ifExpression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_ifExpression
      };

    };

  struct keyValuePair_ast: public ast_node
    {
      enum
      {
        KIND = Kind_keyValuePair
      };

    };

  struct keyword_ast: public ast_node
    {
      enum
      {
        KIND = Kind_keyword
      };

    };

  struct keywordAsMethodName_ast: public ast_node
    {
      enum
      {
        KIND = Kind_keywordAsMethodName
      };

    };

  struct literal_ast: public ast_node
    {
      enum
      {
        KIND = Kind_literal
      };

    };

  struct logicalAndExpression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_logicalAndExpression
      };

    };

  struct logicalOrExpression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_logicalOrExpression
      };

    };

  struct methodCall_ast: public ast_node
    {
      enum
      {
        KIND = Kind_methodCall
      };

    };

  struct methodDefinition_ast: public ast_node
    {
      enum
      {
        KIND = Kind_methodDefinition
      };

    };

  struct methodDefinitionArgument_ast: public ast_node
    {
      enum
      {
        KIND = Kind_methodDefinitionArgument
      };

    };

  struct methodDefinitionArgumentWithoutParen_ast: public ast_node
    {
      enum
      {
        KIND = Kind_methodDefinitionArgumentWithoutParen
      };

    };

  struct methodInvocationArgumentWithParen_ast: public ast_node
    {
      enum
      {
        KIND = Kind_methodInvocationArgumentWithParen
      };

    };

  struct methodInvocationArgumentWithoutParen_ast: public ast_node
    {
      enum
      {
        KIND = Kind_methodInvocationArgumentWithoutParen
      };

    };

  struct methodName_ast: public ast_node
    {
      enum
      {
        KIND = Kind_methodName
      };

    };

  struct methodNameSupplement_ast: public ast_node
    {
      enum
      {
        KIND = Kind_methodNameSupplement
      };

    };

  struct mlhs_item_ast: public ast_node
    {
      enum
      {
        KIND = Kind_mlhs_item
      };

    };

  struct moduleDefinition_ast: public ast_node
    {
      enum
      {
        KIND = Kind_moduleDefinition
      };

    };

  struct moduleName_ast: public ast_node
    {
      enum
      {
        KIND = Kind_moduleName
      };

    };

  struct mrhs_ast: public ast_node
    {
      enum
      {
        KIND = Kind_mrhs
      };

    };

  struct multiplicativeExpression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_multiplicativeExpression
      };

    };

  struct normalMethodDefinitionArgument_ast: public ast_node
    {
      enum
      {
        KIND = Kind_normalMethodDefinitionArgument
      };

    };

  struct normalMethodInvocationArgument_ast: public ast_node
    {
      enum
      {
        KIND = Kind_normalMethodInvocationArgument
      };

    };

  struct notExpression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_notExpression
      };

    };

  struct numeric_ast: public ast_node
    {
      enum
      {
        KIND = Kind_numeric
      };

    };

  struct operatorAsMethodname_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorAsMethodname
      };

    };

  struct orExpression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_orExpression
      };

    };

  struct parallelAssignmentLeftOver_ast: public ast_node
    {
      enum
      {
        KIND = Kind_parallelAssignmentLeftOver
      };

    };

  struct powerExpression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_powerExpression
      };

    };

  struct predefinedValue_ast: public ast_node
    {
      enum
      {
        KIND = Kind_predefinedValue
      };

    };

  struct primaryExpression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_primaryExpression
      };

    };

  struct program_ast: public ast_node
    {
      enum
      {
        KIND = Kind_program
      };

    };

  struct rangeExpression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_rangeExpression
      };

    };

  struct regex_ast: public ast_node
    {
      enum
      {
        KIND = Kind_regex
      };

    };

  struct relationalExpression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_relationalExpression
      };

    };

  struct restMethodDefinitionArgument_ast: public ast_node
    {
      enum
      {
        KIND = Kind_restMethodDefinitionArgument
      };

    };

  struct restMethodInvocationArgument_ast: public ast_node
    {
      enum
      {
        KIND = Kind_restMethodInvocationArgument
      };

    };

  struct shiftExpression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_shiftExpression
      };

    };

  struct statement_ast: public ast_node
    {
      enum
      {
        KIND = Kind_statement
      };

    };

  struct statementWithoutModifier_ast: public ast_node
    {
      enum
      {
        KIND = Kind_statementWithoutModifier
      };

    };

  struct statements_ast: public ast_node
    {
      enum
      {
        KIND = Kind_statements
      };

    };

  struct string_ast: public ast_node
    {
      enum
      {
        KIND = Kind_string
      };

    };

  struct symbol_ast: public ast_node
    {
      enum
      {
        KIND = Kind_symbol
      };

    };

  struct terminal_ast: public ast_node
    {
      enum
      {
        KIND = Kind_terminal
      };

    };

  struct ternaryIfThenElseExpression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_ternaryIfThenElseExpression
      };

    };

  struct thenOrTerminalOrColon_ast: public ast_node
    {
      enum
      {
        KIND = Kind_thenOrTerminalOrColon
      };

    };

  struct unaryExpression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_unaryExpression
      };

    };

  struct undefParameter_ast: public ast_node
    {
      enum
      {
        KIND = Kind_undefParameter
      };

    };

  struct unlessExpression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_unlessExpression
      };

    };

  struct untilExpression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_untilExpression
      };

    };

  struct variable_ast: public ast_node
    {
      enum
      {
        KIND = Kind_variable
      };

    };

  struct whileExpression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_whileExpression
      };

    };



} // end of namespace ruby

#endif



// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#ifndef ruby_AST_H_INCLUDED
#define ruby_AST_H_INCLUDED

#include "kdev-pg-list.h"

#include <kdevast.h>


namespace ruby
  {
  class Lexer;
}

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
  struct keywordAlias_ast;
  struct keywordAnd_ast;
  struct keywordAsMethodName_ast;
  struct keywordBeginUpcase_ast;
  struct keywordBreak_ast;
  struct keywordClass_ast;
  struct keywordDef_ast;
  struct keywordDefined_ast;
  struct keywordDo_ast;
  struct keywordEnd_ast;
  struct keywordFor_ast;
  struct keywordIn_ast;
  struct keywordModule_ast;
  struct keywordNot_ast;
  struct keywordOr_ast;
  struct keywordUndef_ast;
  struct keywordUntil_ast;
  struct keywordWhen_ast;
  struct keywordWhile_ast;
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
  struct operatorAssign_ast;
  struct operatorBAnd_ast;
  struct operatorBAndAssign_ast;
  struct operatorBNot_ast;
  struct operatorBOr_ast;
  struct operatorBXor_ast;
  struct operatorBXorAssign_ast;
  struct operatorBorAssign_ast;
  struct operatorCaseEqual_ast;
  struct operatorColon_ast;
  struct operatorCompare_ast;
  struct operatorDiv_ast;
  struct operatorDivAssign_ast;
  struct operatorEqual_ast;
  struct operatorExclusiveRange_ast;
  struct operatorGreaterOrEqual_ast;
  struct operatorGreaterThan_ast;
  struct operatorInclusiveRange_ast;
  struct operatorLeftShift_ast;
  struct operatorLeftShiftAssign_ast;
  struct operatorLessOrEqual_ast;
  struct operatorLessThan_ast;
  struct operatorLogicalAnd_ast;
  struct operatorLogicalAndAssign_ast;
  struct operatorLogicalOr_ast;
  struct operatorLogicalOrAssign_ast;
  struct operatorMatch_ast;
  struct operatorMinus_ast;
  struct operatorMinusAssign_ast;
  struct operatorMod_ast;
  struct operatorModAssign_ast;
  struct operatorNot_ast;
  struct operatorNotEqual_ast;
  struct operatorNotMatch_ast;
  struct operatorPlus_ast;
  struct operatorPlusAssign_ast;
  struct operatorPower_ast;
  struct operatorPowerAssign_ast;
  struct operatorQuestion_ast;
  struct operatorRightShift_ast;
  struct operatorRightShiftAssign_ast;
  struct operatorStar_ast;
  struct operatorStarAssign_ast;
  struct operatorUnaryMinus_ast;
  struct operatorUnaryPlus_ast;
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
        Kind_keywordAlias = 1035,
        Kind_keywordAnd = 1036,
        Kind_keywordAsMethodName = 1037,
        Kind_keywordBeginUpcase = 1038,
        Kind_keywordBreak = 1039,
        Kind_keywordClass = 1040,
        Kind_keywordDef = 1041,
        Kind_keywordDefined = 1042,
        Kind_keywordDo = 1043,
        Kind_keywordEnd = 1044,
        Kind_keywordFor = 1045,
        Kind_keywordIn = 1046,
        Kind_keywordModule = 1047,
        Kind_keywordNot = 1048,
        Kind_keywordOr = 1049,
        Kind_keywordUndef = 1050,
        Kind_keywordUntil = 1051,
        Kind_keywordWhen = 1052,
        Kind_keywordWhile = 1053,
        Kind_literal = 1054,
        Kind_logicalAndExpression = 1055,
        Kind_logicalOrExpression = 1056,
        Kind_methodCall = 1057,
        Kind_methodDefinition = 1058,
        Kind_methodDefinitionArgument = 1059,
        Kind_methodDefinitionArgumentWithoutParen = 1060,
        Kind_methodInvocationArgumentWithParen = 1061,
        Kind_methodInvocationArgumentWithoutParen = 1062,
        Kind_methodName = 1063,
        Kind_methodNameSupplement = 1064,
        Kind_mlhs_item = 1065,
        Kind_moduleDefinition = 1066,
        Kind_moduleName = 1067,
        Kind_mrhs = 1068,
        Kind_multiplicativeExpression = 1069,
        Kind_normalMethodDefinitionArgument = 1070,
        Kind_normalMethodInvocationArgument = 1071,
        Kind_notExpression = 1072,
        Kind_numeric = 1073,
        Kind_operatorAsMethodname = 1074,
        Kind_operatorAssign = 1075,
        Kind_operatorBAnd = 1076,
        Kind_operatorBAndAssign = 1077,
        Kind_operatorBNot = 1078,
        Kind_operatorBOr = 1079,
        Kind_operatorBXor = 1080,
        Kind_operatorBXorAssign = 1081,
        Kind_operatorBorAssign = 1082,
        Kind_operatorCaseEqual = 1083,
        Kind_operatorColon = 1084,
        Kind_operatorCompare = 1085,
        Kind_operatorDiv = 1086,
        Kind_operatorDivAssign = 1087,
        Kind_operatorEqual = 1088,
        Kind_operatorExclusiveRange = 1089,
        Kind_operatorGreaterOrEqual = 1090,
        Kind_operatorGreaterThan = 1091,
        Kind_operatorInclusiveRange = 1092,
        Kind_operatorLeftShift = 1093,
        Kind_operatorLeftShiftAssign = 1094,
        Kind_operatorLessOrEqual = 1095,
        Kind_operatorLessThan = 1096,
        Kind_operatorLogicalAnd = 1097,
        Kind_operatorLogicalAndAssign = 1098,
        Kind_operatorLogicalOr = 1099,
        Kind_operatorLogicalOrAssign = 1100,
        Kind_operatorMatch = 1101,
        Kind_operatorMinus = 1102,
        Kind_operatorMinusAssign = 1103,
        Kind_operatorMod = 1104,
        Kind_operatorModAssign = 1105,
        Kind_operatorNot = 1106,
        Kind_operatorNotEqual = 1107,
        Kind_operatorNotMatch = 1108,
        Kind_operatorPlus = 1109,
        Kind_operatorPlusAssign = 1110,
        Kind_operatorPower = 1111,
        Kind_operatorPowerAssign = 1112,
        Kind_operatorQuestion = 1113,
        Kind_operatorRightShift = 1114,
        Kind_operatorRightShiftAssign = 1115,
        Kind_operatorStar = 1116,
        Kind_operatorStarAssign = 1117,
        Kind_operatorUnaryMinus = 1118,
        Kind_operatorUnaryPlus = 1119,
        Kind_orExpression = 1120,
        Kind_parallelAssignmentLeftOver = 1121,
        Kind_powerExpression = 1122,
        Kind_predefinedValue = 1123,
        Kind_primaryExpression = 1124,
        Kind_program = 1125,
        Kind_rangeExpression = 1126,
        Kind_regex = 1127,
        Kind_relationalExpression = 1128,
        Kind_restMethodDefinitionArgument = 1129,
        Kind_restMethodInvocationArgument = 1130,
        Kind_shiftExpression = 1131,
        Kind_statement = 1132,
        Kind_statementWithoutModifier = 1133,
        Kind_statements = 1134,
        Kind_string = 1135,
        Kind_symbol = 1136,
        Kind_terminal = 1137,
        Kind_ternaryIfThenElseExpression = 1138,
        Kind_thenOrTerminalOrColon = 1139,
        Kind_unaryExpression = 1140,
        Kind_undefParameter = 1141,
        Kind_unlessExpression = 1142,
        Kind_untilExpression = 1143,
        Kind_variable = 1144,
        Kind_whileExpression = 1145,
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

  struct keywordAlias_ast: public ast_node
    {
      enum
      {
        KIND = Kind_keywordAlias
      };

    };

  struct keywordAnd_ast: public ast_node
    {
      enum
      {
        KIND = Kind_keywordAnd
      };

    };

  struct keywordAsMethodName_ast: public ast_node
    {
      enum
      {
        KIND = Kind_keywordAsMethodName
      };

    };

  struct keywordBeginUpcase_ast: public ast_node
    {
      enum
      {
        KIND = Kind_keywordBeginUpcase
      };

    };

  struct keywordBreak_ast: public ast_node
    {
      enum
      {
        KIND = Kind_keywordBreak
      };

    };

  struct keywordClass_ast: public ast_node
    {
      enum
      {
        KIND = Kind_keywordClass
      };

    };

  struct keywordDef_ast: public ast_node
    {
      enum
      {
        KIND = Kind_keywordDef
      };

    };

  struct keywordDefined_ast: public ast_node
    {
      enum
      {
        KIND = Kind_keywordDefined
      };

    };

  struct keywordDo_ast: public ast_node
    {
      enum
      {
        KIND = Kind_keywordDo
      };

    };

  struct keywordEnd_ast: public ast_node
    {
      enum
      {
        KIND = Kind_keywordEnd
      };

    };

  struct keywordFor_ast: public ast_node
    {
      enum
      {
        KIND = Kind_keywordFor
      };

    };

  struct keywordIn_ast: public ast_node
    {
      enum
      {
        KIND = Kind_keywordIn
      };

    };

  struct keywordModule_ast: public ast_node
    {
      enum
      {
        KIND = Kind_keywordModule
      };

    };

  struct keywordNot_ast: public ast_node
    {
      enum
      {
        KIND = Kind_keywordNot
      };

    };

  struct keywordOr_ast: public ast_node
    {
      enum
      {
        KIND = Kind_keywordOr
      };

    };

  struct keywordUndef_ast: public ast_node
    {
      enum
      {
        KIND = Kind_keywordUndef
      };

    };

  struct keywordUntil_ast: public ast_node
    {
      enum
      {
        KIND = Kind_keywordUntil
      };

    };

  struct keywordWhen_ast: public ast_node
    {
      enum
      {
        KIND = Kind_keywordWhen
      };

    };

  struct keywordWhile_ast: public ast_node
    {
      enum
      {
        KIND = Kind_keywordWhile
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

  struct operatorAssign_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorAssign
      };

    };

  struct operatorBAnd_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorBAnd
      };

    };

  struct operatorBAndAssign_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorBAndAssign
      };

    };

  struct operatorBNot_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorBNot
      };

    };

  struct operatorBOr_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorBOr
      };

    };

  struct operatorBXor_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorBXor
      };

    };

  struct operatorBXorAssign_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorBXorAssign
      };

    };

  struct operatorBorAssign_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorBorAssign
      };

    };

  struct operatorCaseEqual_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorCaseEqual
      };

    };

  struct operatorColon_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorColon
      };

    };

  struct operatorCompare_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorCompare
      };

    };

  struct operatorDiv_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorDiv
      };

    };

  struct operatorDivAssign_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorDivAssign
      };

    };

  struct operatorEqual_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorEqual
      };

    };

  struct operatorExclusiveRange_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorExclusiveRange
      };

    };

  struct operatorGreaterOrEqual_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorGreaterOrEqual
      };

    };

  struct operatorGreaterThan_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorGreaterThan
      };

    };

  struct operatorInclusiveRange_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorInclusiveRange
      };

    };

  struct operatorLeftShift_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorLeftShift
      };

    };

  struct operatorLeftShiftAssign_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorLeftShiftAssign
      };

    };

  struct operatorLessOrEqual_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorLessOrEqual
      };

    };

  struct operatorLessThan_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorLessThan
      };

    };

  struct operatorLogicalAnd_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorLogicalAnd
      };

    };

  struct operatorLogicalAndAssign_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorLogicalAndAssign
      };

    };

  struct operatorLogicalOr_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorLogicalOr
      };

    };

  struct operatorLogicalOrAssign_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorLogicalOrAssign
      };

    };

  struct operatorMatch_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorMatch
      };

    };

  struct operatorMinus_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorMinus
      };

    };

  struct operatorMinusAssign_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorMinusAssign
      };

    };

  struct operatorMod_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorMod
      };

    };

  struct operatorModAssign_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorModAssign
      };

    };

  struct operatorNot_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorNot
      };

    };

  struct operatorNotEqual_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorNotEqual
      };

    };

  struct operatorNotMatch_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorNotMatch
      };

    };

  struct operatorPlus_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorPlus
      };

    };

  struct operatorPlusAssign_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorPlusAssign
      };

    };

  struct operatorPower_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorPower
      };

    };

  struct operatorPowerAssign_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorPowerAssign
      };

    };

  struct operatorQuestion_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorQuestion
      };

    };

  struct operatorRightShift_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorRightShift
      };

    };

  struct operatorRightShiftAssign_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorRightShiftAssign
      };

    };

  struct operatorStar_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorStar
      };

    };

  struct operatorStarAssign_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorStarAssign
      };

    };

  struct operatorUnaryMinus_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorUnaryMinus
      };

    };

  struct operatorUnaryPlus_ast: public ast_node
    {
      enum
      {
        KIND = Kind_operatorUnaryPlus
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



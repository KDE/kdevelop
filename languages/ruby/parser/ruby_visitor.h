// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#ifndef ruby_VISITOR_H_INCLUDED
#define ruby_VISITOR_H_INCLUDED

#include "ruby_ast.h"

namespace ruby
  {

  class visitor
    {
      typedef void (visitor::*parser_fun_t)(ast_node *);
      static parser_fun_t _S_parser_table[];

    public:
      virtual ~visitor()
      {}
      virtual void visit_node(ast_node *node)
      {
        if (node)
          (this->*_S_parser_table[node->kind - 1000])(node);
      }
      virtual void visit_additiveExpression(additiveExpression_ast *)
    {}
      virtual void visit_aliasParameter(aliasParameter_ast *)
      {}
      virtual void visit_andExpression(andExpression_ast *)
      {}
      virtual void visit_andorExpression(andorExpression_ast *)
      {}
      virtual void visit_arrayAccess(arrayAccess_ast *)
      {}
      virtual void visit_arrayExpression(arrayExpression_ast *)
      {}
      virtual void visit_arrayReferenceArgument(arrayReferenceArgument_ast *)
      {}
      virtual void visit_assignmentExpression(assignmentExpression_ast *)
      {}
      virtual void visit_blockContent(blockContent_ast *)
      {}
      virtual void visit_blockMethodDefinitionArgument(blockMethodDefinitionArgument_ast *)
      {}
      virtual void visit_blockMethodInvocationArgument(blockMethodInvocationArgument_ast *)
      {}
      virtual void visit_block_var(block_var_ast *)
      {}
      virtual void visit_block_vars(block_vars_ast *)
      {}
      virtual void visit_bodyStatement(bodyStatement_ast *)
      {}
      virtual void visit_caseExpression(caseExpression_ast *)
      {}
      virtual void visit_classDefinition(classDefinition_ast *)
      {}
      virtual void visit_className(className_ast *)
      {}
      virtual void visit_codeBlock(codeBlock_ast *)
      {}
      virtual void visit_colonAccess(colonAccess_ast *)
      {}
      virtual void visit_command(command_ast *)
      {}
      virtual void visit_commandOutput(commandOutput_ast *)
      {}
      virtual void visit_compoundStatement(compoundStatement_ast *)
      {}
      virtual void visit_doOrTerminalOrColon(doOrTerminalOrColon_ast *)
      {}
      virtual void visit_dotAccess(dotAccess_ast *)
      {}
      virtual void visit_elementReference(elementReference_ast *)
      {}
      virtual void visit_equalityExpression(equalityExpression_ast *)
      {}
      virtual void visit_exceptionHandlingExpression(exceptionHandlingExpression_ast *)
      {}
      virtual void visit_exceptionList(exceptionList_ast *)
      {}
      virtual void visit_expression(expression_ast *)
      {}
      virtual void visit_expressionSubstitution(expressionSubstitution_ast *)
      {}
      virtual void visit_forExpression(forExpression_ast *)
      {}
      virtual void visit_hashExpression(hashExpression_ast *)
      {}
      virtual void visit_ifExpression(ifExpression_ast *)
      {}
      virtual void visit_keyValuePair(keyValuePair_ast *)
      {}
      virtual void visit_keyword(keyword_ast *)
      {}
      virtual void visit_keywordAlias(keywordAlias_ast *)
      {}
      virtual void visit_keywordAnd(keywordAnd_ast *)
      {}
      virtual void visit_keywordAsMethodName(keywordAsMethodName_ast *)
      {}
      virtual void visit_keywordBeginUpcase(keywordBeginUpcase_ast *)
      {}
      virtual void visit_keywordBreak(keywordBreak_ast *)
      {}
      virtual void visit_keywordClass(keywordClass_ast *)
      {}
      virtual void visit_keywordDef(keywordDef_ast *)
      {}
      virtual void visit_keywordDefined(keywordDefined_ast *)
      {}
      virtual void visit_keywordDo(keywordDo_ast *)
      {}
      virtual void visit_keywordEnd(keywordEnd_ast *)
      {}
      virtual void visit_keywordFor(keywordFor_ast *)
      {}
      virtual void visit_keywordIn(keywordIn_ast *)
      {}
      virtual void visit_keywordModule(keywordModule_ast *)
      {}
      virtual void visit_keywordNot(keywordNot_ast *)
      {}
      virtual void visit_keywordOr(keywordOr_ast *)
      {}
      virtual void visit_keywordUndef(keywordUndef_ast *)
      {}
      virtual void visit_keywordUntil(keywordUntil_ast *)
      {}
      virtual void visit_keywordWhen(keywordWhen_ast *)
      {}
      virtual void visit_keywordWhile(keywordWhile_ast *)
      {}
      virtual void visit_literal(literal_ast *)
      {}
      virtual void visit_logicalAndExpression(logicalAndExpression_ast *)
      {}
      virtual void visit_logicalOrExpression(logicalOrExpression_ast *)
      {}
      virtual void visit_methodCall(methodCall_ast *)
      {}
      virtual void visit_methodDefinition(methodDefinition_ast *)
      {}
      virtual void visit_methodDefinitionArgument(methodDefinitionArgument_ast *)
      {}
      virtual void visit_methodDefinitionArgumentWithoutParen(methodDefinitionArgumentWithoutParen_ast *)
      {}
      virtual void visit_methodInvocationArgumentWithParen(methodInvocationArgumentWithParen_ast *)
      {}
      virtual void visit_methodInvocationArgumentWithoutParen(methodInvocationArgumentWithoutParen_ast *)
      {}
      virtual void visit_methodName(methodName_ast *)
      {}
      virtual void visit_methodNameSupplement(methodNameSupplement_ast *)
      {}
      virtual void visit_mlhs_item(mlhs_item_ast *)
      {}
      virtual void visit_moduleDefinition(moduleDefinition_ast *)
      {}
      virtual void visit_moduleName(moduleName_ast *)
      {}
      virtual void visit_mrhs(mrhs_ast *)
      {}
      virtual void visit_multiplicativeExpression(multiplicativeExpression_ast *)
      {}
      virtual void visit_normalMethodDefinitionArgument(normalMethodDefinitionArgument_ast *)
      {}
      virtual void visit_normalMethodInvocationArgument(normalMethodInvocationArgument_ast *)
      {}
      virtual void visit_notExpression(notExpression_ast *)
      {}
      virtual void visit_numeric(numeric_ast *)
      {}
      virtual void visit_operatorAsMethodname(operatorAsMethodname_ast *)
      {}
      virtual void visit_operatorAssign(operatorAssign_ast *)
      {}
      virtual void visit_operatorBAnd(operatorBAnd_ast *)
      {}
      virtual void visit_operatorBAndAssign(operatorBAndAssign_ast *)
      {}
      virtual void visit_operatorBNot(operatorBNot_ast *)
      {}
      virtual void visit_operatorBOr(operatorBOr_ast *)
      {}
      virtual void visit_operatorBXor(operatorBXor_ast *)
      {}
      virtual void visit_operatorBXorAssign(operatorBXorAssign_ast *)
      {}
      virtual void visit_operatorBorAssign(operatorBorAssign_ast *)
      {}
      virtual void visit_operatorCaseEqual(operatorCaseEqual_ast *)
      {}
      virtual void visit_operatorColon(operatorColon_ast *)
      {}
      virtual void visit_operatorCompare(operatorCompare_ast *)
      {}
      virtual void visit_operatorDiv(operatorDiv_ast *)
      {}
      virtual void visit_operatorDivAssign(operatorDivAssign_ast *)
      {}
      virtual void visit_operatorEqual(operatorEqual_ast *)
      {}
      virtual void visit_operatorExclusiveRange(operatorExclusiveRange_ast *)
      {}
      virtual void visit_operatorGreaterOrEqual(operatorGreaterOrEqual_ast *)
      {}
      virtual void visit_operatorGreaterThan(operatorGreaterThan_ast *)
      {}
      virtual void visit_operatorInclusiveRange(operatorInclusiveRange_ast *)
      {}
      virtual void visit_operatorLeftShift(operatorLeftShift_ast *)
      {}
      virtual void visit_operatorLeftShiftAssign(operatorLeftShiftAssign_ast *)
      {}
      virtual void visit_operatorLessOrEqual(operatorLessOrEqual_ast *)
      {}
      virtual void visit_operatorLessThan(operatorLessThan_ast *)
      {}
      virtual void visit_operatorLogicalAnd(operatorLogicalAnd_ast *)
      {}
      virtual void visit_operatorLogicalAndAssign(operatorLogicalAndAssign_ast *)
      {}
      virtual void visit_operatorLogicalOr(operatorLogicalOr_ast *)
      {}
      virtual void visit_operatorLogicalOrAssign(operatorLogicalOrAssign_ast *)
      {}
      virtual void visit_operatorMatch(operatorMatch_ast *)
      {}
      virtual void visit_operatorMinus(operatorMinus_ast *)
      {}
      virtual void visit_operatorMinusAssign(operatorMinusAssign_ast *)
      {}
      virtual void visit_operatorMod(operatorMod_ast *)
      {}
      virtual void visit_operatorModAssign(operatorModAssign_ast *)
      {}
      virtual void visit_operatorNot(operatorNot_ast *)
      {}
      virtual void visit_operatorNotEqual(operatorNotEqual_ast *)
      {}
      virtual void visit_operatorNotMatch(operatorNotMatch_ast *)
      {}
      virtual void visit_operatorPlus(operatorPlus_ast *)
      {}
      virtual void visit_operatorPlusAssign(operatorPlusAssign_ast *)
      {}
      virtual void visit_operatorPower(operatorPower_ast *)
      {}
      virtual void visit_operatorPowerAssign(operatorPowerAssign_ast *)
      {}
      virtual void visit_operatorQuestion(operatorQuestion_ast *)
      {}
      virtual void visit_operatorRightShift(operatorRightShift_ast *)
      {}
      virtual void visit_operatorRightShiftAssign(operatorRightShiftAssign_ast *)
      {}
      virtual void visit_operatorStar(operatorStar_ast *)
      {}
      virtual void visit_operatorStarAssign(operatorStarAssign_ast *)
      {}
      virtual void visit_operatorUnaryMinus(operatorUnaryMinus_ast *)
      {}
      virtual void visit_operatorUnaryPlus(operatorUnaryPlus_ast *)
      {}
      virtual void visit_orExpression(orExpression_ast *)
      {}
      virtual void visit_parallelAssignmentLeftOver(parallelAssignmentLeftOver_ast *)
      {}
      virtual void visit_powerExpression(powerExpression_ast *)
      {}
      virtual void visit_predefinedValue(predefinedValue_ast *)
      {}
      virtual void visit_primaryExpression(primaryExpression_ast *)
      {}
      virtual void visit_program(program_ast *)
      {}
      virtual void visit_rangeExpression(rangeExpression_ast *)
      {}
      virtual void visit_regex(regex_ast *)
      {}
      virtual void visit_relationalExpression(relationalExpression_ast *)
      {}
      virtual void visit_restMethodDefinitionArgument(restMethodDefinitionArgument_ast *)
      {}
      virtual void visit_restMethodInvocationArgument(restMethodInvocationArgument_ast *)
      {}
      virtual void visit_shiftExpression(shiftExpression_ast *)
      {}
      virtual void visit_statement(statement_ast *)
      {}
      virtual void visit_statementWithoutModifier(statementWithoutModifier_ast *)
      {}
      virtual void visit_statements(statements_ast *)
      {}
      virtual void visit_string(string_ast *)
      {}
      virtual void visit_symbol(symbol_ast *)
      {}
      virtual void visit_terminal(terminal_ast *)
      {}
      virtual void visit_ternaryIfThenElseExpression(ternaryIfThenElseExpression_ast *)
      {}
      virtual void visit_thenOrTerminalOrColon(thenOrTerminalOrColon_ast *)
      {}
      virtual void visit_unaryExpression(unaryExpression_ast *)
      {}
      virtual void visit_undefParameter(undefParameter_ast *)
      {}
      virtual void visit_unlessExpression(unlessExpression_ast *)
      {}
      virtual void visit_untilExpression(untilExpression_ast *)
      {}
      virtual void visit_variable(variable_ast *)
      {}
      virtual void visit_whileExpression(whileExpression_ast *)
      {}
    }
  ;

} // end of namespace ruby

#endif



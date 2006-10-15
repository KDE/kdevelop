// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#ifndef ruby_DEBUG_VISITOR_H_INCLUDED
#define ruby_DEBUG_VISITOR_H_INCLUDED

#include "ruby_default_visitor.h"

#include <iostream>
#include <fstream>

namespace ruby
  {

  class debug_visitor: public default_visitor
    {
    public:
      virtual void visit_additiveExpression(additiveExpression_ast *node)
      {
        std::cout << "additiveExpression" << std::endl;
        default_visitor::visit_additiveExpression(node);
      }

      virtual void visit_aliasParameter(aliasParameter_ast *node)
      {
        std::cout << "aliasParameter" << std::endl;
        default_visitor::visit_aliasParameter(node);
      }

      virtual void visit_andExpression(andExpression_ast *node)
      {
        std::cout << "andExpression" << std::endl;
        default_visitor::visit_andExpression(node);
      }

      virtual void visit_andorExpression(andorExpression_ast *node)
      {
        std::cout << "andorExpression" << std::endl;
        default_visitor::visit_andorExpression(node);
      }

      virtual void visit_arrayAccess(arrayAccess_ast *node)
      {
        std::cout << "arrayAccess" << std::endl;
        default_visitor::visit_arrayAccess(node);
      }

      virtual void visit_arrayExpression(arrayExpression_ast *node)
      {
        std::cout << "arrayExpression" << std::endl;
        default_visitor::visit_arrayExpression(node);
      }

      virtual void visit_arrayReferenceArgument(arrayReferenceArgument_ast *node)
      {
        std::cout << "arrayReferenceArgument" << std::endl;
        default_visitor::visit_arrayReferenceArgument(node);
      }

      virtual void visit_assignmentExpression(assignmentExpression_ast *node)
      {
        std::cout << "assignmentExpression" << std::endl;
        default_visitor::visit_assignmentExpression(node);
      }

      virtual void visit_blockContent(blockContent_ast *node)
      {
        std::cout << "blockContent" << std::endl;
        default_visitor::visit_blockContent(node);
      }

      virtual void visit_blockMethodDefinitionArgument(blockMethodDefinitionArgument_ast *node)
      {
        std::cout << "blockMethodDefinitionArgument" << std::endl;
        default_visitor::visit_blockMethodDefinitionArgument(node);
      }

      virtual void visit_blockMethodInvocationArgument(blockMethodInvocationArgument_ast *node)
      {
        std::cout << "blockMethodInvocationArgument" << std::endl;
        default_visitor::visit_blockMethodInvocationArgument(node);
      }

      virtual void visit_block_var(block_var_ast *node)
      {
        std::cout << "block_var" << std::endl;
        default_visitor::visit_block_var(node);
      }

      virtual void visit_block_vars(block_vars_ast *node)
      {
        std::cout << "block_vars" << std::endl;
        default_visitor::visit_block_vars(node);
      }

      virtual void visit_bodyStatement(bodyStatement_ast *node)
      {
        std::cout << "bodyStatement" << std::endl;
        default_visitor::visit_bodyStatement(node);
      }

      virtual void visit_caseExpression(caseExpression_ast *node)
      {
        std::cout << "caseExpression" << std::endl;
        default_visitor::visit_caseExpression(node);
      }

      virtual void visit_classDefinition(classDefinition_ast *node)
      {
        std::cout << "classDefinition" << std::endl;
        default_visitor::visit_classDefinition(node);
      }

      virtual void visit_className(className_ast *node)
      {
        std::cout << "className" << std::endl;
        default_visitor::visit_className(node);
      }

      virtual void visit_codeBlock(codeBlock_ast *node)
      {
        std::cout << "codeBlock" << std::endl;
        default_visitor::visit_codeBlock(node);
      }

      virtual void visit_colonAccess(colonAccess_ast *node)
      {
        std::cout << "colonAccess" << std::endl;
        default_visitor::visit_colonAccess(node);
      }

      virtual void visit_command(command_ast *node)
      {
        std::cout << "command" << std::endl;
        default_visitor::visit_command(node);
      }

      virtual void visit_commandOutput(commandOutput_ast *node)
      {
        std::cout << "commandOutput" << std::endl;
        default_visitor::visit_commandOutput(node);
      }

      virtual void visit_compoundStatement(compoundStatement_ast *node)
      {
        std::cout << "compoundStatement" << std::endl;
        default_visitor::visit_compoundStatement(node);
      }

      virtual void visit_doOrTerminalOrColon(doOrTerminalOrColon_ast *node)
      {
        std::cout << "doOrTerminalOrColon" << std::endl;
        default_visitor::visit_doOrTerminalOrColon(node);
      }

      virtual void visit_dotAccess(dotAccess_ast *node)
      {
        std::cout << "dotAccess" << std::endl;
        default_visitor::visit_dotAccess(node);
      }

      virtual void visit_elementReference(elementReference_ast *node)
      {
        std::cout << "elementReference" << std::endl;
        default_visitor::visit_elementReference(node);
      }

      virtual void visit_equalityExpression(equalityExpression_ast *node)
      {
        std::cout << "equalityExpression" << std::endl;
        default_visitor::visit_equalityExpression(node);
      }

      virtual void visit_exceptionHandlingExpression(exceptionHandlingExpression_ast *node)
      {
        std::cout << "exceptionHandlingExpression" << std::endl;
        default_visitor::visit_exceptionHandlingExpression(node);
      }

      virtual void visit_exceptionList(exceptionList_ast *node)
      {
        std::cout << "exceptionList" << std::endl;
        default_visitor::visit_exceptionList(node);
      }

      virtual void visit_expression(expression_ast *node)
      {
        std::cout << "expression" << std::endl;
        default_visitor::visit_expression(node);
      }

      virtual void visit_expressionSubstitution(expressionSubstitution_ast *node)
      {
        std::cout << "expressionSubstitution" << std::endl;
        default_visitor::visit_expressionSubstitution(node);
      }

      virtual void visit_forExpression(forExpression_ast *node)
      {
        std::cout << "forExpression" << std::endl;
        default_visitor::visit_forExpression(node);
      }

      virtual void visit_hashExpression(hashExpression_ast *node)
      {
        std::cout << "hashExpression" << std::endl;
        default_visitor::visit_hashExpression(node);
      }

      virtual void visit_ifExpression(ifExpression_ast *node)
      {
        std::cout << "ifExpression" << std::endl;
        default_visitor::visit_ifExpression(node);
      }

      virtual void visit_keyValuePair(keyValuePair_ast *node)
      {
        std::cout << "keyValuePair" << std::endl;
        default_visitor::visit_keyValuePair(node);
      }

      virtual void visit_keyword(keyword_ast *node)
      {
        std::cout << "keyword" << std::endl;
        default_visitor::visit_keyword(node);
      }

      virtual void visit_keywordAlias(keywordAlias_ast *node)
      {
        std::cout << "keywordAlias" << std::endl;
        default_visitor::visit_keywordAlias(node);
      }

      virtual void visit_keywordAnd(keywordAnd_ast *node)
      {
        std::cout << "keywordAnd" << std::endl;
        default_visitor::visit_keywordAnd(node);
      }

      virtual void visit_keywordAsMethodName(keywordAsMethodName_ast *node)
      {
        std::cout << "keywordAsMethodName" << std::endl;
        default_visitor::visit_keywordAsMethodName(node);
      }

      virtual void visit_keywordBeginUpcase(keywordBeginUpcase_ast *node)
      {
        std::cout << "keywordBeginUpcase" << std::endl;
        default_visitor::visit_keywordBeginUpcase(node);
      }

      virtual void visit_keywordBreak(keywordBreak_ast *node)
      {
        std::cout << "keywordBreak" << std::endl;
        default_visitor::visit_keywordBreak(node);
      }

      virtual void visit_keywordClass(keywordClass_ast *node)
      {
        std::cout << "keywordClass" << std::endl;
        default_visitor::visit_keywordClass(node);
      }

      virtual void visit_keywordDef(keywordDef_ast *node)
      {
        std::cout << "keywordDef" << std::endl;
        default_visitor::visit_keywordDef(node);
      }

      virtual void visit_keywordDefined(keywordDefined_ast *node)
      {
        std::cout << "keywordDefined" << std::endl;
        default_visitor::visit_keywordDefined(node);
      }

      virtual void visit_keywordDo(keywordDo_ast *node)
      {
        std::cout << "keywordDo" << std::endl;
        default_visitor::visit_keywordDo(node);
      }

      virtual void visit_keywordEndUpcase(keywordEndUpcase_ast *node)
      {
        std::cout << "keywordEndUpcase" << std::endl;
        default_visitor::visit_keywordEndUpcase(node);
      }

      virtual void visit_keywordFor(keywordFor_ast *node)
      {
        std::cout << "keywordFor" << std::endl;
        default_visitor::visit_keywordFor(node);
      }

      virtual void visit_keywordIn(keywordIn_ast *node)
      {
        std::cout << "keywordIn" << std::endl;
        default_visitor::visit_keywordIn(node);
      }

      virtual void visit_keywordModule(keywordModule_ast *node)
      {
        std::cout << "keywordModule" << std::endl;
        default_visitor::visit_keywordModule(node);
      }

      virtual void visit_keywordNot(keywordNot_ast *node)
      {
        std::cout << "keywordNot" << std::endl;
        default_visitor::visit_keywordNot(node);
      }

      virtual void visit_keywordOr(keywordOr_ast *node)
      {
        std::cout << "keywordOr" << std::endl;
        default_visitor::visit_keywordOr(node);
      }

      virtual void visit_keywordUndef(keywordUndef_ast *node)
      {
        std::cout << "keywordUndef" << std::endl;
        default_visitor::visit_keywordUndef(node);
      }

      virtual void visit_keywordUntil(keywordUntil_ast *node)
      {
        std::cout << "keywordUntil" << std::endl;
        default_visitor::visit_keywordUntil(node);
      }

      virtual void visit_keywordWhen(keywordWhen_ast *node)
      {
        std::cout << "keywordWhen" << std::endl;
        default_visitor::visit_keywordWhen(node);
      }

      virtual void visit_keywordWhile(keywordWhile_ast *node)
      {
        std::cout << "keywordWhile" << std::endl;
        default_visitor::visit_keywordWhile(node);
      }

      virtual void visit_literal(literal_ast *node)
      {
        std::cout << "literal" << std::endl;
        default_visitor::visit_literal(node);
      }

      virtual void visit_logicalAndExpression(logicalAndExpression_ast *node)
      {
        std::cout << "logicalAndExpression" << std::endl;
        default_visitor::visit_logicalAndExpression(node);
      }

      virtual void visit_logicalOrExpression(logicalOrExpression_ast *node)
      {
        std::cout << "logicalOrExpression" << std::endl;
        default_visitor::visit_logicalOrExpression(node);
      }

      virtual void visit_methodCall(methodCall_ast *node)
      {
        std::cout << "methodCall" << std::endl;
        default_visitor::visit_methodCall(node);
      }

      virtual void visit_methodDefinition(methodDefinition_ast *node)
      {
        std::cout << "methodDefinition" << std::endl;
        default_visitor::visit_methodDefinition(node);
      }

      virtual void visit_methodDefinitionArgument(methodDefinitionArgument_ast *node)
      {
        std::cout << "methodDefinitionArgument" << std::endl;
        default_visitor::visit_methodDefinitionArgument(node);
      }

      virtual void visit_methodDefinitionArgumentWithoutParen(methodDefinitionArgumentWithoutParen_ast *node)
      {
        std::cout << "methodDefinitionArgumentWithoutParen" << std::endl;
        default_visitor::visit_methodDefinitionArgumentWithoutParen(node);
      }

      virtual void visit_methodInvocationArgumentWithParen(methodInvocationArgumentWithParen_ast *node)
      {
        std::cout << "methodInvocationArgumentWithParen" << std::endl;
        default_visitor::visit_methodInvocationArgumentWithParen(node);
      }

      virtual void visit_methodInvocationArgumentWithoutParen(methodInvocationArgumentWithoutParen_ast *node)
      {
        std::cout << "methodInvocationArgumentWithoutParen" << std::endl;
        default_visitor::visit_methodInvocationArgumentWithoutParen(node);
      }

      virtual void visit_methodName(methodName_ast *node)
      {
        std::cout << "methodName" << std::endl;
        default_visitor::visit_methodName(node);
      }

      virtual void visit_methodNameSupplement(methodNameSupplement_ast *node)
      {
        std::cout << "methodNameSupplement" << std::endl;
        default_visitor::visit_methodNameSupplement(node);
      }

      virtual void visit_mlhs_item(mlhs_item_ast *node)
      {
        std::cout << "mlhs_item" << std::endl;
        default_visitor::visit_mlhs_item(node);
      }

      virtual void visit_moduleDefinition(moduleDefinition_ast *node)
      {
        std::cout << "moduleDefinition" << std::endl;
        default_visitor::visit_moduleDefinition(node);
      }

      virtual void visit_moduleName(moduleName_ast *node)
      {
        std::cout << "moduleName" << std::endl;
        default_visitor::visit_moduleName(node);
      }

      virtual void visit_mrhs(mrhs_ast *node)
      {
        std::cout << "mrhs" << std::endl;
        default_visitor::visit_mrhs(node);
      }

      virtual void visit_multiplicativeExpression(multiplicativeExpression_ast *node)
      {
        std::cout << "multiplicativeExpression" << std::endl;
        default_visitor::visit_multiplicativeExpression(node);
      }

      virtual void visit_normalMethodDefinitionArgument(normalMethodDefinitionArgument_ast *node)
      {
        std::cout << "normalMethodDefinitionArgument" << std::endl;
        default_visitor::visit_normalMethodDefinitionArgument(node);
      }

      virtual void visit_normalMethodInvocationArgument(normalMethodInvocationArgument_ast *node)
      {
        std::cout << "normalMethodInvocationArgument" << std::endl;
        default_visitor::visit_normalMethodInvocationArgument(node);
      }

      virtual void visit_notExpression(notExpression_ast *node)
      {
        std::cout << "notExpression" << std::endl;
        default_visitor::visit_notExpression(node);
      }

      virtual void visit_numeric(numeric_ast *node)
      {
        std::cout << "numeric" << std::endl;
        default_visitor::visit_numeric(node);
      }

      virtual void visit_operatorAsMethodname(operatorAsMethodname_ast *node)
      {
        std::cout << "operatorAsMethodname" << std::endl;
        default_visitor::visit_operatorAsMethodname(node);
      }

      virtual void visit_operatorAssign(operatorAssign_ast *node)
      {
        std::cout << "operatorAssign" << std::endl;
        default_visitor::visit_operatorAssign(node);
      }

      virtual void visit_operatorBAnd(operatorBAnd_ast *node)
      {
        std::cout << "operatorBAnd" << std::endl;
        default_visitor::visit_operatorBAnd(node);
      }

      virtual void visit_operatorBAndAssign(operatorBAndAssign_ast *node)
      {
        std::cout << "operatorBAndAssign" << std::endl;
        default_visitor::visit_operatorBAndAssign(node);
      }

      virtual void visit_operatorBNot(operatorBNot_ast *node)
      {
        std::cout << "operatorBNot" << std::endl;
        default_visitor::visit_operatorBNot(node);
      }

      virtual void visit_operatorBOr(operatorBOr_ast *node)
      {
        std::cout << "operatorBOr" << std::endl;
        default_visitor::visit_operatorBOr(node);
      }

      virtual void visit_operatorBXor(operatorBXor_ast *node)
      {
        std::cout << "operatorBXor" << std::endl;
        default_visitor::visit_operatorBXor(node);
      }

      virtual void visit_operatorBXorAssign(operatorBXorAssign_ast *node)
      {
        std::cout << "operatorBXorAssign" << std::endl;
        default_visitor::visit_operatorBXorAssign(node);
      }

      virtual void visit_operatorBorAssign(operatorBorAssign_ast *node)
      {
        std::cout << "operatorBorAssign" << std::endl;
        default_visitor::visit_operatorBorAssign(node);
      }

      virtual void visit_operatorCaseEqual(operatorCaseEqual_ast *node)
      {
        std::cout << "operatorCaseEqual" << std::endl;
        default_visitor::visit_operatorCaseEqual(node);
      }

      virtual void visit_operatorColon(operatorColon_ast *node)
      {
        std::cout << "operatorColon" << std::endl;
        default_visitor::visit_operatorColon(node);
      }

      virtual void visit_operatorCompare(operatorCompare_ast *node)
      {
        std::cout << "operatorCompare" << std::endl;
        default_visitor::visit_operatorCompare(node);
      }

      virtual void visit_operatorDiv(operatorDiv_ast *node)
      {
        std::cout << "operatorDiv" << std::endl;
        default_visitor::visit_operatorDiv(node);
      }

      virtual void visit_operatorDivAssign(operatorDivAssign_ast *node)
      {
        std::cout << "operatorDivAssign" << std::endl;
        default_visitor::visit_operatorDivAssign(node);
      }

      virtual void visit_operatorEqual(operatorEqual_ast *node)
      {
        std::cout << "operatorEqual" << std::endl;
        default_visitor::visit_operatorEqual(node);
      }

      virtual void visit_operatorExclusiveRange(operatorExclusiveRange_ast *node)
      {
        std::cout << "operatorExclusiveRange" << std::endl;
        default_visitor::visit_operatorExclusiveRange(node);
      }

      virtual void visit_operatorGreaterOrEqual(operatorGreaterOrEqual_ast *node)
      {
        std::cout << "operatorGreaterOrEqual" << std::endl;
        default_visitor::visit_operatorGreaterOrEqual(node);
      }

      virtual void visit_operatorGreaterThan(operatorGreaterThan_ast *node)
      {
        std::cout << "operatorGreaterThan" << std::endl;
        default_visitor::visit_operatorGreaterThan(node);
      }

      virtual void visit_operatorInclusiveRange(operatorInclusiveRange_ast *node)
      {
        std::cout << "operatorInclusiveRange" << std::endl;
        default_visitor::visit_operatorInclusiveRange(node);
      }

      virtual void visit_operatorLeftShift(operatorLeftShift_ast *node)
      {
        std::cout << "operatorLeftShift" << std::endl;
        default_visitor::visit_operatorLeftShift(node);
      }

      virtual void visit_operatorLeftShiftAssign(operatorLeftShiftAssign_ast *node)
      {
        std::cout << "operatorLeftShiftAssign" << std::endl;
        default_visitor::visit_operatorLeftShiftAssign(node);
      }

      virtual void visit_operatorLessOrEqual(operatorLessOrEqual_ast *node)
      {
        std::cout << "operatorLessOrEqual" << std::endl;
        default_visitor::visit_operatorLessOrEqual(node);
      }

      virtual void visit_operatorLessThan(operatorLessThan_ast *node)
      {
        std::cout << "operatorLessThan" << std::endl;
        default_visitor::visit_operatorLessThan(node);
      }

      virtual void visit_operatorLogicalAnd(operatorLogicalAnd_ast *node)
      {
        std::cout << "operatorLogicalAnd" << std::endl;
        default_visitor::visit_operatorLogicalAnd(node);
      }

      virtual void visit_operatorLogicalAndAssign(operatorLogicalAndAssign_ast *node)
      {
        std::cout << "operatorLogicalAndAssign" << std::endl;
        default_visitor::visit_operatorLogicalAndAssign(node);
      }

      virtual void visit_operatorLogicalOr(operatorLogicalOr_ast *node)
      {
        std::cout << "operatorLogicalOr" << std::endl;
        default_visitor::visit_operatorLogicalOr(node);
      }

      virtual void visit_operatorLogicalOrAssign(operatorLogicalOrAssign_ast *node)
      {
        std::cout << "operatorLogicalOrAssign" << std::endl;
        default_visitor::visit_operatorLogicalOrAssign(node);
      }

      virtual void visit_operatorMatch(operatorMatch_ast *node)
      {
        std::cout << "operatorMatch" << std::endl;
        default_visitor::visit_operatorMatch(node);
      }

      virtual void visit_operatorMinus(operatorMinus_ast *node)
      {
        std::cout << "operatorMinus" << std::endl;
        default_visitor::visit_operatorMinus(node);
      }

      virtual void visit_operatorMinusAssign(operatorMinusAssign_ast *node)
      {
        std::cout << "operatorMinusAssign" << std::endl;
        default_visitor::visit_operatorMinusAssign(node);
      }

      virtual void visit_operatorMod(operatorMod_ast *node)
      {
        std::cout << "operatorMod" << std::endl;
        default_visitor::visit_operatorMod(node);
      }

      virtual void visit_operatorModAssign(operatorModAssign_ast *node)
      {
        std::cout << "operatorModAssign" << std::endl;
        default_visitor::visit_operatorModAssign(node);
      }

      virtual void visit_operatorNot(operatorNot_ast *node)
      {
        std::cout << "operatorNot" << std::endl;
        default_visitor::visit_operatorNot(node);
      }

      virtual void visit_operatorNotEqual(operatorNotEqual_ast *node)
      {
        std::cout << "operatorNotEqual" << std::endl;
        default_visitor::visit_operatorNotEqual(node);
      }

      virtual void visit_operatorNotMatch(operatorNotMatch_ast *node)
      {
        std::cout << "operatorNotMatch" << std::endl;
        default_visitor::visit_operatorNotMatch(node);
      }

      virtual void visit_operatorPlus(operatorPlus_ast *node)
      {
        std::cout << "operatorPlus" << std::endl;
        default_visitor::visit_operatorPlus(node);
      }

      virtual void visit_operatorPlusAssign(operatorPlusAssign_ast *node)
      {
        std::cout << "operatorPlusAssign" << std::endl;
        default_visitor::visit_operatorPlusAssign(node);
      }

      virtual void visit_operatorPower(operatorPower_ast *node)
      {
        std::cout << "operatorPower" << std::endl;
        default_visitor::visit_operatorPower(node);
      }

      virtual void visit_operatorPowerAssign(operatorPowerAssign_ast *node)
      {
        std::cout << "operatorPowerAssign" << std::endl;
        default_visitor::visit_operatorPowerAssign(node);
      }

      virtual void visit_operatorQuestion(operatorQuestion_ast *node)
      {
        std::cout << "operatorQuestion" << std::endl;
        default_visitor::visit_operatorQuestion(node);
      }

      virtual void visit_operatorRightShift(operatorRightShift_ast *node)
      {
        std::cout << "operatorRightShift" << std::endl;
        default_visitor::visit_operatorRightShift(node);
      }

      virtual void visit_operatorRightShiftAssign(operatorRightShiftAssign_ast *node)
      {
        std::cout << "operatorRightShiftAssign" << std::endl;
        default_visitor::visit_operatorRightShiftAssign(node);
      }

      virtual void visit_operatorStar(operatorStar_ast *node)
      {
        std::cout << "operatorStar" << std::endl;
        default_visitor::visit_operatorStar(node);
      }

      virtual void visit_operatorStarAssign(operatorStarAssign_ast *node)
      {
        std::cout << "operatorStarAssign" << std::endl;
        default_visitor::visit_operatorStarAssign(node);
      }

      virtual void visit_operatorUnaryMinus(operatorUnaryMinus_ast *node)
      {
        std::cout << "operatorUnaryMinus" << std::endl;
        default_visitor::visit_operatorUnaryMinus(node);
      }

      virtual void visit_operatorUnaryPlus(operatorUnaryPlus_ast *node)
      {
        std::cout << "operatorUnaryPlus" << std::endl;
        default_visitor::visit_operatorUnaryPlus(node);
      }

      virtual void visit_orExpression(orExpression_ast *node)
      {
        std::cout << "orExpression" << std::endl;
        default_visitor::visit_orExpression(node);
      }

      virtual void visit_parallelAssignmentLeftOver(parallelAssignmentLeftOver_ast *node)
      {
        std::cout << "parallelAssignmentLeftOver" << std::endl;
        default_visitor::visit_parallelAssignmentLeftOver(node);
      }

      virtual void visit_powerExpression(powerExpression_ast *node)
      {
        std::cout << "powerExpression" << std::endl;
        default_visitor::visit_powerExpression(node);
      }

      virtual void visit_predefinedValue(predefinedValue_ast *node)
      {
        std::cout << "predefinedValue" << std::endl;
        default_visitor::visit_predefinedValue(node);
      }

      virtual void visit_primaryExpression(primaryExpression_ast *node)
      {
        std::cout << "primaryExpression" << std::endl;
        default_visitor::visit_primaryExpression(node);
      }

      virtual void visit_program(program_ast *node)
      {
        std::cout << "program" << std::endl;
        default_visitor::visit_program(node);
      }

      virtual void visit_rangeExpression(rangeExpression_ast *node)
      {
        std::cout << "rangeExpression" << std::endl;
        default_visitor::visit_rangeExpression(node);
      }

      virtual void visit_regex(regex_ast *node)
      {
        std::cout << "regex" << std::endl;
        default_visitor::visit_regex(node);
      }

      virtual void visit_relationalExpression(relationalExpression_ast *node)
      {
        std::cout << "relationalExpression" << std::endl;
        default_visitor::visit_relationalExpression(node);
      }

      virtual void visit_restMethodDefinitionArgument(restMethodDefinitionArgument_ast *node)
      {
        std::cout << "restMethodDefinitionArgument" << std::endl;
        default_visitor::visit_restMethodDefinitionArgument(node);
      }

      virtual void visit_restMethodInvocationArgument(restMethodInvocationArgument_ast *node)
      {
        std::cout << "restMethodInvocationArgument" << std::endl;
        default_visitor::visit_restMethodInvocationArgument(node);
      }

      virtual void visit_shiftExpression(shiftExpression_ast *node)
      {
        std::cout << "shiftExpression" << std::endl;
        default_visitor::visit_shiftExpression(node);
      }

      virtual void visit_statement(statement_ast *node)
      {
        std::cout << "statement" << std::endl;
        default_visitor::visit_statement(node);
      }

      virtual void visit_statementWithoutModifier(statementWithoutModifier_ast *node)
      {
        std::cout << "statementWithoutModifier" << std::endl;
        default_visitor::visit_statementWithoutModifier(node);
      }

      virtual void visit_statements(statements_ast *node)
      {
        std::cout << "statements" << std::endl;
        default_visitor::visit_statements(node);
      }

      virtual void visit_string(string_ast *node)
      {
        std::cout << "string" << std::endl;
        default_visitor::visit_string(node);
      }

      virtual void visit_symbol(symbol_ast *node)
      {
        std::cout << "symbol" << std::endl;
        default_visitor::visit_symbol(node);
      }

      virtual void visit_terminal(terminal_ast *node)
      {
        std::cout << "terminal" << std::endl;
        default_visitor::visit_terminal(node);
      }

      virtual void visit_ternaryIfThenElseExpression(ternaryIfThenElseExpression_ast *node)
      {
        std::cout << "ternaryIfThenElseExpression" << std::endl;
        default_visitor::visit_ternaryIfThenElseExpression(node);
      }

      virtual void visit_thenOrTerminalOrColon(thenOrTerminalOrColon_ast *node)
      {
        std::cout << "thenOrTerminalOrColon" << std::endl;
        default_visitor::visit_thenOrTerminalOrColon(node);
      }

      virtual void visit_unaryExpression(unaryExpression_ast *node)
      {
        std::cout << "unaryExpression" << std::endl;
        default_visitor::visit_unaryExpression(node);
      }

      virtual void visit_undefParameter(undefParameter_ast *node)
      {
        std::cout << "undefParameter" << std::endl;
        default_visitor::visit_undefParameter(node);
      }

      virtual void visit_unlessExpression(unlessExpression_ast *node)
      {
        std::cout << "unlessExpression" << std::endl;
        default_visitor::visit_unlessExpression(node);
      }

      virtual void visit_untilExpression(untilExpression_ast *node)
      {
        std::cout << "untilExpression" << std::endl;
        default_visitor::visit_untilExpression(node);
      }

      virtual void visit_variable(variable_ast *node)
      {
        std::cout << "variable" << std::endl;
        default_visitor::visit_variable(node);
      }

      virtual void visit_whileExpression(whileExpression_ast *node)
      {
        std::cout << "whileExpression" << std::endl;
        default_visitor::visit_whileExpression(node);
      }

    };

} // end of namespace ruby

#endif



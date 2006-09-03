// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#ifndef ruby_DEFAULT_VISITOR_H_INCLUDED
#define ruby_DEFAULT_VISITOR_H_INCLUDED

#include "ruby_visitor.h"

namespace ruby
  {

  class default_visitor: public visitor
    {
    public:
      virtual void visit_additiveExpression(additiveExpression_ast *node);
      virtual void visit_aliasParameter(aliasParameter_ast *node);
      virtual void visit_andExpression(andExpression_ast *node);
      virtual void visit_andorExpression(andorExpression_ast *node);
      virtual void visit_arrayAccess(arrayAccess_ast *node);
      virtual void visit_arrayExpression(arrayExpression_ast *node);
      virtual void visit_arrayReferenceArgument(arrayReferenceArgument_ast *node);
      virtual void visit_assignmentExpression(assignmentExpression_ast *node);
      virtual void visit_blockContent(blockContent_ast *node);
      virtual void visit_blockMethodDefinitionArgument(blockMethodDefinitionArgument_ast *node);
      virtual void visit_blockMethodInvocationArgument(blockMethodInvocationArgument_ast *node);
      virtual void visit_block_var(block_var_ast *node);
      virtual void visit_block_vars(block_vars_ast *node);
      virtual void visit_bodyStatement(bodyStatement_ast *node);
      virtual void visit_caseExpression(caseExpression_ast *node);
      virtual void visit_classDefinition(classDefinition_ast *node);
      virtual void visit_className(className_ast *node);
      virtual void visit_codeBlock(codeBlock_ast *node);
      virtual void visit_colonAccess(colonAccess_ast *node);
      virtual void visit_command(command_ast *node);
      virtual void visit_commandOutput(commandOutput_ast *node);
      virtual void visit_compoundStatement(compoundStatement_ast *node);
      virtual void visit_doOrTerminalOrColon(doOrTerminalOrColon_ast *node);
      virtual void visit_dotAccess(dotAccess_ast *node);
      virtual void visit_elementReference(elementReference_ast *node);
      virtual void visit_equalityExpression(equalityExpression_ast *node);
      virtual void visit_exceptionHandlingExpression(exceptionHandlingExpression_ast *node);
      virtual void visit_exceptionList(exceptionList_ast *node);
      virtual void visit_expression(expression_ast *node);
      virtual void visit_expressionSubstitution(expressionSubstitution_ast *node);
      virtual void visit_forExpression(forExpression_ast *node);
      virtual void visit_hashExpression(hashExpression_ast *node);
      virtual void visit_ifExpression(ifExpression_ast *node);
      virtual void visit_keyValuePair(keyValuePair_ast *node);
      virtual void visit_keyword(keyword_ast *node);
      virtual void visit_keywordAsMethodName(keywordAsMethodName_ast *node);
      virtual void visit_literal(literal_ast *node);
      virtual void visit_logicalAndExpression(logicalAndExpression_ast *node);
      virtual void visit_logicalOrExpression(logicalOrExpression_ast *node);
      virtual void visit_methodCall(methodCall_ast *node);
      virtual void visit_methodDefinition(methodDefinition_ast *node);
      virtual void visit_methodDefinitionArgument(methodDefinitionArgument_ast *node);
      virtual void visit_methodDefinitionArgumentWithoutParen(methodDefinitionArgumentWithoutParen_ast *node);
      virtual void visit_methodInvocationArgumentWithParen(methodInvocationArgumentWithParen_ast *node);
      virtual void visit_methodInvocationArgumentWithoutParen(methodInvocationArgumentWithoutParen_ast *node);
      virtual void visit_methodName(methodName_ast *node);
      virtual void visit_methodNameSupplement(methodNameSupplement_ast *node);
      virtual void visit_mlhs_item(mlhs_item_ast *node);
      virtual void visit_moduleDefinition(moduleDefinition_ast *node);
      virtual void visit_moduleName(moduleName_ast *node);
      virtual void visit_mrhs(mrhs_ast *node);
      virtual void visit_multiplicativeExpression(multiplicativeExpression_ast *node);
      virtual void visit_normalMethodDefinitionArgument(normalMethodDefinitionArgument_ast *node);
      virtual void visit_normalMethodInvocationArgument(normalMethodInvocationArgument_ast *node);
      virtual void visit_notExpression(notExpression_ast *node);
      virtual void visit_numeric(numeric_ast *node);
      virtual void visit_operatorAsMethodname(operatorAsMethodname_ast *node);
      virtual void visit_orExpression(orExpression_ast *node);
      virtual void visit_parallelAssignmentLeftOver(parallelAssignmentLeftOver_ast *node);
      virtual void visit_powerExpression(powerExpression_ast *node);
      virtual void visit_predefinedValue(predefinedValue_ast *node);
      virtual void visit_primaryExpression(primaryExpression_ast *node);
      virtual void visit_program(program_ast *node);
      virtual void visit_rangeExpression(rangeExpression_ast *node);
      virtual void visit_regex(regex_ast *node);
      virtual void visit_relationalExpression(relationalExpression_ast *node);
      virtual void visit_restMethodDefinitionArgument(restMethodDefinitionArgument_ast *node);
      virtual void visit_restMethodInvocationArgument(restMethodInvocationArgument_ast *node);
      virtual void visit_shiftExpression(shiftExpression_ast *node);
      virtual void visit_statement(statement_ast *node);
      virtual void visit_statementWithoutModifier(statementWithoutModifier_ast *node);
      virtual void visit_statements(statements_ast *node);
      virtual void visit_string(string_ast *node);
      virtual void visit_symbol(symbol_ast *node);
      virtual void visit_terminal(terminal_ast *node);
      virtual void visit_ternaryIfThenElseExpression(ternaryIfThenElseExpression_ast *node);
      virtual void visit_thenOrTerminalOrColon(thenOrTerminalOrColon_ast *node);
      virtual void visit_unaryExpression(unaryExpression_ast *node);
      virtual void visit_undefParameter(undefParameter_ast *node);
      virtual void visit_unlessExpression(unlessExpression_ast *node);
      virtual void visit_untilExpression(untilExpression_ast *node);
      virtual void visit_variable(variable_ast *node);
      virtual void visit_whileExpression(whileExpression_ast *node);
    };

} // end of namespace ruby

#endif



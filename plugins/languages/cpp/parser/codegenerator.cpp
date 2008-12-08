/*
   Copyright 2008 Hamish Rodda <rodda@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "codegenerator.h"

#include "tokens.h"
#include "lexer.h"
#include "parsesession.h"

/*

Thoughts.
- tokens are actually the position in the token stream, whose position can be determined, and thus replaced.
- we need to be able to make changes where there are changes [todo], or generate directly when it's a new node [done]

*/

CodeGenerator::CodeGenerator(ParseSession* session)
  : m_output(&m_outputString)
  , m_session(session)
{
}

CodeGenerator::~CodeGenerator()
{
}

QString CodeGenerator::output()
{
  m_output.flush();
  return m_outputString;
}

void CodeGenerator::outputToken(std::size_t tokenPosition)
{
  if (tokenPosition) {
    const Token& t = m_session->token_stream->token(tokenPosition);
    m_output << t.symbolString();/*
    if (t.kind == Token_identifier || t.kind == Token_string_literal || t.kind == Token_number_literal || t.kind == Token_char_literal)
      m_output << t.symbolString().str();
    else
      m_output << token_text( t.kind );*/
  }
}

void CodeGenerator::print(const ListNode<std::size_t>* tokenList, bool followingSpace)
{
  if (!tokenList)
    return;

  const ListNode<std::size_t>* it = tokenList->toFront(), *end = it;
  bool first = true;
  do {
    if (first) first = false; else m_output << " ";
    outputToken(it->element);
    it = it->next;
  } while (it != end);

  if (followingSpace)
    m_output << " ";
}

void CodeGenerator::print(std::size_t token, bool followingSpace)
{
  if (!token)
    return;

  outputToken(token);
  if (followingSpace)
    m_output << " ";
}

void CodeGenerator::printToken(int token, bool followingSpace)
{
  m_output << token_text(token);
  if (followingSpace)
    m_output << " ";
}


void CodeGenerator::visitAccessSpecifier(AccessSpecifierAST* node)
{
  print(node->specs);
  m_output << ":";
}

void CodeGenerator::visitAsmDefinition(AsmDefinitionAST* node)
{
  print(node->cv);

  DefaultVisitor::visitAsmDefinition(node);
}

void CodeGenerator::visitBaseClause(BaseClauseAST* node)
{
  m_output << ":";

  commaPrintNodes(this, node->base_specifiers);
}

void CodeGenerator::visitBaseSpecifier(BaseSpecifierAST* node)
{
  if (node->virt) {
    print(node->virt, true);
  }

  print(node->access_specifier, true);

  DefaultVisitor::visitBaseSpecifier(node);
}

void CodeGenerator::visitBinaryExpression(BinaryExpressionAST* node)
{
  visit(node->left_expression);

  // TODO whitespace
  print(node->op);

  visit(node->right_expression);
}

void CodeGenerator::visitCastExpression(CastExpressionAST* node)
{
  m_output << "(";
  visit(node->type_id);
  m_output << ")";
  visit(node->expression);
}

void CodeGenerator::visitClassMemberAccess(ClassMemberAccessAST* node)
{
  print(node->op);
  DefaultVisitor::visitClassMemberAccess(node);
}

void CodeGenerator::visitClassSpecifier(ClassSpecifierAST* node)
{
  print(node->class_key, true);

  visit(node->win_decl_specifiers);
  visit(node->name);
  visit(node->base_clause);

  m_output << "{";

  visitNodes(this, node->member_specs);

  m_output << "}";
}

void CodeGenerator::visitCompoundStatement(CompoundStatementAST* node)
{
  m_output << "{";

  DefaultVisitor::visitCompoundStatement(node);

  m_output << "}";
}

void CodeGenerator::visitCondition(ConditionAST* node)
{
  if (node->declarator) {
    visit(node->type_specifier);
    visit(node->declarator);

    m_output << "=";
  }

  visit(node->expression);
}

void CodeGenerator::visitConditionalExpression(ConditionalExpressionAST* node)
{
  visit(node->condition);

  m_output << "?";

  visit(node->left_expression);

  m_output << ":";

  visit(node->right_expression);
}

void CodeGenerator::visitCppCastExpression(CppCastExpressionAST* node)
{
  print(node->op);

  m_output << "<";

  visit(node->type_id);

  m_output << ">(";

  visit(node->expression);

  m_output << ")";

  visitNodes(this, node->sub_expressions);
}

void CodeGenerator::visitCtorInitializer(CtorInitializerAST* node)
{
  m_output << ":";

  visitCommaPrint(node->member_initializers);
}

void CodeGenerator::visitDeclarationStatement(DeclarationStatementAST* node)
{
  DefaultVisitor::visitDeclarationStatement(node);
}

void CodeGenerator::visitDeclarator(DeclaratorAST* node)
{
  if (node->sub_declarator) {
    m_output << "(";
    visit(node->sub_declarator);
    m_output << ")";
  }

  visitNodes(this, node->ptr_ops);
  visit(node->id);

  if (node->bit_expression) {
    m_output << ":";
    visit(node->bit_expression);
  }

  surroundPrintNodes(this, node->array_dimensions, "[", "]");

  if (node->parameter_declaration_clause) {
    m_output << "(";
    visit(node->parameter_declaration_clause);
    m_output << ")";
  }

  print(node->fun_cv, true);

  visit(node->exception_spec);
}

void CodeGenerator::visitDeleteExpression(DeleteExpressionAST* node)
{
  print( node->scope_token );
  print( node->delete_token );
  print( node->lbracket_token );
  print( node->rbracket_token );

  m_output << " ";

  DefaultVisitor::visitDeleteExpression(node);
}

void CodeGenerator::visitDoStatement(DoStatementAST* node)
{
  printToken( Token_do, true );

  visit(node->statement);

  printToken( Token_while );
  m_output << "(";

  visit(node->expression);

  m_output << ");";
}

void CodeGenerator::visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST* node)
{
  print(node->type, true);

  DefaultVisitor::visitElaboratedTypeSpecifier(node);
}

void CodeGenerator::visitEnumSpecifier(EnumSpecifierAST* node)
{
  printToken(Token_enum, true);

  visit(node->name);

  if (node->enumerators) {
    m_output << "{";
    visitCommaPrint(node->enumerators);
    m_output << "}";
  }
}

void CodeGenerator::visitEnumerator(EnumeratorAST* node)
{
  print(node->id);

  if (node->expression) {
    m_output << "=";

    visit(node->expression);
  }
}

void CodeGenerator::visitExceptionSpecification(ExceptionSpecificationAST* node)
{
  printToken(Token_throw);

  m_output << "(";

  print(node->ellipsis);

  DefaultVisitor::visitExceptionSpecification(node);

  m_output << ")";
}

void CodeGenerator::visitExpressionOrDeclarationStatement(ExpressionOrDeclarationStatementAST* node)
{
  DefaultVisitor::visitExpressionOrDeclarationStatement(node);
}

void CodeGenerator::visitExpressionStatement(ExpressionStatementAST* node)
{
  DefaultVisitor::visitExpressionStatement(node);
  m_output << ";";
}

void CodeGenerator::visitForStatement(ForStatementAST* node)
{
  printToken(Token_for);
  m_output << "(";
  if (node->init_statement) {
    visit(node->init_statement);
    // Init statement gives its own ;
  } else {
    m_output << ";";
  }

  visit(node->condition);
  m_output << ";";
  visit(node->expression);
  m_output << ")";
  visit(node->statement);
}

void CodeGenerator::visitFunctionCall(FunctionCallAST* node)
{
  m_output << "(";
  DefaultVisitor::visitFunctionCall(node);
  m_output << ")";
}

void CodeGenerator::visitFunctionDefinition(FunctionDefinitionAST* node)
{
  visit(node->type_specifier);
  visit(node->init_declarator);
  visit(node->constructor_initializers);
  visit(node->function_body);
  visit(node->win_decl_specifiers);
}

void CodeGenerator::visitIfStatement(IfStatementAST* node)
{
  printToken(Token_if);

  m_output << "(";
  visit(node->condition);
  m_output << ")";

  visit(node->statement);

  if (node->else_statement) {
    printToken(Token_else, true);
    visit(node->else_statement);
  }
}

void CodeGenerator::visitIncrDecrExpression(IncrDecrExpressionAST* node)
{
  print(node->op);
}

void CodeGenerator::visitInitDeclarator(InitDeclaratorAST* node)
{
  DefaultVisitor::visitInitDeclarator(node);
}

void CodeGenerator::visitInitializer(InitializerAST* node)
{
  if (node->initializer_clause) {
    m_output << "=";
    visit(node->initializer_clause);

  } else if (node->expression) {
    m_output << "(";
    visit(node->expression);
    m_output << ")";
  }
}

void CodeGenerator::visitInitializerClause(InitializerClauseAST* node)
{
  if (node->initializer_list) {
    m_output << "{";
    visitCommaPrint(node->initializer_list);
    m_output << "}";

  } else {
    visit(node->expression);
  }
}

void CodeGenerator::visitJumpStatement(JumpStatementAST* node)
{
  print(node->op, true);
  print(node->identifier);
  m_output << ";";
}

void CodeGenerator::visitLabeledStatement(LabeledStatementAST* node)
{
  print(node->label, true);

  visit(node->expression);

  m_output << ":";

  visit(node->statement);
}

void CodeGenerator::visitLinkageBody(LinkageBodyAST* node)
{
  m_output << "{";

  DefaultVisitor::visitLinkageBody(node);

  m_output << "}";
}

void CodeGenerator::visitLinkageSpecification(LinkageSpecificationAST* node)
{
  printToken(Token_extern, true);
  print(node->extern_type, true);

  DefaultVisitor::visitLinkageSpecification(node);
}

void CodeGenerator::visitMemInitializer(MemInitializerAST* node)
{
  visit(node->initializer_id);

  m_output << "(";
  visit(node->expression);
  m_output << ")";
}

void CodeGenerator::visitName(NameAST* node)
{
  if (node->global)
    printToken( Token_scope );

  if (node->qualified_names) {
    commaPrintNodes( this, node->qualified_names, token_text( Token_scope ) );
    printToken( Token_scope );
  }

  visit(node->unqualified_name);
}

void CodeGenerator::visitNamespace(NamespaceAST* node)
{
  printToken( Token_namespace, true );

  print(node->namespace_name);

  DefaultVisitor::visitNamespace(node);
}

void CodeGenerator::visitNamespaceAliasDefinition(NamespaceAliasDefinitionAST* node)
{
  printToken( Token_namespace, true );

  print(node->namespace_name);

  m_output << "=";

  DefaultVisitor::visitNamespaceAliasDefinition(node);

  m_output << ";";
}

void CodeGenerator::visitNewDeclarator(NewDeclaratorAST* node)
{
  visit(node->ptr_op);
  visit(node->sub_declarator);
  surroundPrintNodes(this, node->expressions, "[", "]");
}

void CodeGenerator::visitNewExpression(NewExpressionAST* node)
{
  print(node->scope_token);
  print(node->new_token, true);

  if (node->expression) {
    m_output << "(";
    visit(node->expression);
    m_output << ")";
  }

  if (node->type_id) {
    m_output << "(";
    visit(node->type_id);
    m_output << ")";
  }

  visit(node->new_type_id);
  visit(node->new_initializer);
}

void CodeGenerator::visitNewInitializer(NewInitializerAST* node)
{
  m_output << "(";
  DefaultVisitor::visitNewInitializer(node);
  m_output << ")";
}

void CodeGenerator::visitNewTypeId(NewTypeIdAST* node)
{
  DefaultVisitor::visitNewTypeId(node);
}

void CodeGenerator::visitOperator(OperatorAST* node)
{
  print(node->op);

  if (node->op != node->open)
    print(node->open);

  print(node->close);

  DefaultVisitor::visitOperator(node);
}

void CodeGenerator::visitOperatorFunctionId(OperatorFunctionIdAST* node)
{
  printToken(Token_operator, true);

  DefaultVisitor::visitOperatorFunctionId( node );
}

void CodeGenerator::visitParameterDeclaration(ParameterDeclarationAST* node)
{
  if (node->type_specifier) {
    visit(node->type_specifier);
    m_output << " ";
  }
  visit(node->declarator);

  if (node->expression) {
    m_output << "=";
    visit(node->expression);
  }
}

void CodeGenerator::visitParameterDeclarationClause(ParameterDeclarationClauseAST* node)
{
  commaPrintNodes(this, node->parameter_declarations);

  // TODO add elipsis
}

void CodeGenerator::visitPostfixExpression(PostfixExpressionAST* node)
{
  DefaultVisitor::visitPostfixExpression(node);
}

void CodeGenerator::visitPrimaryExpression(PrimaryExpressionAST* node)
{
  print(node->token);

  visit(node->literal);

  if (node->expression_statement || node->sub_expression) {
    m_output << "(";
    visit(node->expression_statement);
    visit(node->sub_expression);
    m_output << ")";
  }

  visit(node->name);
}

void CodeGenerator::visitPtrOperator(PtrOperatorAST* node)
{
  print(node->op);

  DefaultVisitor::visitPtrOperator(node);

  print(node->cv);
}

void CodeGenerator::visitPtrToMember(PtrToMemberAST* node)
{
  // TODO fix AST
  printToken(Token_scope);
  m_output << "*";
}

void CodeGenerator::visitReturnStatement(ReturnStatementAST* node)
{
  printToken(Token_return, true);

  DefaultVisitor::visitReturnStatement(node);

  m_output << ";";
}

void CodeGenerator::visitSimpleDeclaration(SimpleDeclarationAST* node)
{
  print(node->storage_specifiers, true);
  print(node->function_specifiers, true);
  visit(node->type_specifier);
  commaPrintNodes(this, node->init_declarators);
  visit(node->win_decl_specifiers);

  m_output << ";";
}

void CodeGenerator::visitSimpleTypeSpecifier(SimpleTypeSpecifierAST* node)
{
  print(node->cv, true);

  print(node->integrals, true);

  print(node->type_of);

  if (node->name) {
    visit(node->name);
    m_output << " ";
  }

  if (node->type_id) {
    m_output << "(";
    visit(node->type_id);
    m_output << ")";
  }

  visit(node->expression);
}

void CodeGenerator::visitSizeofExpression(SizeofExpressionAST* node)
{
  printToken(Token_sizeof);

  if (node->type_id) {
    m_output << "(";
    visit(node->type_id);
    m_output << ")";
  }

  visit(node->expression);
}

void CodeGenerator::visitStringLiteral(StringLiteralAST* node)
{
  print(node->literals);
}

void CodeGenerator::visitSubscriptExpression(SubscriptExpressionAST* node)
{
  m_output << "[";
  DefaultVisitor::visitSubscriptExpression(node);
  m_output << "]";
}

void CodeGenerator::visitSwitchStatement(SwitchStatementAST* node)
{
  printToken(Token_switch);

  m_output << "(";
  visit(node->condition);
  m_output << ")";

  visit(node->statement);
}

void CodeGenerator::visitTemplateArgument(TemplateArgumentAST* node)
{
  DefaultVisitor::visitTemplateArgument(node);
}

void CodeGenerator::visitTemplateDeclaration(TemplateDeclarationAST* node)
{
  if (node->exported) {
    printToken(Token_export, true);
  }

  printToken(Token_template, true);

  if (node->template_parameters) {
    m_output << "< ";
    commaPrintNodes(this, node->template_parameters);
    m_output << " >";
  }

  visit(node->declaration);
}

void CodeGenerator::visitTemplateParameter(TemplateParameterAST* node)
{
  DefaultVisitor::visitTemplateParameter(node);
}

void CodeGenerator::visitThrowExpression(ThrowExpressionAST* node)
{
  printToken(Token_throw);

  DefaultVisitor::visitThrowExpression(node);
}

void CodeGenerator::visitTranslationUnit(TranslationUnitAST* node)
{
  DefaultVisitor::visitTranslationUnit(node);
}

void CodeGenerator::visitTryBlockStatement(TryBlockStatementAST* node)
{
  printToken(Token_try);
  visit(node->try_block);
  visitNodes(this, node->catch_blocks);
}

void CodeGenerator::visitCatchStatement(CatchStatementAST* node)
{
  printToken(Token_catch);

  m_output << "(";
  visit(node->condition);
  m_output << ")";

  visit(node->statement);
}

void CodeGenerator::visitTypeId(TypeIdAST* node)
{
  DefaultVisitor::visitTypeId(node);
}

void CodeGenerator::visitTypeIdentification(TypeIdentificationAST* node)
{
  print(node->typename_token);

  visit(node->name);

  if (node->expression) {
    m_output << "(";
    visit(node->expression);
    m_output << ")";
  }
}

void CodeGenerator::visitTypeParameter(TypeParameterAST* node)
{
  print(node->type, true);

  visit(node->name);

  if (node->type_id) {
    m_output << "=";
    visit(node->type_id);
  }

  if (node->template_parameters) {
    m_output << "<";
    commaPrintNodes(this, node->template_parameters);
    m_output << ">";

    // TODO when AST ready: if (node->template_class_token) printToken(Token_class);
  }

  if (node->template_name) {
    m_output << "=";
    visit(node->template_name);
  }
}

void CodeGenerator::visitTypedef(TypedefAST* node)
{
  printToken( Token_typedef );

  visit(node->type_specifier);

  commaPrintNodes(this, node->init_declarators);

  m_output << ";";
}

void CodeGenerator::visitUnaryExpression(UnaryExpressionAST* node)
{
  print(node->op);

  DefaultVisitor::visitUnaryExpression(node);
}

void CodeGenerator::visitUnqualifiedName(UnqualifiedNameAST* node)
{
  print(node->tilde);
  print(node->id);

  visit(node->operator_id);

  if (node->template_arguments) {
    m_output << "< ";
    visitNodes(this, node->template_arguments);
    m_output << " >";
  }
}

void CodeGenerator::visitUsing(UsingAST* node)
{
  printToken(Token_using, true);
  print(node->type_name);

  DefaultVisitor::visitUsing(node);

  m_output << ";";
}

void CodeGenerator::visitUsingDirective(UsingDirectiveAST* node)
{
  printToken(Token_using, true);
  printToken(Token_namespace, true);

  DefaultVisitor::visitUsingDirective(node);

  m_output << ";";
}

void CodeGenerator::visitWhileStatement(WhileStatementAST* node)
{
  printToken(Token_while);
  m_output << "(";
  visit(node->condition);
  m_output << ")";

  visit(node->statement);
}

void CodeGenerator::visitWinDeclSpec(WinDeclSpecAST* node)
{
  print(node->specifier);
  m_output << "(";
  print(node->modifier);
  m_output << ")";
}


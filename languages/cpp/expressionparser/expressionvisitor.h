/* This file is part of KDevelop
    Copyright (C) 2007 David Nolden [david.nolden.kdevelop  art-master.de]

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

#ifndef EXPRESSIONVISITOR_H
#define EXPRESSIONVISITOR_H

#include <typesystem.h>
#include "visitor.h"
#include "cppexpressionparserexport.h"


namespace KDevelop {
  class Declaration;
};

class Token;
class TranslationUnitAST;
class AST;

namespace Cpp {
using namespace KDevelop;

/** A type-builder must have been run already over the ast.
 *
 *  The code is somewhat redundant with UseBuilder. Maybe UseBuilder should be based on this class. However a TypeBuilder must have been run already on the code, so the declarations of uses are located.
**/
class KDEVCPPEXPRESSIONPARSER_EXPORT ExpressionVisitor : public Visitor {
  public:
    ExpressionVisitor( ParseSession* session );
    ~ExpressionVisitor();

    /**
     * Will parse the tree and call expressionType(..) for each successfully evaluated type
     * @param ast the syntax-tree to evaluate. The context must already be built.
     **/
    void parse( AST* ast );

    AbstractType::Ptr lastType();
    Declaration* lastDeclaration();

    ParseSession* session();
  protected:
    /**
     * Will be called for each relevant sub-node with the resolved type of that expression. This is not guaranteed to be called.
     * There is also no guarantee in which order expressionType() will be called.
     * The du-chain will not be locked in the moment this is called.
     *
     * @param ast the AST-Node
     * @param type the type the expression in the AST-node evaluates to
     * @param decl If the expression evaluates to an instance of a type, this is the declaration of that instance.
     * If this is zero, the expression evaluates to a type.
     * examples:
     * the expression "AbstractType::Ptr" evaluates to a type, so @param type would be filled and @param decl would be zero.
     * When the context contains "AbstractType::Ptr ptr;", the expression "ptr" will evaluate to an instance of
     * AbstractType::Ptr, so @param type will contain the type AbstractType::Ptr, and @param decl will point to the declaration of ptr.
     * 
     **/
    virtual void expressionType( AST* node, const AbstractType::Ptr& type, Declaration* decl ) {
    }

    /** Called when there is a problem, with a string for that problem.
     * The default-implementation dumps all relevant information to
     * kdDebug.
     * @param node the node the problem is about
     * @param str a string that describes the problem
     */
    virtual void problem( AST* node, const QString& str );
    
  private:
    AbstractType::Ptr m_lastType;
    Declaration* m_lastDeclaration;
    ParseSession* m_session;

  inline void clearLast() {
    m_lastDeclaration = 0;
    m_lastType = 0;
  }

  const Token& tokenFromIndex( int index );

  /** If the member was found, it is returned through m_lastType and m_lastDeclaration. On fail those are zero.
   * du-chain must not be locked.
   * @param node that issues the command. Only used for posting problems.
   * @param type The structure the member should be searched in
   * @param member Identifier of the member to search
   * @param isConst Whether the type inherits const-ness from above
   * @param postError whether a problem should be posted when the member is not found
   */
  void findMember( AST* node, AbstractType::Ptr type, const QualifiedIdentifier& member, bool isConst = false, bool postProblem = true );

  /**
   *  Visits post-fix sub-expressions.
   * m_lastType and m_lastDeclaration must be filled with the types where to start.
   * When the evaluation is successful, expressionType(..) is called with the resulting type.
   * @param node The basic AST-node which will only be used for reporting problems
   * @param nodes the nodes to visit
   **/
  void visitSubExpressions( AST* node, const ListNode<ExpressionAST*>* nodes );

  virtual void visitAccessSpecifier(AccessSpecifierAST *) ;
  virtual void visitAsmDefinition(AsmDefinitionAST *) ;
  virtual void visitBaseClause(BaseClauseAST *) ;
  virtual void visitBaseSpecifier(BaseSpecifierAST *) ;
  virtual void visitBinaryExpression(BinaryExpressionAST *) ;
  virtual void visitCastExpression(CastExpressionAST *) ;
  virtual void visitClassMemberAccess(ClassMemberAccessAST *) ;
  virtual void visitClassSpecifier(ClassSpecifierAST *) ;
  virtual void visitCompoundStatement(CompoundStatementAST *) ;
  virtual void visitCondition(ConditionAST *) ;
  virtual void visitConditionalExpression(ConditionalExpressionAST *) ;
  virtual void visitCppCastExpression(CppCastExpressionAST *) ;
  virtual void visitCtorInitializer(CtorInitializerAST *) ;
  virtual void visitDeclarationStatement(DeclarationStatementAST *) ;
  virtual void visitDeclarator(DeclaratorAST *) ;
  virtual void visitDeleteExpression(DeleteExpressionAST *) ;
  virtual void visitDoStatement(DoStatementAST *) ;
  virtual void visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST *) ;
  virtual void visitEnumSpecifier(EnumSpecifierAST *) ;
  virtual void visitEnumerator(EnumeratorAST *) ;
  virtual void visitExceptionSpecification(ExceptionSpecificationAST *) ;
  virtual void visitExpressionOrDeclarationStatement(ExpressionOrDeclarationStatementAST *) ;
  virtual void visitExpressionStatement(ExpressionStatementAST *) ;
  virtual void visitForStatement(ForStatementAST *) ;
  virtual void visitFunctionCall(FunctionCallAST *) ;
  virtual void visitFunctionDefinition(FunctionDefinitionAST *) ;
  virtual void visitIfStatement(IfStatementAST *) ;
  virtual void visitIncrDecrExpression(IncrDecrExpressionAST *) ;
  virtual void visitInitDeclarator(InitDeclaratorAST *) ;
  virtual void visitInitializer(InitializerAST *) ;
  virtual void visitInitializerClause(InitializerClauseAST *) ;
  virtual void visitLabeledStatement(LabeledStatementAST *) ;
  virtual void visitLinkageBody(LinkageBodyAST *) ;
  virtual void visitLinkageSpecification(LinkageSpecificationAST *) ;
  virtual void visitMemInitializer(MemInitializerAST *) ;
  virtual void visitName(NameAST *) ;
  virtual void visitNamespace(NamespaceAST *) ;
  virtual void visitNamespaceAliasDefinition(NamespaceAliasDefinitionAST *) ;
  virtual void visitNewDeclarator(NewDeclaratorAST *) ;
  virtual void visitNewExpression(NewExpressionAST *) ;
  virtual void visitNewInitializer(NewInitializerAST *) ;
  virtual void visitNewTypeId(NewTypeIdAST *) ;
  virtual void visitOperator(OperatorAST *) ;
  virtual void visitOperatorFunctionId(OperatorFunctionIdAST *) ;
  virtual void visitParameterDeclaration(ParameterDeclarationAST *) ;
  virtual void visitParameterDeclarationClause(ParameterDeclarationClauseAST *) ;
  virtual void visitPostfixExpression(PostfixExpressionAST *) ;
  virtual void visitPrimaryExpression(PrimaryExpressionAST *) ;
  virtual void visitPtrOperator(PtrOperatorAST *) ;
  virtual void visitPtrToMember(PtrToMemberAST *) ;
  virtual void visitReturnStatement(ReturnStatementAST *) ;
  virtual void visitSimpleDeclaration(SimpleDeclarationAST *) ;
  virtual void visitSimpleTypeSpecifier(SimpleTypeSpecifierAST *) ;
  virtual void visitSizeofExpression(SizeofExpressionAST *) ;
  virtual void visitStringLiteral(StringLiteralAST *) ;
  virtual void visitSubscriptExpression(SubscriptExpressionAST *) ;
  virtual void visitSwitchStatement(SwitchStatementAST *) ;
  virtual void visitTemplateArgument(TemplateArgumentAST *) ;
  virtual void visitTemplateDeclaration(TemplateDeclarationAST *) ;
  virtual void visitTemplateParameter(TemplateParameterAST *) ;
  virtual void visitThrowExpression(ThrowExpressionAST *) ;
  virtual void visitTranslationUnit(TranslationUnitAST *) ;
  virtual void visitTryBlockStatement(TryBlockStatementAST *) ;
  virtual void visitTypeId(TypeIdAST *) ;
  virtual void visitTypeIdentification(TypeIdentificationAST *) ;
  virtual void visitTypeParameter(TypeParameterAST *) ;
  virtual void visitTypedef(TypedefAST *) ;
  virtual void visitUnaryExpression(UnaryExpressionAST *) ;
  virtual void visitUnqualifiedName(UnqualifiedNameAST *) ;
  virtual void visitUsing(UsingAST *) ;
  virtual void visitUsingDirective(UsingDirectiveAST *) ;
  virtual void visitWhileStatement(WhileStatementAST *) ;
  virtual void visitWinDeclSpec(WinDeclSpecAST *) ;
};
}

#endif

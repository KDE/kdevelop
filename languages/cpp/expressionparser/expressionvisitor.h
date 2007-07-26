/*
   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

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
#include "overloadresolution.h" /* needed for OverloadResover::Parameter */


namespace KDevelop {
  class Declaration;
  class DUContext;
}

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
    /**
     * @param strict When this is false, the expression-visitor tries to recover from problems. For example when it cannot find a matching function, it returns the first of the candidates.
     * */
    explicit ExpressionVisitor( ParseSession* session, bool strict = false );
    ~ExpressionVisitor();

    struct Instance {
      Instance() : isInstance(false), declaration(0) {
      }
      Instance( bool is ) : isInstance(is), declaration(0) {
      }
      Instance( Declaration* decl ) : isInstance(true), declaration(decl) {
      }
      inline operator bool() const {
        return isInstance;
      }

      bool isInstance;
      Declaration* declaration; //May contain the declaration of the instance, but only when isInstance is true. May also contain type-declaration, which signalizes that this is an instance of that type.
    };

    /**
     * Will parse the tree and call expressionType(..) for each successfully evaluated type
     * @param ast the syntax-tree to evaluate. The context must already be built, the given AST needs to have a filled ducontext.
     **/
    void parse( AST* ast );

    AbstractType::Ptr lastType();
    Instance lastInstance();

    ParseSession* session();

    ///Returns the last queried list of declarations
    QList<Declaration*> lastDeclarations() const;
  protected:
    /**
     * Will be called for each relevant sub-node with the resolved type of that expression. This is not guaranteed to be called.
     * There is also no guarantee in which order expressionType() will be called.
     * The du-chain will not be locked in the moment this is called.
     *
     * @param node the AST-Node
     * @param type the type the expression in the AST-node evaluates to
     * @param instance If the expression evaluates to an instance of a type, this contains information about that instance. declaration is only filled for explicitly declared instances.
     * If this is zero, the expression evaluates to a type.
     *
     * Warning:
     * In case of temporary instances, decl will be the declaration of the basic type, not of an instance.
     * Since temporary instances are never declared, there's no other way.
     *
     * examples:
     * the expression "AbstractType::Ptr" evaluates to a type, so @param type would be filled and @param decl would be zero.
     * When the context contains "AbstractType::Ptr ptr;", the expression "ptr" will evaluate to an instance of
     * AbstractType::Ptr, so @param type will contain the type AbstractType::Ptr, and @param decl will point to the declaration of ptr.
     *
     * Problem:
     **/
    virtual void expressionType( AST* node, const AbstractType::Ptr& type, Instance instance ) {
      Q_UNUSED(node) Q_UNUSED(type) Q_UNUSED(instance)
    }

    /** Called when there is a problem, with a string for that problem.
     * The default-implementation dumps all relevant information to
     * kdDebug.
     * @param node the node the problem is about
     * @param str a string that describes the problem
     */
    virtual void problem( AST* node, const QString& str );

  private:
    bool m_strict;
    AbstractType::Ptr m_lastType;
    Instance m_lastInstance; //Contains whether the last evaluation resulted in an instance, and maybe the instance-declaration

    //Whenever a list of declarations is queried, it is stored here. Especially in visitName(...) and findMember(...)
    QList<Declaration*> m_lastDeclarations;

    //Here the parameters of function-calls are collected
    //When a parameter could not be evaluated, this will hold a parameter with null-value type
    QList<OverloadResolver::Parameter> m_parameters;

    ParseSession* m_session;
    KDevelop::DUContext* m_currentContext;

  inline void clearLast() {
    m_lastInstance = Instance();
    m_lastType = 0;
  }

  ///Returns whether the given type and instance-info are an lvalue
  bool isLValue( const AbstractType::Ptr& type, const Instance& instance );


  /**
   * Returns the dereferenced type(example: ReferenceType(PointerType) -> PointerType)
   *
   *
   *  !!DU-Chain must be locked!
  * @param constant will be set to true when one of the references made the result constant
   * @return return-value will only be zero if m_lastType is zero
   */
  AbstractType::Ptr realLastType(bool* constant = 0) const;

  /**
   * Returns true when m_lastType either is a pointer, or a reference to a pointer
   *
   *  !!DU-Chain must be locked!
   **/
  bool isPointerType() const;

  /**
   *  m_lastType must be a pointer. It will be dereferenced in m_lastType,
   *  and m_lastDeclaration filled appropriately.
   *
   *  !!DU-Chain must be locked!
   *
   *  @param node Node shown while reporting problems
   *  @param constant will be set to true if something constant is dereferenced
   *  @return false on fail(for example type is no pointer)
   */
  bool getPointerTarget( AST* node, bool* constant = 0 );

  /**
   *  !!DU-Chain must be locked!
   **/
  Declaration* getDeclaration( AST* node, const AbstractType::Ptr& base );

  /**
   *  m_lastType must be a function.
   *  Return-value will be extracted, m_lastType and m_lastDeclaration
   *  filled appropriately.
   *
   *  On failure, m_lastType will be 0.
   *
   *  !!DU-Chain must be locked!
   *
   *  @param node Node shown while reporting problems
   *  @return false on fail(for example type is no function)
   */
  void getReturnValue( AST* node );

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

  ///Visits all nodes, and resets m_lastType and m_lastDeclaration to the previous values before each visit so they cannot influence each other
  template <class _Tp>
  void visitIndependentNodes(const ListNode<_Tp> *nodes);
};
}

#endif

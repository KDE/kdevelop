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

#include <language/duchain/types/abstracttype.h>
#include <language/duchain/declaration.h>
#include <language/interfaces/iproblem.h>
#include "default_visitor.h"
#include "cppduchainexport.h"
#include "overloadresolution.h" /* needed for OverloadResover::Parameter */
#include "cpptypes.h"

namespace KDevelop {
  class Declaration;
  class DUContext;
  class TopDUContext;
  class Problem;
}

class Token;

namespace Cpp {
using namespace KDevelop;

/**
 * \short A class which visits an expression and determines the type of the expression.
 *
 * \note A type-builder must have been run already over the ast.
 *
 * \todo The code is somewhat redundant with UseBuilder. Maybe UseBuilder should be based on this class. However a TypeBuilder must have been run already on the code, so the declarations of uses are located.
**/
class KDEVCPPDUCHAIN_EXPORT ExpressionVisitor : public DefaultVisitor {
  public:
    /**
     * @param strict When this is false, the expression-visitor tries to recover from problems.
     *               For example when it cannot find a matching function, it returns the first of the candidates.
     * @param propagateConstness When this is set to true, the expression visitor will propagate the constness
     *                           in member accesses. Required for decltype support, i.e.:
     *                           'const A* a; decltype((a->x)) b;', here b should be const
     * */
    explicit ExpressionVisitor( ParseSession* session, const KDevelop::TopDUContext* source = 0,
                                bool strict = false, bool propagateConstness = false, bool mapAst = false );
    ~ExpressionVisitor();

    struct Instance {
      Instance() : isInstance(false) {
      }
      Instance( bool is ) : isInstance(is) {
      }
      Instance( DeclarationPointer decl ) : isInstance(true), declaration(decl) {
      }
      Instance( Declaration* decl ) : isInstance(true), declaration(DeclarationPointer(decl)) {
      }
      inline operator bool() const {
        return isInstance;
      }

      bool isInstance;
      DeclarationPointer declaration; //May contain the declaration of the instance, but only when isInstance is true. May also contain type-declaration, which signalizes that this is an instance of that type.
    };

    /**
     * Will parse the tree and call expressionType(..) for each successfully evaluated type.
     * A DelayedType is created for possible static expressions that use non-assigned template-arguments, so they can be re-evaluated later.
     * @param ast the syntax-tree to evaluate. The context must already be built, the given AST needs to have a filled ducontext.
     **/
    void parse( AST* ast );

    ///Parses the qualification prefix of the given name
    void parseNamePrefix( NameAST* ast );

    AbstractType::Ptr lastType();
    Instance lastInstance();

    ParseSession* session();

    void reportRealProblems(bool);
    
    //Returns all posted real problems that appeared during this run. Only if reportRealProblems(true) has been called before.
    QList<KSharedPtr<KDevelop::Problem> > realProblems() const;
    
    ///Returns the last queried list of declarations
    QList<DeclarationPointer> lastDeclarations() const;
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

  /** The duchain is not locked when this is called */
  virtual void usingDeclaration( AST* node, size_t start_token, size_t end_token, const KDevelop::DeclarationPointer& decl ) {
    Q_UNUSED(node) Q_UNUSED(start_token) Q_UNUSED(end_token) Q_UNUSED(decl)
    }

    /** Called when there is a problem, with a string for that problem.
     * The default-implementation dumps all relevant information to
     * kdDebug.
     * @param node the node the problem is about
     * @param str a string that describes the problem
     */
    virtual void problem( AST* node, const QString& str );

    /**
     * Called for important issues in code.
     *
     * @see reportRealProblems()
     */
    void realProblem( ProblemPointer problem );

    const DUContext* currentContext() const;

    const TopDUContext* topContext() const;

  private:

    bool m_strict, m_memberAccess, m_skipLastNamePart, m_mapAst;
    AbstractType::Ptr m_lastType;
    bool m_hadMemberAccess; // Whether during last call to findName() a class-member was accessed
    Instance m_lastInstance; //Contains whether the last evaluation resulted in an instance, and maybe the instance-declaration

    const KDevelop::TopDUContext* m_source;

    //Whenever a list of declarations is queried, it is stored here. Especially in visitName(...) and findMember(...)
    QList<DeclarationPointer> m_lastDeclarations;

    //Here the parameters of function-calls are collected
    //When a parameter could not be evaluated, this will hold a parameter with null-value type
    QList<OverloadResolver::Parameter> m_parameters;
    //One AST-node for each of the parameters in m_parameters
    KDevVarLengthArray<AST*> m_parameterNodes;
    int m_ignore_uses;

public:
    /**
     * Calls usingDeclaration(..) for any delayed uses, and registers a new use in m_currentUse.
     * The whole sense of this thing is to allow updating an earlier created use in a later AST, like necessary because of overload-resolution.
     * Must be called when the du-chain is not locked.
     * */
    void newUse( AST* node, size_t start_token, size_t end_token, const KDevelop::DeclarationPointer& decl ) {
      if( !m_ignore_uses ) {
        flushUse();
        m_currentUse.isValid = true;
        m_currentUse.node = node;
        m_currentUse.start_token = start_token;
        m_currentUse.end_token = end_token;
        m_currentUse.declaration = decl;
      }
    }
    
protected:
  const Token& tokenFromIndex( int index );
    
private:
    ///Fills m_parameters from the given argument-expression
    ///Returns true if all parameters could be evaluated successfully
    bool buildParametersFromExpression(AST* expression);
    bool buildParametersFromDeclaration(ParameterDeclarationClauseAST* paramDecl, bool store = true);
    
    ///If m_lastType is a valid constructed class-type, returns that type,
    ///and fills all its constructors into m_lastDeclarations
    CppClassType::Ptr computeConstructedType();
  
    void flushUse() {
      if( m_currentUse.isValid )
        usingDeclaration( m_currentUse.node, m_currentUse.start_token, m_currentUse.end_token, m_currentUse.declaration );
      m_currentUse.isValid = false;
    }

    struct CurrentUse {
      CurrentUse() : isValid(false), start_token(0), end_token(0) {
      }
      bool isValid;
      AST* node;
      size_t start_token, end_token;
      KDevelop::DeclarationPointer declaration;
    } m_currentUse; //This is used to temporarily delay the calling of usingDeclaration.

    ParseSession* m_session;
    const KDevelop::DUContext* m_currentContext;
    const KDevelop::TopDUContext* m_topContext;
    bool m_reportRealProblems;

    QList<KSharedPtr<KDevelop::Problem> > m_problems;

    /// set to true when member access on a const object should result in a const type
    /// i.e.: 'const A* a; decltype((a->x)) b;', here b should be const
    bool m_propagateConstness;

  inline void clearLast() {
    m_lastInstance = Instance();
    m_lastType = 0;
    m_lastDeclarations.clear();
  }

  ///Returns whether the given type and instance-info are an lvalue
  bool isLValue( const AbstractType::Ptr& type, const Instance& instance );

  ///Creates a delayed type that refers to the expression within the given node. Sets the type as m_lastType
  void createDelayedType( AST* node, bool expression = true );

  /**
   * Returns the dereferenced type(example: ReferenceType(PointerType) -> PointerType)
   *
   *
   *  !!DU-Chain must be locked!
   * @return return-value will only be zero if m_lastType is zero
   */
  AbstractType::Ptr realLastType() const;

  /**
   * Dereferences m_lastType if it is a pointer-type, else returns false.
   *
   * !!DU-Chain must be locked!
   * */
  bool dereferenceLastPointer();

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
   *  Note that this may return zero even when base is valid, it needs to be an instance of IdentifiedType to be able to retrieve a declaration.
   **/
  Declaration* getDeclaration( const AbstractType::Ptr& base );

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

  /** If the member was found, it is returned through m_lastType and m_lastDeclaration. On fail those are zero.
   * du-chain must not be locked.
   * @param node that issues the command. Only used for posting problems.
   * @param type The structure the member should be searched in
   * @param member Identifier of the member to search
   * @param isConst Whether the type inherits const-ness from above
   * @param postError whether a problem should be posted when the member is not found
   */
  void findMember( AST* node, AbstractType::Ptr type, const Identifier& member, bool isConst = false, bool postProblem = true );

  /**
   *  Visits post-fix sub-expressions.
   * m_lastType and m_lastDeclaration must be filled with the types where to start.
   * When the evaluation is successful, expressionType(..) is called with the resulting type.
   * @param node The basic AST-node which will only be used for reporting problems
   * @param nodes the nodes to visit
   **/
  void visitSubExpressions( AST* node, const ListNode<ExpressionAST*>* nodes );

  void visitTypeSpecifier(TypeSpecifierAST*);

  virtual void visitBinaryExpression(BinaryExpressionAST *) ;
  virtual void visitCastExpression(CastExpressionAST *) ;
  virtual void visitClassMemberAccess(ClassMemberAccessAST *) ;
  virtual void visitCompoundStatement(CompoundStatementAST *) ;
  virtual void visitCondition(ConditionAST *) ;
  virtual void visitConditionalExpression(ConditionalExpressionAST *) ;
  virtual void visitCppCastExpression(CppCastExpressionAST *) ;
  virtual void visitDeclarationStatement(DeclarationStatementAST *) ;
  virtual void visitDeclarator(DeclaratorAST *) ;
  virtual void visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST *) ;
  virtual void visitExpressionOrDeclarationStatement(ExpressionOrDeclarationStatementAST *) ;
  virtual void visitExpressionStatement(ExpressionStatementAST *) ;
  virtual void visitFunctionCall(FunctionCallAST *) ;
  virtual void visitIncrDecrExpression(IncrDecrExpressionAST *) ;
  virtual void visitInitDeclarator(InitDeclaratorAST *) ;
  virtual void visitInitializerClause(InitializerClauseAST *);
  virtual void visitMemInitializer(MemInitializerAST *) ;
  virtual void visitName(NameAST *) ;
  virtual void visitNewDeclarator(NewDeclaratorAST *) ;
  virtual void visitNewExpression(NewExpressionAST *) ;
  virtual void visitPostfixExpression(PostfixExpressionAST *) ;
  virtual void visitPrimaryExpression(PrimaryExpressionAST *) ;
  virtual void visitPtrOperator(PtrOperatorAST *) ;
  virtual void visitSimpleDeclaration(SimpleDeclarationAST *) ;
  virtual void visitSimpleTypeSpecifier(SimpleTypeSpecifierAST *) ;
  virtual void visitSizeofExpression(SizeofExpressionAST *) ;
  virtual void visitStringLiteral(StringLiteralAST *) ;
  virtual void visitSubscriptExpression(SubscriptExpressionAST *) ;
  virtual void visitTranslationUnit(TranslationUnitAST *) ;
  virtual void visitTypeId(TypeIdAST *) ;
  virtual void visitUnaryExpression(UnaryExpressionAST *) ;
  virtual void visitSignalSlotExpression (SignalSlotExpressionAST*);
  virtual void visitTypeIDOperator(TypeIDOperatorAST *);
  virtual void visitLambdaExpression(LambdaExpressionAST *);
  virtual void visitBracedInitList(BracedInitListAST *);
  virtual void visit(AST* node);

  void visitExpressionToken(uint tokenIndex, AST* node);

  void putStringType();
  AbstractType::Ptr qObjectPtrType() const;
  
  ///Visits all nodes, and resets m_lastType and m_lastDeclaration to the previous values before each visit so they cannot influence each other
  template <class _Tp>
  void visitIndependentNodes(const ListNode<_Tp> *nodes);

private:
  void handleFunctionCallOrInit(AST* node, ExpressionAST* arguments);
  bool m_handlingFunctionCallOrInit;
};
}

#endif

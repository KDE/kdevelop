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

#include "expressionvisitor.h"

#include <duchainlock.h>
#include <duchain.h>
#include <ducontext.h>
#include <parsesession.h>
#include <declaration.h>
#include <identifiedtype.h>
#include <typeinfo>
#include "tokens.h"
#include "duchainbuilder/typebuilder.h"

#include "duchainbuilder/cpptypes.h"
#include "duchainbuilder/dumpchain.h"
#include "name_compiler.h"
#include "lexer.h"

///Remember to always when visiting a node create a PushPositiveValue object for the context

/** A typical expression:
 | | \ExpressionStatement[(39) (0, 92)] "d -> a = 5 ;"
| | | | \BinaryExpression[(39) (0, 92)] "d -> a = 5"
| | | | | \PostfixExpression[(39) (0, 92)] "d -> a"
| | | | | | \PrimaryExpression[(39) (0, 92)] "d"
| | | | | | | \Name[(39) (0, 92)] "d"
| | | | | | | | \UnqualifiedName[(39) (0, 92)] "d"
| | | | | | | | /UnqualifiedName[(40) (0, 93)]
| | | | | | | /Name[(40) (0, 93)]
| | | | | | /PrimaryExpression[(40) (0, 93)]
| | | | | | \ClassMemberAccess[(40) (0, 93)] "-> a"
| | | | | | | \Name[(41) (0, 95)] "a"
| | | | | | | | \UnqualifiedName[(41) (0, 95)] "a"
| | | | | | | | /UnqualifiedName[(42) (0, 97)]
| | | | | | | /Name[(42) (0, 97)]
| | | | | | /ClassMemberAccess[(42) (0, 97)]
| | | | | /PostfixExpression[(42) (0, 97)]
| | | | | \PrimaryExpression[(43) (0, 99)] "5"
| | | | | /PrimaryExpression[(44) (0, 100)]
| | | | /BinaryExpression[(44) (0, 100)]
| | | /ExpressionStatement[(45) (0, 102)
*/

#define LOCKDUCHAIN     DUChainReadLocker lock(DUChain::lock());
#define MUST_HAVE(X) if(!X) { problem( node, "no X" ); return; }

namespace Cpp {


template <class _Tp>
void ExpressionVisitor::visitIndependentNodes(const ListNode<_Tp> *nodes)
{
  if (!nodes)
    return;

  AbstractType::Ptr oldLastType = m_lastType;
  Declaration* oldLastDeclaration = m_lastDeclaration;
  
  const ListNode<_Tp>
    *it = nodes->toFront(),
    *end = it;

  do
    {
      m_lastType =  oldLastType;
      m_lastDeclaration = oldLastDeclaration;
      
      visit(it->element);
      it = it->next;
    }
  while (it != end);
}

  
template<class Type>
bool isType( const AbstractType::Ptr& t ) {
  return (bool)dynamic_cast<Type>( t.data() );
}

bool isConstant( AbstractType* t ) {
  CppCVType* cv = dynamic_cast<CppCVType*>( t );
  return cv && cv->isConstant();
}

///Replaces a given value if the new value evaluates to be positive, and puts the old one back on destruction
template<class Value>
class PushPositiveValue {
  public:
    PushPositiveValue( Value& ptr, const Value& push = Value()  ) : m_ptr(ptr)  {
      m_oldPtr = m_ptr;
      if( push ) {
        m_ptr = push;
      }
    }
    ~PushPositiveValue() {
      m_ptr = m_oldPtr;
    }
  private:
    Value& m_ptr;
    Value m_oldPtr;
};

template<class Value>
class PushValue {
  public:
    PushValue( Value& ptr, const Value& push = Value()  ) : m_ptr(ptr)  {
      m_oldPtr = m_ptr;
      m_ptr = push;
    }
    ~PushValue() {
      m_ptr = m_oldPtr;
    }
  private:
    Value& m_ptr;
    Value m_oldPtr;
};

typedef PushPositiveValue<DUContext*> PushPositiveContext;

const Token& ExpressionVisitor::tokenFromIndex( int index ) {
  return m_session->token_stream->token(index);
}


typedef PushValue<AbstractType::Ptr> PushAbstractType;
  
ExpressionVisitor::ExpressionVisitor(ParseSession* session) : m_lastDeclaration(0), m_session(session), m_currentContext(0) {
}

ExpressionVisitor::~ExpressionVisitor() {
}

ParseSession* ExpressionVisitor::session() {
  return m_session;
}

void ExpressionVisitor::parse( AST* ast ) {
  m_lastType = 0;
  m_lastDeclaration = 0;
  Q_ASSERT(ast->ducontext);
  visit(ast);
}

void ExpressionVisitor::problem( AST* node, const QString& str ) {
  kDebug() << "Cpp::ExpressionVisitor problem: " << str << endl;

  kDebug() << "Cpp::ExpressionVisitor dumping the node that created the problem" << endl;
  DumpChain d;
  d.dump(node, m_session);
}

AbstractType::Ptr ExpressionVisitor::lastType() {
  return m_lastType;
}

Declaration* ExpressionVisitor::lastDeclaration() {
  return m_lastDeclaration;
}

/** Find the member in the declaration's du-chain. This does not respect function-parameters yet.
 **/
void ExpressionVisitor::findMember( AST* node, AbstractType::Ptr base, const QualifiedIdentifier& member, bool isConst, bool postProblem ) {
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    
    clearLast();
    
    LOCKDUCHAIN;

    isConst |= isConstant(base.data());
    
    //When the type is a reference, dereference it so we get to the pointer-type
    ReferenceType* ref = dynamic_cast<ReferenceType*>( base.data() );
    if( ref )
      base = ref->baseType();

    isConst |= isConstant(base.data());
    
    IdentifiedType* idType = dynamic_cast<IdentifiedType*>( base.data() );
    MUST_HAVE( idType );

    //Make sure that it is a structure-type, because other types do not have members
    StructureType* structureType = dynamic_cast<StructureType*>( base.data() );
    MUST_HAVE(structureType);

    Declaration* declaration = idType->declaration();
    MUST_HAVE(declaration);
    MUST_HAVE(declaration->context());

    ///@todo isn't there a nicer solution for this? Maybe the internal context should be stored in CppStructureType,
    ///or CppStructureType should store better information so context's are not needed
    QList<DUContext*> internalContext = declaration->context()->findContexts(DUContext::Class, idType->identifier());
    internalContext += declaration->context()->findContexts(DUContext::Namespace, idType->identifier());
    internalContext += declaration->context()->findContexts(DUContext::Global, idType->identifier());

    MUST_HAVE( (internalContext.size()==1) );
    
    QList<Declaration*> decls = internalContext.front()->findLocalDeclarations( member );

    if( decls.isEmpty() ) {
      if( postProblem ) {
        problem( node, QString("could not find member \"%1\" in \"%2\", scope of context: %3").arg(member.toString()).arg(declaration->toString()).arg(declaration->context()->scopeIdentifier().toString()) );
      }
      return;
    }

    ///@todo match function-parameters
    
    //Give a default return without const-checking.
    m_lastType = decls.front()->abstractType();
    m_lastDeclaration = decls.front();

    //If it is a function, match the const qualifier
    for( QList<Declaration*>::const_iterator it = decls.begin(); it != decls.end(); ++it ) {
      CppCVType* functionCVType = dynamic_cast<CppCVType*>( (*it)->abstractType().data() );
      if( functionCVType ) {
        if( functionCVType->isConstant() == isConst ) {
          m_lastType = (*it)->abstractType();
          m_lastDeclaration = *it;
          break;
        }
      }
    }
}

/**
 *  Here the . and -> operators are implemented.
 *  Before visitClassMemberAccess is called, m_lastType and m_lastDeclaration must be set
 *  to the base-types
 **/
  void ExpressionVisitor::visitClassMemberAccess(ClassMemberAccessAST* node)
{
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    
    AbstractType::Ptr base = m_lastType;
    Declaration* baseDeclaration = m_lastDeclaration;

    m_lastType = 0;
    m_lastDeclaration = 0;

    if( !baseDeclaration || !base ) {
      problem(node, "VisitClassMemberAccess called without a base-declaration. '.' and '->' operators are only allowed on type-instances.");
      return;
    }
    
    StructureType* st = dynamic_cast<StructureType*>( base.data() );

    if( !st ) {
      problem( node, QString("member-access requested on non-structure type: %1").arg( typeid(*base.data()).name() ) );
      return;
    }
    
    NameCompiler nameC( m_session );
    nameC.run(node->name);

    if( nameC.identifier().isEmpty() ) {
      problem( node, "name is empty" );
      return;
    }

    bool isConst = false;
    
    switch( tokenFromIndex(node->op).kind ) {
      case Token_arrow:
      {
        LOCKDUCHAIN;
        //When the type is a reference, dereference it so we get to the pointer-type
        ReferenceType* ref = dynamic_cast<ReferenceType*>( base.data() );
        if( ref )
          base = ref->baseType();
        //Either the type is a pointer, or it must contain the -> operator

        PointerType* pnt = dynamic_cast<PointerType*>( base.data() );
        if( pnt ) {
          isConst |= isConstant(pnt);
          //It is a pointer, reduce the pointer-depth by one
          base = pnt->baseType();
        } else {
          findMember( node, base, QualifiedIdentifier("operator->") );
          if( !m_lastType ) {
            problem( node, "no overloaded operator-> found" );
            return;
          }
          
          base = m_lastType;
          baseDeclaration = m_lastDeclaration;
          clearLast();

        }
      }
      case '.':
      break;
      default:
        problem( node, QString("unknown class-member access operation: %1").arg( tokenFromIndex(node->op).kind ) );
        return;
      break;
    }

    findMember( node, base, nameC.identifier() );
    
    if( m_lastType )
      expressionType( node, m_lastType, m_lastDeclaration );
  }

  /**
   * Here declarations are located
   **/
  
  void ExpressionVisitor::visitName(NameAST* node)
  {
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    
    m_lastType = 0;
    m_lastDeclaration = 0;
    
    ///@todo Check if this node represents a use, if yes take the declaration from that use(we don't need to search then)
    NameCompiler nameC( m_session );
    nameC.run(node);

    if( nameC.identifier().isEmpty() ) {
      problem( node, "name is empty" );
      return;
    }

    {
      LOCKDUCHAIN;

      int line, column;
      QString file;
      
      m_session->positionAt( m_session->token_stream->position(node->start_token), &line, &column, &file );

      QList<Declaration*> dec = m_currentContext->findDeclarations(nameC.identifier(), KTextEditor::Cursor(line, column) );
      if( dec.isEmpty() ) {
        problem( node, QString("could not find declaration of %1").arg( nameC.identifier().toString() ) );
      } else {
        ///@todo for overloaded functions, choose the right function
        m_lastType = dec.first()->abstractType();

        ///If the found declaration declares a type, this is a type-expression and m_lastDeclaration should be zero.
        ///The declaration declares a type if it's abstractType's declaration is that declaration. Else it is an insantiation, and m_lastType should be filled.

        ///@todo what is a not IdentifiedType type, what to do with it?
        IdentifiedType* idType = dynamic_cast<IdentifiedType*>( m_lastType.data() );
        if( idType && idType->declaration() != dec.first() )
          m_lastDeclaration = dec.first();
      }
    }

    if( m_lastType )
      expressionType( node, m_lastType, m_lastDeclaration );
  }
  
  /** Primary expressions just forward to their encapsulated expression */
  void ExpressionVisitor::visitPrimaryExpression(PrimaryExpressionAST* node)
  {
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    
    visit( node->sub_expression );
    visit( node->literal );
    visit( node->expression_statement );
    visit( node->name );
    
    if( m_lastType )
      expressionType( node, m_lastType, m_lastDeclaration );
  }

  /** Translation-units just forward to their encapsulated expression */
  void ExpressionVisitor::visitTranslationUnit(TranslationUnitAST* node)
  {
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    
    visitNodes(this, node->declarations);
    
    if( m_lastType )
      expressionType( node, m_lastType, m_lastDeclaration );
  }

  /** Sub-expressions of a post-fix expression, will be applied in order to m_lastType */
  void  ExpressionVisitor::visitSubExpressions( AST* node, const ListNode<ExpressionAST*>* nodes ) {
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    
    if( !m_lastType ) {
       problem( node, "primary expression returned no type" );
       return;
    }
    const ListNode<ExpressionAST*> *it = nodes->toFront(), *end = it;

    int num = 0;
    do
      {
        visit(it->element);
        
        if( !m_lastType ) {
          problem( node, QString("while parsing post-fix-expression: sub-expression %1 returned no type").arg(num) );
          return;
        }
        it = it->next;
        num++;
      }
    while (it != end);
    
    if( m_lastType )
      expressionType( node, m_lastType, m_lastDeclaration );
  }
  
  /** A postfix-expression is a primary expression together with a chain of sub-expressions that are applied from left to right */
  void ExpressionVisitor::visitPostfixExpression(PostfixExpressionAST* node)
  {
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    
    clearLast();
    if( node->type_specifier ) {
      problem( node, "unexpected type-specifier" );
      return;
    }
    if( !node->expression ) {
      problem( node, "primary expression missing" );
      return;
    }
    //First evaluate the primary expression, and then pass the result from sub-expression to sub-expression through m_lastType
    visit( node->expression );

    if( !node->sub_expressions )
      return;

    visitSubExpressions( node, node->sub_expressions );
  }

  /**
   **/
  void ExpressionVisitor::visitBinaryExpression(BinaryExpressionAST* node)  {
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    
    clearLast();

    ///First resolve left part, then right, then combine
    visit(node->left_expression);

    Declaration* leftDeclaration = m_lastDeclaration;
    AbstractType::Ptr leftType = m_lastType;
    clearLast();
    
    visit(node->right_expression);

    Declaration* rightDeclaration = m_lastDeclaration;
    AbstractType::Ptr rightType = m_lastType;
    clearLast();

    if( !leftDeclaration ) {
      problem( node, "left operand of binary expression could not be evaluated" );
      return;
    }
    
    if( !rightDeclaration ) {
      problem( node, "right operand of binary expression could not be evaluated" );
      m_lastDeclaration = leftDeclaration;
      m_lastType = leftType;
      return;
    }

    switch( tokenFromIndex(node->op).kind ) {
      ///@todo implement all the other binary expressions
      default:
        problem(node, "not implemented binary expression" );
      case Token_assign:
      case '=':
        m_lastType = leftType;
        m_lastDeclaration = leftDeclaration;
      break;
    };
    
    if( m_lastType )
      expressionType( node, m_lastType, m_lastDeclaration );
  }
  
  void ExpressionVisitor::visitCppCastExpression(CppCastExpressionAST* node)  {
    
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    
    //Visit the expression just so it is evaluated and expressionType(..) eventually called, the result will not be used here
    clearLast();
    visit( node->expression );
    clearLast();

    TypeBuilder t( m_session );
    t.supportBuild(node->type_id);
    if( t. topTypes().size() != 1 ) {
      problem( node, QString("wrong count of types built: %1").arg(t.topTypes().size() ) );
      return;
    }
    {
      LOCKDUCHAIN;
      const IdentifiedType* idType = dynamic_cast<const IdentifiedType*>( t.topTypes().first().data() );
      MUST_HAVE( idType );
      MUST_HAVE( idType->declaration() );

      m_lastType = t.topTypes().first();
      m_lastDeclaration = idType->declaration();
    }

    visitSubExpressions( node, node->sub_expressions );
  }
  
  void ExpressionVisitor::visitCastExpression(CastExpressionAST* node)  {

    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    
    //Visit the expression just so it is evaluated and expressionType(..) eventually called, the result will not be used here
    clearLast();
    visit( node->expression );
    clearLast();
    
    TypeBuilder t( m_session );
    t.supportBuild(node->type_id);
    if( t. topTypes().size() != 1 ) {
      problem( node, QString("wrong count of types built: %1").arg(t.topTypes().size() ) );
      return;
    }
    {
      LOCKDUCHAIN;
      const IdentifiedType* idType = dynamic_cast<const IdentifiedType*>( t.topTypes().first().data() );
      MUST_HAVE( idType );
      MUST_HAVE( idType->declaration() );
      
      m_lastType = t.topTypes().first();
      m_lastDeclaration = idType->declaration();
    }

    if( m_lastType )
      expressionType( node, m_lastType, m_lastDeclaration );
  }

  void ExpressionVisitor::visitConditionalExpression(ConditionalExpressionAST* node)
  {
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    
    //Also visit the not interesting parts, so they are evaluated
    clearLast();
    visit(node->condition);
    clearLast();
    visit(node->left_expression);
    clearLast();

    //Since both possible results of a conditional expression must have the same type, we only consider the right one here
    visit(node->right_expression);
    
    if( m_lastType )
      expressionType( node, m_lastType, m_lastDeclaration );
  }  
  
  void ExpressionVisitor::visitExpressionStatement(ExpressionStatementAST* node)
  {
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    clearLast();
    visit(node->expression);
    if( m_lastType )
      expressionType( node, m_lastType, m_lastDeclaration );
  }

  void ExpressionVisitor::visitPtrOperator(PtrOperatorAST* node)
  {
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    problem(node, "node-type cannot be parsed");
  }

  /** For a compound statement, process all statements and return the type of the last one */
  void ExpressionVisitor::visitCompoundStatement(CompoundStatementAST* node)
  {
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    visitIndependentNodes(node->statements);
  }
  
  void ExpressionVisitor::visitExpressionOrDeclarationStatement(ExpressionOrDeclarationStatementAST* node)  {
    //visit(node->declaration);
    visit(node->expression);
    
    if( m_lastType )
      expressionType( node, m_lastType, m_lastDeclaration );
  }
  
  void ExpressionVisitor::visitFunctionCall(FunctionCallAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitCondition(ConditionAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitDeleteExpression(DeleteExpressionAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitOperator(OperatorAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitOperatorFunctionId(OperatorFunctionIdAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitPtrToMember(PtrToMemberAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitUnaryExpression(UnaryExpressionAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitSubscriptExpression(SubscriptExpressionAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitSizeofExpression(SizeofExpressionAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitStringLiteral(StringLiteralAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitIncrDecrExpression(IncrDecrExpressionAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitNewExpression(NewExpressionAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitUnqualifiedName(UnqualifiedNameAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitTypeId(TypeIdAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitNewTypeId(NewTypeIdAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitSimpleDeclaration(SimpleDeclarationAST* node)  { problem(node, "node-type cannot be parsed"); }
  
  ///Nodes that are invalid inside an expression:
  void ExpressionVisitor::visitAccessSpecifier(AccessSpecifierAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitAsmDefinition(AsmDefinitionAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitBaseClause(BaseClauseAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitBaseSpecifier(BaseSpecifierAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitClassSpecifier(ClassSpecifierAST* node)  { problem(node, "node-type cannot be parsed"); }

  void ExpressionVisitor::visitCtorInitializer(CtorInitializerAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitDeclarationStatement(DeclarationStatementAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitDeclarator(DeclaratorAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitDoStatement(DoStatementAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitEnumSpecifier(EnumSpecifierAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitEnumerator(EnumeratorAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitExceptionSpecification(ExceptionSpecificationAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitForStatement(ForStatementAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitFunctionDefinition(FunctionDefinitionAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitIfStatement(IfStatementAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitInitDeclarator(InitDeclaratorAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitInitializer(InitializerAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitInitializerClause(InitializerClauseAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitLabeledStatement(LabeledStatementAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitLinkageBody(LinkageBodyAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitLinkageSpecification(LinkageSpecificationAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitMemInitializer(MemInitializerAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitNamespace(NamespaceAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitNamespaceAliasDefinition(NamespaceAliasDefinitionAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitNewDeclarator(NewDeclaratorAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitNewInitializer(NewInitializerAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitParameterDeclaration(ParameterDeclarationAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitParameterDeclarationClause(ParameterDeclarationClauseAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitReturnStatement(ReturnStatementAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitSimpleTypeSpecifier(SimpleTypeSpecifierAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitSwitchStatement(SwitchStatementAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitTemplateArgument(TemplateArgumentAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitTemplateDeclaration(TemplateDeclarationAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitTemplateParameter(TemplateParameterAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitThrowExpression(ThrowExpressionAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitTryBlockStatement(TryBlockStatementAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitTypeParameter(TypeParameterAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitTypedef(TypedefAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitUsing(UsingAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitUsingDirective(UsingDirectiveAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitWhileStatement(WhileStatementAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitWinDeclSpec(WinDeclSpecAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitTypeIdentification(TypeIdentificationAST* node)  { problem(node, "node-type cannot be parsed"); }

}

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
#include "duchainbuilder/typerepository.h"
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
#define MUST_HAVE(X) if(!X) { problem( node, "no " # X ); return; }

namespace Cpp {


template <class _Tp>
void ExpressionVisitor::visitIndependentNodes(const ListNode<_Tp> *nodes)
{
  if (!nodes)
    return;

  AbstractType::Ptr oldLastType = m_lastType;
  Instance oldLastInstance = m_lastInstance;
  
  const ListNode<_Tp>
    *it = nodes->toFront(),
    *end = it;

  do
    {
      m_lastType =  oldLastType;
      m_lastInstance = oldLastInstance;
      
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
  
ExpressionVisitor::ExpressionVisitor(ParseSession* session) : m_session(session), m_currentContext(0) {
}

ExpressionVisitor::~ExpressionVisitor() {
}

ParseSession* ExpressionVisitor::session() {
  return m_session;
}

void ExpressionVisitor::parse( AST* ast ) {
  m_lastType = 0;
  m_lastInstance = Instance();
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

ExpressionVisitor::Instance ExpressionVisitor::lastInstance() {
  return m_lastInstance;
}

/** Find the member in the declaration's du-chain. This does not respect function-parameters yet.
 **/
void ExpressionVisitor::findMember( AST* node, AbstractType::Ptr base, const QualifiedIdentifier& member, bool isConst, bool postProblem ) {
    PushPositiveContext pushContext( m_currentContext, node->ducontext );

    LOCKDUCHAIN;

    base = realType(base,&isConst);
    
    clearLast();
    
    isConst |= isConstant(base.data());
    
    IdentifiedType* idType = dynamic_cast<IdentifiedType*>( base.data() );
    if( !idType )  {
      problem( node, QString("findMember called on not identified type \"%1\"").arg(base->toString()) );
    }

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
    m_lastInstance = Instance( decls.front() );

    //If it is a function, match the const qualifier
    for( QList<Declaration*>::const_iterator it = decls.begin(); it != decls.end(); ++it ) {
      CppCVType* functionCVType = dynamic_cast<CppCVType*>( (*it)->abstractType().data() );
      if( functionCVType ) {
        if( functionCVType->isConstant() == isConst ) {
          m_lastType = (*it)->abstractType();
          m_lastInstance.declaration = *it;
          break;
        }
      }
    }
}

/**
 *  Here the . and -> operators are implemented.
 *  Before visitClassMemberAccess is called, m_lastType and m_lastInstance must be set
 *  to the base-types
 **/
  void ExpressionVisitor::visitClassMemberAccess(ClassMemberAccessAST* node)
{
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    
    AbstractType::Ptr base = m_lastType;
    Instance baseInstance = m_lastInstance;

    clearLast();

    if( !baseInstance || !base ) {
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

        PointerType* pnt = dynamic_cast<PointerType*>( realLastType().data() );
        if( pnt ) {
          isConst = isConstant(pnt);
          //It is a pointer, reduce the pointer-depth by one
          base = pnt->baseType();
          baseInstance = Instance(getDeclaration(node, base));
        } else {
          findMember( node, base, QualifiedIdentifier("operator->") );
          if( !m_lastType ) {
            problem( node, "no overloaded operator-> found" );
            return;
          }
          getReturnValue(node);
          if( !m_lastType ) {
            problem( node, "could not get return-type of operator->" );
            return;
          }

          if( !getPointerTarget(node, &isConst) ) {
            clearLast();
            return;
          }
          
          base = m_lastType;
          baseInstance = m_lastInstance;
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
      expressionType( node, m_lastType, m_lastInstance );
  }


  AbstractType::Ptr ExpressionVisitor::realLastType(bool* constant) const {
    return realType( m_lastType, constant );
  }
  
  AbstractType::Ptr ExpressionVisitor::realType(AbstractType::Ptr base, bool* constant) const {
    
    CppReferenceType* ref = dynamic_cast<CppReferenceType*>( base.data() );
    
    while( ref ) {
      if( constant )
        (*constant) |= ref->isConstant();
      base = ref->baseType();
      ref = dynamic_cast<CppReferenceType*>( base.data() );
    }

    return base;
  }
  
  AbstractType::Ptr ExpressionVisitor::targetType(AbstractType::Ptr base, bool* constant) const {
    
    CppReferenceType* ref = dynamic_cast<CppReferenceType*>( base.data() );
    CppPointerType* pnt = dynamic_cast<CppPointerType*>( base.data() );
    
    while( ref || pnt ) {
      if( ref ) {
        if( constant )
          (*constant) |= ref->isConstant();
        base = ref->baseType();
      } else {
        if( constant )
          (*constant) |= pnt->isConstant();
        base = pnt->baseType();
      }
      ref = dynamic_cast<CppReferenceType*>( base.data() );
      pnt = dynamic_cast<CppPointerType*>( base.data() );
    }

    return base;
  }
  
  bool ExpressionVisitor::isPointerType() const {
    return dynamic_cast<PointerType*>( realLastType().data() );
  }

  bool ExpressionVisitor::getPointerTarget( AST* node, bool* constant )  {
    if( !m_lastType ) return false;
    
    AbstractType::Ptr base = realLastType();

    clearLast();

    CppPointerType* pnt = dynamic_cast<CppPointerType*>( base.data() );
    if( pnt ) {
      if( constant )
        (*constant) |= pnt->isConstant();
      m_lastType = pnt->baseType();
      m_lastInstance = Instance( getDeclaration(node, m_lastType) );
      return true;
    } else {
      LOCKDUCHAIN;
      problem(node, QString("Cannot dereference base-type \"%1\"").arg(base->toString()) );
      return false;
    }
  }

  Declaration* ExpressionVisitor::getDeclaration( AST* node, const AbstractType::Ptr& base ) {
    if( !base ) return 0;
    
    IdentifiedType* idType = dynamic_cast<IdentifiedType*>(m_lastType.data());
    if( idType ) {
      return idType->declaration();
    } else {
      LOCKDUCHAIN;
      problem(node, QString("type \"%1\" is not identified").arg(base->toString() ) );
      return 0;
    }
    
  }
  
  /**
   * Here declarations are located
   **/
  
  void ExpressionVisitor::visitName(NameAST* node)
  {
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    
    m_lastType = 0;
    m_lastInstance = Instance();
    
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

        ///If the found declaration declares a type, this is a type-expression and m_lastInstance should be zero.
        ///The declaration declares a type if it's abstractType's declaration is that declaration. Else it is an insantiation, and m_lastType should be filled.

        //Check if the declaration of the type differs from the declaration we found.
        //If it does, it is not the declaration of the type but the declaration of an instance of that type.
        //Then we can fill m_lastInstance, because that is only filled for type-instances, not for types.
        Declaration* d = getDeclaration(node, m_lastType);
        if( d != dec.first() ) {
          m_lastInstance = Instance( dec.first() );
        }
      }
    }

    if( m_lastType )
      expressionType( node, m_lastType, m_lastInstance );
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
      expressionType( node, m_lastType, m_lastInstance );
  }

  /** Translation-units just forward to their encapsulated expression */
  void ExpressionVisitor::visitTranslationUnit(TranslationUnitAST* node)
  {
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    
    visitNodes(this, node->declarations);
    
    if( m_lastType )
      expressionType( node, m_lastType, m_lastInstance );
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
      expressionType( node, m_lastType, m_lastInstance );
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

    Instance leftInstance = m_lastInstance;
    AbstractType::Ptr leftType = m_lastType;
    clearLast();
    
    visit(node->right_expression);

    Instance rightInstance = m_lastInstance;
    AbstractType::Ptr rightType = m_lastType;
    clearLast();

    if( !leftInstance ) {
      problem( node, "left operand of binary expression could not be evaluated" );
      return;
    }
    
    if( !rightInstance ) {
      problem( node, "right operand of binary expression could not be evaluated" );
      m_lastInstance = leftInstance;
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
        m_lastInstance = leftInstance;
      break;
    };
    
    if( m_lastType )
      expressionType( node, m_lastType, m_lastInstance );
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

      m_lastType = t.topTypes().first();
      m_lastInstance = Instance( getDeclaration(node, m_lastType ) );
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
      
      m_lastType = t.topTypes().first();
      m_lastInstance = Instance(getDeclaration(node, m_lastType ));
    }

    if( m_lastType )
      expressionType( node, m_lastType, m_lastInstance );
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
      expressionType( node, m_lastType, m_lastInstance );
  }  
  
  void ExpressionVisitor::visitExpressionStatement(ExpressionStatementAST* node)
  {
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    clearLast();
    visit(node->expression);
    if( m_lastType )
      expressionType( node, m_lastType, m_lastInstance );
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
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    //visit(node->declaration);
    visit(node->expression);

    if( m_lastType )
      expressionType( node, m_lastType, m_lastInstance );
  }

  void ExpressionVisitor::visitUnaryExpression(UnaryExpressionAST* node)
  {
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    
    clearLast();

    visit(node->expression);
    
    if( !m_lastInstance || !m_lastType ) {
      clearLast();
      problem(node, "Tried to evaluate unary expression on a non-instance item" );
      return;
    }
    
    switch( tokenFromIndex(node->op).kind ) {
    case '*':
    {
      LOCKDUCHAIN;
      
      if( PointerType* pt = dynamic_cast<PointerType*>( m_lastType.data() ) )
      {
        //Dereference
        m_lastType = pt->baseType();
        m_lastInstance = Instance(getDeclaration(node,m_lastType));
      } else {
        //Get return-value of operator*
        findMember(node, m_lastType, QualifiedIdentifier("operator*") );
        if( !m_lastType ) {
          problem( node, "no overloaded operator-> found" );
          return;
        }
        getReturnValue(node);
      }
    }
    break;
    case '&':
    {
      bool constant = false;
      AbstractType::Ptr oldType = realLastType(&constant); ///Dereference references
      CppPointerType::Ptr newPointer(new CppPointerType( constant ? Declaration::Const : Declaration::CVNone ));
      newPointer->setBaseType( oldType );
      m_lastType = TypeRepository::self()->registerType(AbstractType::Ptr(newPointer.data()));
      //m_lastInstance will be left alone as it was before. A pointer is not identified, and has no declaration.
    }
    break;
    case Token_incr:
    case Token_decr:
    case '+':
    case '-':
    case '!':
    case '~':
      ///@todo implement
    break;
    }
    
    if( m_lastType )
      expressionType( node, m_lastType, m_lastInstance );
  }

  void ExpressionVisitor::getReturnValue( AST* node ) {
    if( !m_lastType )
      return;

    CppFunctionType* f = dynamic_cast<CppFunctionType*>( m_lastType.data() );
    if( !f ) {
      LOCKDUCHAIN;
      problem(node, QString("cannot get return-type of type %1, it is not a function-type").arg(m_lastType->toString()));
      m_lastType = 0;
      m_lastInstance = Instance();
      return;
    }

    m_lastType = f->returnType();
    m_lastInstance = Instance(getDeclaration(node, m_lastType));
  }
  
  void ExpressionVisitor::visitFunctionCall(FunctionCallAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitCondition(ConditionAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitDeleteExpression(DeleteExpressionAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitOperator(OperatorAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitOperatorFunctionId(OperatorFunctionIdAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitPtrToMember(PtrToMemberAST* node)  { problem(node, "node-type cannot be parsed"); }
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

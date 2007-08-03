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

#include "expressionvisitor.h"

#include <duchainlock.h>
#include <duchain.h>
#include <ducontext.h>
#include <parsesession.h>
#include <declaration.h>
#include <identifiedtype.h>
#include <typeinfo>
#include "tokens.h"
#include "typebuilder.h"
#include "typerepository.h"
#include "cpptypes.h"
#include "dumpchain.h"
#include "typeutils.h"
#include "name_compiler.h"
#include "lexer.h"
#include "overloadresolution.h"
#include "cppduchain.h"

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

#define LOCKDUCHAIN     DUChainReadLocker lock(DUChain::lock())
#define MUST_HAVE(X) if(!X) { problem( node, "no " # X ); return; }

namespace Cpp {
using namespace KDevelop;
using namespace TypeUtils;

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


bool ExpressionVisitor::isLValue( const AbstractType::Ptr& type, const Instance& instance ) {
  return instance && (instance.declaration || isReferenceType(type));
}

  
ExpressionVisitor::ExpressionVisitor(ParseSession* session, bool strict) : m_strict(strict), m_session(session), m_currentContext(0) {
}

ExpressionVisitor::~ExpressionVisitor() {
}

QList<Declaration*> ExpressionVisitor::lastDeclarations() const {
  return m_lastDeclarations;
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
  kDebug(9007) << "Cpp::ExpressionVisitor problem:" << str;

  kDebug(9007) << "Cpp::ExpressionVisitor dumping the node that created the problem";
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
 *
 * @todo make this deal with klass->ParentClass::member
 * @todo make this deal with parent-classes
 **/
void ExpressionVisitor::findMember( AST* node, AbstractType::Ptr base, const QualifiedIdentifier& member, bool isConst, bool postProblem ) {
    
    ///have test
    
    PushPositiveContext pushContext( m_currentContext, node->ducontext );

    LOCKDUCHAIN;

    base = realType(base,&isConst);
    
    clearLast();
    
    isConst |= isConstant(base.data());
    
    IdentifiedType* idType = dynamic_cast<IdentifiedType*>( base.data() );
    //Make sure that it is a structure-type, because other types do not have members
    StructureType* structureType = dynamic_cast<StructureType*>( base.data() );
    
    if( !structureType || !idType ) {
      problem( node, QString("findMember called on non-identified or non-structure type \"%1\"").arg(base->toString()) );
      return;
    }

    Declaration* declaration = idType->declaration();
    MUST_HAVE(declaration);
    MUST_HAVE(declaration->context());

    DUContext* internalContext = getInternalContext(declaration);

    MUST_HAVE( internalContext );
    
    m_lastDeclarations = findLocalDeclarations( internalContext, member );

    
    if( m_lastDeclarations.isEmpty() ) {
      if( postProblem ) {
        problem( node, QString("could not find member \"%1\" in \"%2\", scope of context: %3").arg(member.toString()).arg(declaration->toString()).arg(declaration->context()->scopeIdentifier().toString()) );
      }
      return;
    }

    //Give a default return without const-checking.
    m_lastType = m_lastDeclarations.front()->abstractType();
    m_lastInstance = Instance( m_lastDeclarations.front() );

    //If it is a function, match the const qualifier
    for( QList<Declaration*>::const_iterator it = m_lastDeclarations.begin(); it != m_lastDeclarations.end(); ++it ) {
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
 *
 * have test
 * 
 **/
  void ExpressionVisitor::visitClassMemberAccess(ClassMemberAccessAST* node)
{
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    
    AbstractType::Ptr base = m_lastType;
    Instance baseInstance = m_lastInstance;
    m_lastDeclarations.clear();

    clearLast();

    if( !baseInstance || !base ) {
      problem(node, "VisitClassMemberAccess called without a base-declaration. '.' and '->' operators are only allowed on type-instances.");
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
        ///have test
        LOCKDUCHAIN;
        //When the type is a reference, dereference it so we get to the pointer-type

        PointerType* pnt = dynamic_cast<PointerType*>( realType(base) );
        if( pnt ) {
          kDebug(9007) << "got type:" << pnt->toString();
          kDebug(9007) << "base-type:" << pnt->baseType()->toString();
          
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
        ///have test
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
    return AbstractType::Ptr(realType( m_lastType, constant ));
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
    Q_UNUSED(node)
    if( !base ) return 0;
    
    const IdentifiedType* idType = dynamic_cast<const IdentifiedType*>(base.data());
    if( idType ) {
      LOCKDUCHAIN;
      return idType->declaration();
    } else {
      return 0;
    }
  }
  
  /**
   * Here declarations are located
   *
   * have test
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

      ///@todo reenable(first make sure the conversion works properly)
      m_lastDeclarations = m_currentContext->findDeclarations(nameC.identifier() );//, KTextEditor::Cursor(line, column) ); 
      if( m_lastDeclarations.isEmpty() ) {
        problem( node, QString("could not find declaration of %1").arg( nameC.identifier().toString() ) );
      } else {
        m_lastType = m_lastDeclarations.first()->abstractType();

        ///If the found declaration declares a type, this is a type-expression and m_lastInstance should be zero.
        ///The declaration declares a type if it's abstractType's declaration is that declaration. Else it is an insantiation, and m_lastType should be filled.

        if( m_lastDeclarations.first()->kind() == Declaration::Instance )
          m_lastInstance = Instance( m_lastDeclarations.first() );
        else
          m_lastInstance = Instance(false);
      }
    }

    if( m_lastType )
      expressionType( node, m_lastType, m_lastInstance );
  }
  
  /** Primary expressions just forward to their encapsulated expression
   *
   * have test
   * 
  */
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

  /** Sub-expressions of a post-fix expression, will be applied in order to m_lastType
   *
   * have test  */
  
  void  ExpressionVisitor::visitSubExpressions( AST* node, const ListNode<ExpressionAST*>* nodes ) {
    if( !nodes )
      return;
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
  
  /** A postfix-expression is a primary expression together with a chain of sub-expressions that are applied from left to right
   *
   * have test */
  
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
   *
   * partially have test **/
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

    if( tokenFromIndex(node->op).kind == ',' ) {
      /**A ',' binary expression is used for separating the argument-expressions in a function-call.
       * Those should be collected into m_parameters
       *
       * How this should work: Every binary ',' expression yields a m_lastType of null.
       *
       * So whenever an operand(left or right side) yields a type, we can be sure it is not a binary-expression
       * so we can add the type to the parameter-list.
       * */
      if( leftType && leftInstance) {
        m_parameters << OverloadResolver::Parameter(leftType.data(), isLValue( leftType, leftInstance ) );
      } else {
        problem(node->left_expression, "left operand of binary ','-expression is no type-instance" );
        m_parameters << OverloadResolver::Parameter(0, false);
      }

      if( rightType && rightInstance) {
        m_parameters << OverloadResolver::Parameter(rightType.data(), isLValue( rightType, rightInstance ) );
      } else {
        problem(node->right_expression, "right operand of binary ','-expression is no type-instance" );
        m_parameters << OverloadResolver::Parameter(0, false);
      }
      
      clearLast();
    }
    

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
        ///have test
        m_lastType = leftType;
        m_lastInstance = leftInstance;
      break;
    };
    
    if( m_lastType )
      expressionType( node, m_lastType, m_lastInstance );
  }

  /**
   *
   * Not ready yet */
  
  void ExpressionVisitor::visitCppCastExpression(CppCastExpressionAST* node)  {
    
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    
    //Visit the expression just so it is evaluated and expressionType(..) eventually called, the result will not be used here
    clearLast();
    visit( node->expression );
    clearLast();

    ///@todo Change this, share code with TypeBuilder::visitSimpleTypeSpecifier to also respect integral types, const, volatile, pointer, etc.
    ///We need to parse a TypeIdAST for this. That's not done anywhere until now.
//     TypeCompiler compiler(m_session);
//     compiler.run(node->type_id);
//     
//     
//     if( compiler.identifier().isEmpty() ) {
//       problem( node, QString("could not compile identifier") );
//       return;
//     }
//     {
//       LOCKDUCHAIN;
//       int line, column;
//       QString file;
//       
//       m_session->positionAt( m_session->token_stream->position(node->start_token), &line, &column, &file );
// 
//       m_lastDeclarations = m_currentContext->findDeclarations(compiler.identifier(), KTextEditor::Cursor(line, column) );
// 
//       if( m_lastDeclarations.isEmpty() ) {
//         problem( node, QString("could not find declaration of %1").arg(compiler.identifier().toString()) );
//         return;
//       }
//       if( m_lastDeclarations.size() > 1 ) {
//         problem( node, QString("found multiple declarations of %1").arg(compiler.identifier().toString()) );
//       }
//       
//       m_lastType = m_lastDeclarations.front()->abstractType();
//       m_lastInstance = Instance( getDeclaration(node, m_lastType ) );
//     }

    visitSubExpressions( node, node->sub_expressions );
  }
  
  /**
   *
   * Not ready yet */
  void ExpressionVisitor::visitCastExpression(CastExpressionAST* node)  {

    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    
    //Visit the expression just so it is evaluated and expressionType(..) eventually called, the result will not be used here
    clearLast();
    visit( node->expression );
    clearLast();
    
    ///Change this, don't use type-builder, instead somehow parse a QualifiedIdentifier and some decoration, and search for that in the du-context
    TypeBuilder t( m_session );
    t.supportBuild(node->type_id, m_currentContext);
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

  /**
   *
   * have test */
  void ExpressionVisitor::visitConditionalExpression(ConditionalExpressionAST* node)
  {
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    
    //Also visit the not interesting parts, so they are evaluated
    clearLast();
    visit(node->condition);
    clearLast();
    visit(node->left_expression);
    clearLast();

    ///@todo test if result of right expression can be converted to the result of the right expression. If not, post a problem(because c++ wants it that way)

    //Since both possible results of a conditional expression must have the same type, we only consider the right one here
    visit(node->right_expression);
    
    if( m_lastType )
      expressionType( node, m_lastType, m_lastInstance );
  }  

  /**
   * have test */
  void ExpressionVisitor::visitExpressionStatement(ExpressionStatementAST* node)
  {
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    clearLast();
    visit(node->expression);
    if( m_lastType )
      expressionType( node, m_lastType, m_lastInstance );
  }

  /** For a compound statement, process all statements and return the type of the last one
   *
   * have test */
  void ExpressionVisitor::visitCompoundStatement(CompoundStatementAST* node)
  {
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    visitIndependentNodes(node->statements);
  }

  /**
   * have test */
  
  void ExpressionVisitor::visitExpressionOrDeclarationStatement(ExpressionOrDeclarationStatementAST* node)  {
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    //visit(node->declaration);
    visit(node->expression);

    if( m_lastType )
      expressionType( node, m_lastType, m_lastInstance );
  }

  /**
   * partially have test */
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
      ///@todo use overloaded functions
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
  
  void ExpressionVisitor::visitFunctionCall(FunctionCallAST* node) {
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    if( m_lastDeclarations.isEmpty() ) {
      problem( node, "function-call: no matching declarations found" );
      return;
    }

    /**
     * Step 1: Evaluate the function-argument types. Those are represented a little strangely:
     * node->arguments contains them, using recursive binary expressions
     * */
    QList<Declaration*> declarations = m_lastDeclarations;
    
    m_parameters.clear();

    clearLast();
    visit(node->arguments);

    //binary expressions don't yield m_lastType, so when m_lastType is set wo probably only have one single parameter
    if( m_lastType )
      m_parameters << OverloadResolver::Parameter(m_lastType.data(), isLValue( m_lastType, m_lastInstance ) );
    
    //Check if all parameters could be evaluated
    int paramNum = 1;
    bool fail = false;
    for( QList<OverloadResolver::Parameter>::const_iterator it = m_parameters.begin(); it != m_parameters.end(); ++it ) {
      if( (*it).type == 0 ) {
        problem(node, QString("parameter %1 could not be evaluated").arg(paramNum) );
        fail = true;
        paramNum++;
      }
    }
    LOCKDUCHAIN;
    Declaration* chosenFunction = 0;
    OverloadResolver resolver( m_currentContext );

    if( !fail ) {
      chosenFunction = resolver.resolveList(m_parameters, declarations);
    }

    if( !chosenFunction && !m_strict ) {
      //Because we do not want to rely too much on our understanding of the code, we take the first function instead of totally failing.
      problem(node, "Could not find a function that matches the parameters. Using first candidate function.");
      fail = true;
    }

    if( fail ) {
      //Since not all parameters could be evaluated, Choose the first function
      chosenFunction = declarations.front();
    }

    m_parameters.clear();

    clearLast();
    
    CppFunctionType* functionType = dynamic_cast<CppFunctionType*>( chosenFunction->abstractType().data() );
    if( !chosenFunction || !functionType ) {
      problem( node, QString( "could not find a matching function for function-call" ) );
    } else {
      m_lastType = functionType->returnType();
      m_lastInstance = Instance(true);
    }
  }
  
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
  void ExpressionVisitor::visitTypeIdentification(TypeIdentificationAST* node)  { problem(node, "node-type cannot be parsed"); }

  void ExpressionVisitor::visitPtrOperator(PtrOperatorAST* node)
  {
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    problem(node, "node-type cannot be parsed");
  }

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
}

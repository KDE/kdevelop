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
#include <parsesession.h>
#include <declaration.h>
#include <definition.h>
#include <identifiedtype.h>
#include <typeinfo>
#include "tokens.h"
#include "typebuilder.h"
#include "typerepository.h"
#include "cpptypes.h"
#include "dumpchain.h"
#include "typeutils.h"
#include "name_visitor.h"
#include "type_visitor.h"
#include "lexer.h"
#include "overloadresolution.h"
#include "cppduchain.h"
#include "overloadresolutionhelper.h"

//If this is enabled and a type is not found, it is searched again with verbose debug output.
#define DEBUG_RESOLUTION_PROBLEMS

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

/**
 * @todo Deal DelayedType correctly everywhere.
 * When a DelayedType is encountered, it should be filled with the
 * appropriate expression to compute the type/value later on.
 * */

#define LOCKDUCHAIN     DUChainReadLocker lock(DUChain::lock())
#define MUST_HAVE(X) if(!X) { problem( node, "no " # X ); return; }

namespace Cpp {
using namespace KDevelop;
using namespace TypeUtils;

bool isNumber( const QString& str ) {
  if( str.isEmpty() )
    return false;
    QChar c = str[0];
    return c == '0' || c == '1' || c == '2' || c == '3' || c == '4'|| c == '5'|| c == '6'|| c == '7'|| c == '8'|| c == '9';
}

QHash<int, QString> initOperatorNames() {
  QHash<int, QString> ret;
  ret['+'] = "+";
  ret['-'] = "-";
  ret['*'] = "*";
  ret['/'] = "/";
  ret['%'] = "%";
  ret['^'] = "^";
  ret['&'] = "&";
  ret['|'] = "|";
  ret['~'] = "~";
  ret['!'] = "!";
  ret['='] = "=";
  ret['<'] = "<";
  ret['>'] = ">";
  ret[','] = ",";
  ret[Token_assign] = "=";
  ret[Token_shift] = "<<"; ///@todo Parser does not differentiate between << and >>
  ret[Token_eq] = "==";
  ret[Token_not_eq] = "!=";
  ret[Token_leq] = "<=";
  ret[Token_geq] = ">=";
  ret[Token_not_eq] = "!=";
  ret[Token_and] = "&&";
  ret[Token_or] = "||";

  return ret;
}

QHash<int, QString> operatorNames = initOperatorNames();

QString operatorNameFromTokenKind( int tokenKind )
{
  QHash<int, QString>::const_iterator it = operatorNames.find(tokenKind);
  if( it == operatorNames.end() )
    return QString();
  else
    return *it;
}

QList<DeclarationPointer> convert( const QList<Declaration*>& list ) {
  QList<DeclarationPointer> ret;
  foreach( Declaration* decl, list )
    ret << DeclarationPointer(decl);
  return ret;
}

QList<Declaration*> convert( const QList<DeclarationPointer>& list ) {
  QList<Declaration*> ret;
  foreach( DeclarationPointer decl, list )
    if( decl )
      ret << decl.data();
  return ret;
}

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

TopDUContext* ExpressionVisitor::topContext() const {
  if( !m_inclusionTrace.isEmpty() ) {
    return m_inclusionTrace.front().ctx->topContext();
  }else{
    return m_topContext;
  }
}

bool ExpressionVisitor::isLValue( const AbstractType::Ptr& type, const Instance& instance ) {
  return instance && (instance.declaration || isReferenceType(type));
}

ExpressionVisitor::ExpressionVisitor(ParseSession* session, const KDevelop::ImportTrace& inclusionTrace, bool strict) : m_strict(strict), m_memberAccess(false), m_inclusionTrace(inclusionTrace), m_ignore_uses(0), m_session(session), m_currentContext(0), m_topContext(0) {
}

ExpressionVisitor::~ExpressionVisitor() {
}

QList<DeclarationPointer> ExpressionVisitor::lastDeclarations() const {
  return m_lastDeclarations;
}


ParseSession* ExpressionVisitor::session() {
  return m_session;
}

void ExpressionVisitor::parse( AST* ast ) {
  m_lastType = 0;
  m_lastInstance = Instance();
  Q_ASSERT(ast->ducontext);
  m_topContext = ast->ducontext->topContext();
  visit(ast);
  m_topContext = 0;
  flushUse();
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

/** Find the member in the declaration's du-chain. **/
void ExpressionVisitor::findMember( AST* node, AbstractType::Ptr base, const QualifiedIdentifier& member, bool isConst, bool postProblem ) {
    
    ///have test
    
    PushPositiveContext pushContext( m_currentContext, node->ducontext );

    LOCKDUCHAIN;

    base = realType(base, topContext(), &isConst);
    
    clearLast();
    
    isConst |= isConstant(base.data());
    
    IdentifiedType* idType = dynamic_cast<IdentifiedType*>( base.data() );
    //Make sure that it is a structure-type, because other types do not have members
    StructureType* structureType = dynamic_cast<StructureType*>( base.data() );
    
    if( !structureType || !idType ) {
      problem( node, QString("findMember called on non-identified or non-structure type \"%1\"").arg(base ? base->toString() : "<type disappeared>") );
      return;
    }

    Declaration* declaration = idType->declaration();
    MUST_HAVE(declaration);
    MUST_HAVE(declaration->context());

    DUContext* internalContext = declaration->logicalInternalContext(topContext());

    MUST_HAVE( internalContext );
    
    m_lastDeclarations = convert(findLocalDeclarations( internalContext, member ));

    
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
    for( QList<DeclarationPointer>::const_iterator it = m_lastDeclarations.begin(); it != m_lastDeclarations.end(); ++it ) {
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
  ///@todo allow bla->BlaBase::member()
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    
    if( !m_lastInstance || !m_lastType ) {
      problem(node, "VisitClassMemberAccess called without a base-declaration. '.' and '->' operators are only allowed on type-instances.");
      return;
    }
    
    bool isConst = false;
    
    switch( tokenFromIndex(node->op).kind ) {
      case Token_arrow:
      {
        ///have test
        LOCKDUCHAIN;
        //When the type is a reference, dereference it so we get to the pointer-type

        PointerType* pnt = dynamic_cast<PointerType*>( realType(m_lastType, topContext()) );
        if( pnt ) {
/*          kDebug(9007) << "got type:" << pnt->toString();
          kDebug(9007) << "base-type:" << pnt->baseType()->toString();*/
          
          isConst = isConstant(pnt);
          //It is a pointer, reduce the pointer-depth by one
          m_lastType = pnt->baseType();
          m_lastInstance = Instance( getDeclaration(node, m_lastType) );
        } else {
          findMember( node, m_lastType, QualifiedIdentifier("operator->") ); ///@todo respect const
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

          if( !m_lastDeclarations.isEmpty() ) {
            DeclarationPointer decl(m_lastDeclarations.first());
            lock.unlock();
            newUse( node, node->op, node->op+1, decl );
          }
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

  m_memberAccess = true;
  visitName(node->name);
  m_memberAccess = false;
  }


  AbstractType::Ptr ExpressionVisitor::realLastType(bool* constant) const {
    return AbstractType::Ptr(realType( m_lastType, topContext(), constant ));
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
      m_lastInstance = Instance(getDeclaration(node, m_lastType));
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
    PushPositiveContext pushContext( m_currentContext, node->ducontext ? node->ducontext : m_currentContext ); //Definitely push one up here, so we can change the context without side-effects
    
    SimpleCursor position = m_session->positionAt( m_session->token_stream->position(node->start_token) );
    if( m_currentContext->url() != m_session->m_url ) //.equals( m_session->m_url, KUrl::CompareWithoutTrailingSlash ) )
      position = position.invalid();

    if( m_memberAccess ) {
      LOCKDUCHAIN;
      bool isConst = false; //@todo get this from upside
      
      m_lastType = realType(m_lastType, topContext(), &isConst);

      isConst |= isConstant(m_lastType.data());
      
      IdentifiedType* idType = dynamic_cast<IdentifiedType*>( m_lastType.data() );
      //Make sure that it is a structure-type, because other types do not have members
      StructureType* structureType = dynamic_cast<StructureType*>( m_lastType.data() );
      
      if( !structureType || !idType ) {
        problem( node, QString("member searched in non-identified or non-structure type \"%1\"").arg(m_lastType ? m_lastType->toString() : "<type disappeared>") );
        clearLast();
        return;
      }

      Declaration* declaration = idType->declaration();
      MUST_HAVE(declaration);
      MUST_HAVE(declaration->context());

      m_currentContext = declaration->logicalInternalContext(topContext());

      MUST_HAVE( m_currentContext );
    }

    clearLast();

    ImportTrace trace;
    {
      LOCKDUCHAIN;
      trace = topContext()->importTrace(m_currentContext->topContext());
    }
    
    NameASTVisitor nameV( m_session, this, m_currentContext, trace, position.isValid() ? position : m_currentContext->range().end, m_memberAccess ? DUContext::DontSearchInParent : DUContext::NoSearchFlags );
    nameV.run(node);

    if( nameV.identifier().isEmpty() ) {
      problem( node, "name is empty" );
      return;
    }

    QualifiedIdentifier identifier = nameV.identifier();

    ///@todo It would be better if the parser would treat true and false exactly
    ///like constant-integer expressions, storing them in a primary expression.
    static QualifiedIdentifier trueIdentifier("true");
    static QualifiedIdentifier falseIdentifier("false");

    if( identifier == trueIdentifier || identifier == falseIdentifier ) {
      ///We have a boolean constant, we need to catch that here
      LOCKDUCHAIN;
      m_lastType = TypeRepository::self()->registerType( AbstractType::Ptr(new CppConstantIntegralType(CppConstantIntegralType::TypeBool, CppIntegralType::ModifierNone)) );
      m_lastInstance = Instance( true );
      static_cast<CppConstantIntegralType*>(m_lastType.data())->setValue<qint64>( identifier == trueIdentifier );
    } else {
      LOCKDUCHAIN;

      m_lastDeclarations = nameV.declarations();
      
      if( m_lastDeclarations.isEmpty() ) {
        problem( node, QString("could not find declaration of %1").arg( nameV.identifier().toString() ) );
      } else {
        m_lastType = m_lastDeclarations.first()->abstractType();
        //kDebug(9007) << "found declaration: " << m_lastDeclarations.first()->toString();

        ///If the found declaration declares a type, this is a type-expression and m_lastInstance should be zero.
        ///The declaration declares a type if its abstractType's declaration is that declaration. Else it is an insantiation, and m_lastType should be filled.

        if( m_lastDeclarations.first()->kind() == Declaration::Instance )
          m_lastInstance = Instance( m_lastDeclarations.first() );
        else
          m_lastInstance = Instance(false);

        //A CppTemplateParameterType represents an unresolved template-parameter, so create a DelayedType instead.
        if( dynamic_cast<CppTemplateParameterType*>(m_lastType.data()) )
          createDelayedType(node, false);
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

    clearLast();

    if( node->literal ) {
      visit( node->literal );
      return; //We had a string-literal
    }

    if( !node->literal && !node->sub_expression && !node->expression_statement && !node->name )
    {
      QString symbol = tokenFromIndex(node->start_token).symbol();
      
      if( isNumber(symbol) )
      {
        QString num;
        for( size_t a = node->start_token; a < node->end_token; a++ )
          num += tokenFromIndex(a).symbol();
        
        
        LOCKDUCHAIN;
        if( num.indexOf('.') != -1 || num.endsWith('f') || num.endsWith('d') ) {
          double val = 0;
          bool ok = false;
          while( !num.isEmpty() && !ok ) {
            val = num.toDouble(&ok);
            num.truncate(num.length()-1);
          }


          if( num.endsWith('f') ) {
            m_lastType = TypeRepository::self()->registerType( AbstractType::Ptr(new CppConstantIntegralType(CppConstantIntegralType::TypeFloat, CppIntegralType::ModifierNone)));
            static_cast<CppConstantIntegralType*>(m_lastType.data())->setValue<float>((float)val);
          } else {
            m_lastType = TypeRepository::self()->registerType( AbstractType::Ptr(new CppConstantIntegralType(CppConstantIntegralType::TypeDouble, CppIntegralType::ModifierNone)));
            static_cast<CppConstantIntegralType*>(m_lastType.data())->setValue<double>(val);
          }
        } else {
          qint64 val;
          CppIntegralType::TypeModifier mod = CppIntegralType::ModifierNone;

          if( num.endsWith("u") || ( num.length() > 1 && num[1] == 'x' ) )
            mod = CppIntegralType::ModifierUnsigned;

          bool ok = false;
          while( !num.isEmpty() && !ok ) {
            val = num.toLongLong(&ok, 0);
            num.truncate(num.length()-1);
          }

          m_lastType = TypeRepository::self()->registerType(AbstractType::Ptr(new CppConstantIntegralType(CppConstantIntegralType::TypeInt, mod)));

          if( mod & CppIntegralType::ModifierUnsigned )
            static_cast<CppConstantIntegralType*>(m_lastType.data())->setValue<quint64>(val);
          else
            static_cast<CppConstantIntegralType*>(m_lastType.data())->setValue<qint64>(val);
        }
        m_lastInstance = Instance(true);
        
        return;
      }
    }
    
    visit( node->sub_expression );
    visit( node->expression_statement );
    visit( node->name );

    //Respect "this" token
    if( tokenFromIndex(node->token).kind == Token_this ) {
      LOCKDUCHAIN;

      AbstractType::Ptr thisType;

      DUContext* context = m_currentContext; //Here we find the context of the function-declaration/definition we're currently in
      while( context->parentContext() && context->type() == DUContext::Other && context->parentContext()->type() == DUContext::Other )
      { //Move context to the top context of type "Other". This is needed because every compound-statement creates a new sub-context.
        context = context->parentContext();
      }

      ///Step 1: Find the function-declaration for the function we are in
      Declaration* functionDeclaration = 0;

      if( context->owner() && context->owner()->asDefinition() )
      {
        //If we are in a definition, move the classContext to the declaration's classContext, and take the type from there
        if(!context->owner()->asDefinition()->declaration()) {
          problem(node, "No declaration for definition");
          return;
        }
        
        functionDeclaration = context->owner()->asDefinition()->declaration();
      }
      else if( context->owner()->asDeclaration() )
      {
        functionDeclaration = context->owner()->asDeclaration();
      }
       
      if( !functionDeclaration )
      {
        problem(node, "\"this\" used, but no function-declaration could be found");
        return;
      }

      ///Step 2: Find the type of "this" from the function-declaration
      DUContext* classContext = functionDeclaration->context();

      //Take the type from the classContext
      if( classContext && classContext->type() == DUContext::Class && classContext->owner() && classContext->owner()->asDeclaration() )
        thisType = classContext->owner()->asDeclaration()->abstractType();

      if( !thisType ) {
        problem(node, "\"this\" used in invalid classContext");
        return;
      }

      ///Step 3: Create a pointer-type for the "this" type and return it
      CppFunctionType* cppFunction = dynamic_cast<CppFunctionType*>(functionDeclaration->abstractType().data());

      if( cppFunction ) {
        CppPointerType::Ptr thisPointer( new CppPointerType( cppFunction->cv() ) );
        thisPointer->setBaseType( thisType );

        m_lastType = TypeRepository::self()->registerType(AbstractType::Ptr(thisPointer.data()) );
        m_lastInstance = Instance(true);
      }else{
        if( context->owner() && context->owner()->asDeclaration() && context->owner()->asDeclaration()->abstractType() )
          problem(node, QString("\"this\" used in non-function context of type %1(%2)").arg( typeid(m_currentContext->owner()->asDeclaration()->abstractType().data()).name() ) .arg(m_currentContext->owner()->asDeclaration()->abstractType()->toString()));
        else
          problem(node, "\"this\" used in non-function context with invalid type");
      }
    }
    
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

/** A helper-class for evaluating constant binary expressions under different types(int, float, etc.) */
template<class Type>
struct ConstantBinaryExpressionEvaluator {

  Type endValue;

  CppIntegralType::IntegralTypes type;
  CppIntegralType::TypeModifiers modifier;

  /**
   * Writes the results into endValue, type, and modifier.
   * */
  ConstantBinaryExpressionEvaluator( int tokenKind, CppConstantIntegralType* left, CppConstantIntegralType* right ) {
    endValue = 0;
    type = left->integralType(); ///@todo choose the resulting type better
    modifier = left->typeModifiers();

    evaluateSpecialTokens(tokenKind, left, right);
    
    switch( tokenKind ) {
      case '+':
        endValue = left->CppConstantIntegralType::value<Type>() + right->CppConstantIntegralType::value<Type>();
      break;
      case '-':
        endValue = left->CppConstantIntegralType::value<Type>() - right->CppConstantIntegralType::value<Type>();
      break;
      case '*':
        endValue = left->CppConstantIntegralType::value<Type>() * right->CppConstantIntegralType::value<Type>();
      break;
      case '/':
        endValue = left->CppConstantIntegralType::value<Type>() / right->CppConstantIntegralType::value<Type>();
      break;
      case '=':
        endValue = right->CppConstantIntegralType::value<Type>();
      break;
      case '<':
        endValue = left->CppConstantIntegralType::value<Type>() < right->CppConstantIntegralType::value<Type>();
        type = CppIntegralType::TypeBool;
      break;
      case '>':
        endValue = left->CppConstantIntegralType::value<Type>() > right->CppConstantIntegralType::value<Type>();
        type = CppIntegralType::TypeBool;
      break;
      case Token_assign:
        endValue = right->CppConstantIntegralType::value<Type>();
      break;
      case Token_eq:
        endValue = left->CppConstantIntegralType::value<Type>() == right->CppConstantIntegralType::value<Type>();
        type = CppIntegralType::TypeBool;
      break;
      case Token_not_eq:
        endValue = left->CppConstantIntegralType::value<Type>() != right->CppConstantIntegralType::value<Type>();
        type = CppIntegralType::TypeBool;
      break;
      case Token_leq:
        endValue = left->CppConstantIntegralType::value<Type>() <= right->CppConstantIntegralType::value<Type>();
        type = CppIntegralType::TypeBool;
      break;
      case Token_geq:
        endValue = left->CppConstantIntegralType::value<Type>() >= right->CppConstantIntegralType::value<Type>();
        type = CppIntegralType::TypeBool;
      break;
    }
  }

  //This function is used to disable some operators on bool and double values
  void evaluateSpecialTokens( int tokenKind, CppConstantIntegralType* left, CppConstantIntegralType* right ) {
    switch( tokenKind ) {
      case '%':
        endValue = left->CppConstantIntegralType::value<Type>() % right->CppConstantIntegralType::value<Type>();
      break;
      case '^':
        endValue = left->CppConstantIntegralType::value<Type>() ^ right->CppConstantIntegralType::value<Type>();
      break;
      case '&':
        endValue = left->CppConstantIntegralType::value<Type>() & right->CppConstantIntegralType::value<Type>();
      break;
      case '|':
        endValue = left->CppConstantIntegralType::value<Type>() | right->CppConstantIntegralType::value<Type>();
      break;
      case Token_shift:
        ///@todo shift-direction?
        endValue = left->CppConstantIntegralType::value<Type>() << right->CppConstantIntegralType::value<Type>();
      break;
      case Token_and:
        endValue = left->CppConstantIntegralType::value<Type>() && right->CppConstantIntegralType::value<Type>();
        type = CppIntegralType::TypeBool;
      break;
      case Token_or:
        endValue = left->CppConstantIntegralType::value<Type>() || right->CppConstantIntegralType::value<Type>();
        type = CppIntegralType::TypeBool;
      break;
    }
  }
  
  AbstractType::Ptr createType() {
    AbstractType::Ptr ret = TypeRepository::self()->registerType( AbstractType::Ptr(new CppConstantIntegralType(type, modifier)) );
    static_cast<CppConstantIntegralType*>(ret.data())->CppConstantIntegralType::setValue<Type>( endValue );
    return ret;
  }
};

template<>
void ConstantBinaryExpressionEvaluator<double>::evaluateSpecialTokens( int tokenKind, CppConstantIntegralType* left, CppConstantIntegralType* right ) {
}

template<>
void ConstantBinaryExpressionEvaluator<float>::evaluateSpecialTokens( int tokenKind, CppConstantIntegralType* left, CppConstantIntegralType* right ) {
}

/** A helper-class for evaluating constant unary expressions under different types(int, float, etc.) */
template<class Type>
struct ConstantUnaryExpressionEvaluator {

  Type endValue;

  CppIntegralType::IntegralTypes type;
  CppIntegralType::TypeModifiers modifier;

  /**
   * Writes the results into endValue, type, and modifier.
   * */
  ConstantUnaryExpressionEvaluator( int tokenKind, CppConstantIntegralType* left ) {
    endValue = 0;
    type = left->integralType();
    modifier = left->typeModifiers();
    evaluateSpecialTokens( tokenKind, left );
    switch( tokenKind ) {
      case '+':
        endValue = +left->CppConstantIntegralType::value<Type>();
      break;
      case '-':
        endValue = -left->CppConstantIntegralType::value<Type>();
      break;
      case Token_incr:
        endValue = left->CppConstantIntegralType::value<Type>()+1;
      case Token_decr:
        endValue = left->CppConstantIntegralType::value<Type>()-1;
    }
  }

  //This function is used to disable some operators on bool and double values
  void evaluateSpecialTokens( int tokenKind, CppConstantIntegralType* left ) {
    switch( tokenKind ) {
      case '~':
        endValue = ~left->CppConstantIntegralType::value<Type>();
      break;
      case '!':
        endValue = !left->CppConstantIntegralType::value<Type>();
      break;
    }
  }

  AbstractType::Ptr createType() {
    AbstractType::Ptr ret = TypeRepository::self()->registerType( AbstractType::Ptr(new CppConstantIntegralType(type, modifier)) );
    static_cast<CppConstantIntegralType*>(ret.data())->CppConstantIntegralType::setValue<Type>( endValue );
    return ret;
  }
};

template<>
void ConstantUnaryExpressionEvaluator<double>::evaluateSpecialTokens( int tokenKind, CppConstantIntegralType* left ){
}

template<>
void ConstantUnaryExpressionEvaluator<float>::evaluateSpecialTokens( int tokenKind, CppConstantIntegralType* left ){
}

void ExpressionVisitor::createDelayedType( AST* node , bool expression ) {
  DelayedType::Ptr type(new DelayedType());
  QString id;
  for( size_t s = node->start_token; s < node->end_token; ++s )
    id += m_session->token_stream->token(s).symbol();

  QualifiedIdentifier ident( id );
  ident.setIsExpression( expression );
  type->setQualifiedIdentifier( ident );
  m_lastType = AbstractType::Ptr( type.data() );
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

        //LOCKDUCHAIN;
        //kDebug(9007) << "Adding parameter from left: " << (leftType.data() ? leftType->toString() : QString("<notype>"));
      } else {
        //If neither leftType nor leftInstance are true, the expression was probably another binary
        //expression that has put the types/instances into m_parameters and returns nothing.
        if( leftType || leftInstance ) {
          if( leftType )
            problem( node->left_expression, "left operand of binary ','-expression is no type-instance" );
          else
            problem( node->left_expression, "left operand of binary ','-expression could not be evaluated" );
          
          m_parameters << OverloadResolver::Parameter(0, false);
          //LOCKDUCHAIN;
          //kDebug(9007) << "Adding empty from left";
        }
      }

      if( rightType && rightInstance) {
        m_parameters << OverloadResolver::Parameter(rightType.data(), isLValue( rightType, rightInstance ) );
        //LOCKDUCHAIN;
        //kDebug(9007) << "Adding parameter from right: " << (rightType.data() ? rightType->toString() : QString("<notype>"));
      } else {
        //If neither leftType nor leftInstance are true, the expression was probably another binary
        //expression that has put the types/instances into m_parameters and returns nothing.
        if( rightType || rightInstance ) {
          if( rightType )
            problem( node->right_expression, "right operand of binary ','-expression is no type-instance" );
          else
            problem( node->right_expression, "right operand of binary ','-expression could not be evaluated" );
          
          m_parameters << OverloadResolver::Parameter(0, false);
          //kDebug(9007) << "Adding empty from right";
        }
      }
      
      clearLast();
      return;
    }
    
    if( !leftInstance && !leftType ) {
      problem( node, "left operand of binary expression could not be evaluated" );
      return;
    }
    
    if( !rightInstance && !rightType ) {
      problem( node, "right operand of binary expression could not be evaluated" );
      m_lastInstance = leftInstance;
      m_lastType = leftType;
      return;
    }

    if( dynamic_cast<DelayedType*>(rightType.data()) || dynamic_cast<DelayedType*>(leftType.data()) ) {
      m_lastInstance = Instance(true);
      createDelayedType(node);
      return;
    }
    

    if( rightInstance && leftInstance && rightType && leftType &&
        dynamic_cast<CppConstantIntegralType*>(rightType.data()) &&
        dynamic_cast<CppConstantIntegralType*>(leftType.data()) ) {
        //Constantly evaluate integral expressions
        CppConstantIntegralType* left = static_cast<CppConstantIntegralType*>(leftType.data());
        CppConstantIntegralType* right = static_cast<CppConstantIntegralType*>(rightType.data());

        LOCKDUCHAIN;

        ///@todo better choice of resulting type
        switch( left->integralType() ) {
          case CppIntegralType::TypeFloat:
          {
            ConstantBinaryExpressionEvaluator<float> evaluator( tokenFromIndex(node->op).kind, left, right );
            m_lastType = evaluator.createType();
            break;
          }
          case CppIntegralType::TypeDouble:
          {
            ConstantBinaryExpressionEvaluator<double> evaluator( tokenFromIndex(node->op).kind, left, right );
            m_lastType = evaluator.createType();
            break;
          }
          default:
            if( left->typeModifiers() & CppIntegralType::ModifierUnsigned ) {
              ConstantBinaryExpressionEvaluator<quint64> evaluator( tokenFromIndex(node->op).kind, left, right);
              m_lastType = evaluator.createType();
            } else {
              ConstantBinaryExpressionEvaluator<qint64> evaluator( tokenFromIndex(node->op).kind, left, right);
              m_lastType = evaluator.createType();
            }
            break;
        }
    
        m_lastInstance = Instance(true);
      } else {
      switch( tokenFromIndex(node->op).kind ) {
        case Token_assign:
        default:
        {
          QString op = operatorNameFromTokenKind(tokenFromIndex(node->op).kind);

          bool success = false;
          if( !op.isEmpty() )
          {
            LOCKDUCHAIN;
            KDevelop::DUContextPointer ptr(m_currentContext);
            OverloadResolutionHelper helper(ptr);
            helper.setOperator( OverloadResolver::Parameter(leftType.data(), isLValue( leftType, leftInstance ) ), op );
            helper.setKnownParameters( OverloadResolver::ParameterList( OverloadResolver::Parameter(rightType.data(), isLValue( rightType, rightInstance ) ) ) );
            QList<OverloadResolutionFunction> functions = helper.resolve(false);

            if( !functions.isEmpty() )
            {
              CppFunctionType::Ptr function = functions.first().function.declaration()->type<CppFunctionType>();
              if( functions.first().function.isViable() && function ) {
                success = true;
                m_lastType = function->returnType();
                m_lastInstance = Instance(function->declaration());
                
                lock.unlock();
                newUse( node, node->op, node->op+1, functions.first().function.declaration() );
              }else{
                //Do not complain here, because we do not check for builtin operators
                //problem(node, "No fitting operator. found" );
                //problem(node, QString("Found no viable operator-function"));
              }
            }else{
              //Do not complain here, because we do not check for builtin operators
              //problem(node, "No fitting operator. found" );
            }
            //Find an overloaded binary operator
          } else {
            problem(node, "not implemented binary expression" );
          }

          if( !success ) {
            m_lastType = leftType;
            m_lastInstance = leftInstance;
          }
        }
        break;
      }
    }

    if( m_lastType )
      expressionType( node, m_lastType, m_lastInstance );
  }

  /**
   *
   * Not ready yet */
  
  void ExpressionVisitor::visitTypeSpecifier(TypeSpecifierAST* ast)
  {
    PushPositiveContext pushContext( m_currentContext, ast->ducontext );
    
    ///@todo cv-qualifiers
    clearLast();
    
    ImportTrace trace;
    {
      LOCKDUCHAIN;
      trace = topContext()->importTrace(m_currentContext->topContext());
    }
    
    TypeASTVisitor comp(m_session, this, m_currentContext, trace);
    comp.run(ast);
    
    LOCKDUCHAIN;

    QList<DeclarationPointer> decls = comp.declarations();

    if( !decls.isEmpty() )
    {
      m_lastDeclarations = decls;
      m_lastType = decls.first()->abstractType();

      if( decls.first()->kind() == Declaration::Type )
        m_lastInstance = Instance(false);
      else
        ///Allow non-types, because we sometimes don't know whether something is a type or not, and it may get parsed as a type.
        m_lastInstance = Instance(decls.first());

      if( dynamic_cast<CppTemplateParameterType*>(m_lastType.data()) )
        createDelayedType(ast, false);
    } else {
      //Run the ast-visitor in debug mode
      problem(ast, "Could not resolve type, running ast-visitor in debug mode");
#ifdef DEBUG_RESOLUTION_PROBLEMS
      ImportTrace trace;
      {
        LOCKDUCHAIN;
        trace = topContext()->importTrace(m_currentContext->topContext());
      }
      ++m_ignore_uses;
      TypeASTVisitor comp2(m_session, this, m_currentContext, trace, true);
      comp2.run(ast);
      --m_ignore_uses;
#endif
    }
  }

  void ExpressionVisitor::visitSimpleTypeSpecifier(SimpleTypeSpecifierAST* node)
  {
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    
    clearLast();
    
    if (node->integrals) {
      CppIntegralType::IntegralTypes type = CppIntegralType::TypeNone;
      CppIntegralType::TypeModifiers modifiers = CppIntegralType::ModifierNone;

      const ListNode<std::size_t> *it = node->integrals->toFront();
      const ListNode<std::size_t> *end = it;
      do {
        int kind = m_session->token_stream->kind(it->element);
        switch (kind) {
          case Token_char:
            type = CppIntegralType::TypeChar;
            break;
          case Token_wchar_t:
            type = CppIntegralType::TypeWchar_t;
            break;
          case Token_bool:
            type = CppIntegralType::TypeBool;
            break;
          case Token_short:
            modifiers |= CppIntegralType::ModifierShort;
            break;
          case Token_int:
            type = CppIntegralType::TypeInt;
            break;
          case Token_long:
            if (modifiers & CppIntegralType::ModifierLong)
              modifiers |= CppIntegralType::ModifierLongLong;
            else
              modifiers |= CppIntegralType::ModifierLong;
            break;
          case Token_signed:
            modifiers |= CppIntegralType::ModifierSigned;
            break;
          case Token_unsigned:
            modifiers |= CppIntegralType::ModifierUnsigned;
            break;
          case Token_float:
            type = CppIntegralType::TypeFloat;
            break;
          case Token_double:
            type = CppIntegralType::TypeDouble;
            break;
          case Token_void:
            type = CppIntegralType::TypeVoid;
            break;
        }

        it = it->next;
      } while (it != end);

      if(type == CppIntegralType::TypeNone)
        type = CppIntegralType::TypeInt; //Happens, example: "unsigned short"

      CppIntegralType::Ptr integral = TypeRepository::self()->integral(type, modifiers/*, parseConstVolatile(node->cv)*/);
      if (integral)
        m_lastType = AbstractType::Ptr(integral.data());
    } else {
      visitTypeSpecifier(node);
    }
  }
  

  //Used to parse pointer-depth and cv-qualifies of types in new-expessions and casts
  void ExpressionVisitor::visitDeclarator(DeclaratorAST* node)  {
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    
    if( !m_lastType ) {
      problem(node, "Declarator used without type");
      return;
    }
    
    if( m_lastInstance ) {
      problem(node, "Declarator used on an instance instead of a type");
      return;
    }
    
    LOCKDUCHAIN;
    if( node->array_dimensions ) {
      ///@todo cv-qualifiers
      CppArrayType::Ptr p( new CppArrayType() );
      p->setElementType( m_lastType );
      
      m_lastType = AbstractType::Ptr( TypeRepository::self()->registerType( AbstractType::Ptr(p.data()) ).data() );
      m_lastInstance = Instance(false);
    }

    visitNodes(this, node->ptr_ops);
  }

  void ExpressionVisitor::visitNewDeclarator(NewDeclaratorAST* node)  {
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    
    if( !m_lastType ) {
      problem(node, "Declarator used without type");
      return;
    }
    
    if( m_lastInstance ) {
      problem(node, "Declarator used on an instance instead of a type");
      return;
    }
    
    AbstractType::Ptr lastType = m_lastType;
    Instance instance = m_lastInstance;

    DefaultVisitor::visitNewDeclarator(node);
    
    m_lastType = lastType;
    m_lastInstance = instance;

    LOCKDUCHAIN;
    visit(node->ptr_op);
  }
  
  void ExpressionVisitor::visitCppCastExpression(CppCastExpressionAST* node)  {
    
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    
    //Visit the expression just so it is evaluated and expressionType(..) eventually called, the result will not be used here
    clearLast();
    visit( node->expression );
    clearLast();

    if( node->type_id )
      visit(node->type_id);
    
    if( !m_lastType ) {
      problem(node, "Could not resolve type");
      return;
    }

    m_lastInstance = Instance(true);
    
    if( m_lastType )
      expressionType( node, m_lastType, m_lastInstance );

    visitSubExpressions( node, node->sub_expressions );
  }  
  //Used to parse pointer-depth and cv-qualifies of types in new-expessions and casts
  void ExpressionVisitor::visitPtrOperator(PtrOperatorAST* node) {
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    
    if( !m_lastType ) {
      problem(node, "Pointer-operator used without type");
      return;
    }
    
    if( m_lastInstance ) {
      problem(node, "Pointer-operator used on an instance instead of a type");
      return;
    }
  
    LOCKDUCHAIN;
    ///@todo cv-qualifiers
    CppPointerType::Ptr p( new CppPointerType( KDevelop::Declaration::CVNone) );
    p->setBaseType( m_lastType );

    m_lastType = AbstractType::Ptr( TypeRepository::self()->registerType( AbstractType::Ptr(p.data()) ).data() );
    m_lastInstance = Instance(false);
  }
  
  /**
   *
   * Has test */
  void ExpressionVisitor::visitCastExpression(CastExpressionAST* node)  {

    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    
    //Visit the expression just so it is evaluated and expressionType(..) eventually called, the result will not be used here
    clearLast();
    
    visit( node->expression );
    
    clearLast();

    //Visit declarator and type-specifier, which should build the type
    if( node->type_id ) {
      visit(node->type_id->type_specifier);
      visit(node->type_id->declarator);
    }
    if( !m_lastType ) {
      problem(node, "Could not resolve type");
      return;
    }

    m_lastInstance = Instance(true);
    
    if( m_lastType )
      expressionType( node, m_lastType, m_lastInstance );
  }

  void ExpressionVisitor::visitNewExpression(NewExpressionAST* node)  {
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    clearLast();
    visit( node->expression );
    clearLast();

    //Visit declarator and type-specifier, which should build the type
    if( node->type_id ) {
      visit(node->type_id->type_specifier);
      visit(node->type_id->declarator);
    } else if( node->new_type_id ) {
      visit(node->new_type_id->type_specifier);
      visit(node->new_type_id->new_declarator);
    }
    if( m_lastType )
    {
      LOCKDUCHAIN;
      ///@todo cv-qualifiers
      CppPointerType::Ptr p( new CppPointerType( KDevelop::Declaration::CVNone) );
      p->setBaseType( m_lastType );

      m_lastType = AbstractType::Ptr( TypeRepository::self()->registerType( AbstractType::Ptr(p.data()) ).data() );
      
      
      m_lastInstance = Instance(true);
      
      if( m_lastType )
        expressionType( node, m_lastType, m_lastInstance );
    }else{
      problem(node, "Could not resolve type");
    }

    AbstractType::Ptr lastType = m_lastType;
    Instance instance = m_lastInstance;

    visit(node->new_initializer);
    
    m_lastType = lastType;
    m_lastInstance = instance;
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
    
    if( dynamic_cast<DelayedType*>(m_lastType.data()) ) {
      //Store the expression so it's evaluated later
      m_lastInstance = Instance(true);
      createDelayedType(node);
      return;
    }

    AbstractType::Ptr conditionType = m_lastType;
    
    clearLast();
    visit(node->left_expression);
    AbstractType::Ptr leftType = m_lastType;
    clearLast();

    
    ///@todo test if result of right expression can be converted to the result of the right expression. If not, post a problem(because c++ wants it that way)

    //Since both possible results of a conditional expression must have the same type, we only consider the right one here
    visit(node->right_expression);

    {
      LOCKDUCHAIN;
      if( CppConstantIntegralType* condition = dynamic_cast<CppConstantIntegralType*>( conditionType.data() ) ) {
        ///For constant integral types, the condition could be evaluated, so we choose the correct result.
        if( condition->value<quint64>() == 0 ) {
          ///The right expression is the correct one, so do nothing
        } else {
          ///Condition is true, so we choose the left expression value/type
          m_lastType = leftType;
        }
      }
    }
    
    
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
    kDebug(9007) << "expression kind" << node->kind;
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

  bool ExpressionVisitor::dereferenceLastPointer(AST* node) {
    if( CppPointerType* pt = dynamic_cast<CppPointerType*>( realLastType().data() ) )
    { ///@todo what about const in pointer?
      //Dereference
      m_lastType = pt->baseType();
      m_lastInstance = Instance( getDeclaration(node,m_lastType) );
      return true;
    }else if( CppArrayType* pt = dynamic_cast<CppArrayType*>( realLastType().data() ) ) {
      m_lastType = pt->elementType();
      m_lastInstance = Instance( getDeclaration(node,m_lastType) );
      return true;
    }else{
      return false;
    }
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

    if( dynamic_cast<DelayedType*>(m_lastType.data()) ) {
      //Store the expression so it's evaluated later
      m_lastInstance = Instance(true);
      createDelayedType(node);
      return;
    }
    
    switch( tokenFromIndex(node->op).kind ) {
    case '*':
    {
      LOCKDUCHAIN;
      if( dereferenceLastPointer(node) ) {
      } else {
        //Get return-value of operator*
        findMember(node, m_lastType, QualifiedIdentifier("operator*") );
        if( !m_lastType ) {
          problem( node, "no overloaded operator* found" );
          return;
        }
        
        getReturnValue(node);
        
        if( !m_lastDeclarations.isEmpty() ) {
          DeclarationPointer decl( m_lastDeclarations.first() );
          lock.unlock();
          newUse( node, node->op, node->op+1, decl );
        }
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
    default:
    {
      CppIntegralType* integral = dynamic_cast<CppIntegralType*>(m_lastType.data());
      if( integral ) {
        //The type of integral types does not change on unary operators
        //Eventually evaluate the value of constant integral types
        CppConstantIntegralType* constantIntegral = dynamic_cast<CppConstantIntegralType*>(integral);

        if( constantIntegral ) {

          switch( constantIntegral->integralType() ) {
            case CppIntegralType::TypeFloat:
            {
              ConstantUnaryExpressionEvaluator<float> evaluator( tokenFromIndex(node->op).kind, constantIntegral );
              m_lastType = evaluator.createType();
              break;
            }
            case CppIntegralType::TypeDouble:
            {
              ConstantUnaryExpressionEvaluator<double> evaluator( tokenFromIndex(node->op).kind, constantIntegral );
              m_lastType = evaluator.createType();
              break;
            }
            default:
              if( constantIntegral->typeModifiers() & CppIntegralType::ModifierUnsigned ) {
                ConstantUnaryExpressionEvaluator<quint64> evaluator( tokenFromIndex(node->op).kind, constantIntegral );
                m_lastType = evaluator.createType();
              } else {
                ConstantUnaryExpressionEvaluator<qint64> evaluator( tokenFromIndex(node->op).kind, constantIntegral );
                m_lastType = evaluator.createType();
              }
              break;
          }

          m_lastInstance = Instance(true);
        }
      } else {
        QString op = operatorNameFromTokenKind(tokenFromIndex(node->op).kind);
        if( !op.isEmpty() )
        {
          LOCKDUCHAIN;
          KDevelop::DUContextPointer ptr(m_currentContext);
          OverloadResolutionHelper helper(ptr);
          helper.setOperator( OverloadResolver::Parameter(m_lastType.data(), isLValue( m_lastType, m_lastInstance ) ), op );

          //helper.setKnownParameters( OverloadResolver::Parameter(rightType, isLValue( rightType, rightInstance ) ) );
          QList<OverloadResolutionFunction> functions = helper.resolve(false);
          
          if( !functions.isEmpty() )
          {
            CppFunctionType::Ptr function = functions.first().function.declaration()->type<CppFunctionType>();
            if( functions.first().function.isViable() && function ) {
              m_lastType = function->returnType();
              m_lastInstance = Instance(true);

              lock.unlock();
              newUse( node, node->op, node->op+1, functions.first().function.declaration() );
            }else{
              problem(node, QString("Found no viable function"));
            }
          }else{
            //Do not complain here, because we do not check for builtin operators
            //problem(node, "No fitting operator. found" );
          }
          
        }else{
          problem(node, "Invalid unary expression");
        }
      }
    }
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
    m_lastInstance = Instance(f->declaration());//Mark instances of function return-types with the function they were returned by
  }
  
  void ExpressionVisitor::visitFunctionCall(FunctionCallAST* node) {
    PushPositiveContext pushContext( m_currentContext, node->ducontext );

    /**
     * If a class-name was found, get its constructors.
     *
     * @todo Think uses should be built for typedef class constructors.
     *       Normally, it should be a use of the typedef class, so refactoring can work
     *       correctly. However we also want to know which constructor was called in other places.
     * */
    CppClassType::Ptr constructedType;

    {
      LOCKDUCHAIN;
      if( !m_lastDeclarations.isEmpty() && m_lastDeclarations.first()->kind() == Declaration::Type && (constructedType = m_lastDeclarations.first()->logicalDeclaration(topContext())->type<CppClassType>()) ) {
        
        if( constructedType && constructedType->declaration() && constructedType->declaration()->internalContext() )
        {
          m_lastDeclarations = convert(constructedType->declaration()->internalContext()->findLocalDeclarations( QualifiedIdentifier(constructedType->declaration()->identifier()), constructedType->declaration()->internalContext()->range().end, topContext(), AbstractType::Ptr(), true, DUContext::OnlyFunctions ));
        }
      }
    }
    
    if( m_lastDeclarations.isEmpty() && !constructedType ) {
      problem( node, "function-call: no matching declarations found" );
      return;
    }
    
    /**
     * Step 1: Evaluate the function-argument types. Those are represented a little strangely:
     * node->arguments contains them, using recursive binary expressions
     * */
    QList<DeclarationPointer> declarations = m_lastDeclarations;

    QList<OverloadResolver::Parameter> oldParams = m_parameters;
    m_parameters.clear();
    //kDebug(9007) << "clearing parameters";

    //Backup the current use and invalidate it, we will update and create it after overload-resolution
    CurrentUse oldCurrentUse = m_currentUse;
    m_currentUse.isValid = false;
    
    clearLast();
    visit(node->arguments);

    //binary expressions don't yield m_lastType, so when m_lastType is set wo probably only have one single parameter
    if( m_lastType ) {
      m_parameters << OverloadResolver::Parameter( m_lastType.data(), isLValue( m_lastType, m_lastInstance ) );
      //LOCKDUCHAIN;
      //kDebug(9007) << "adding last parameter: " << (m_lastType.data() ? m_lastType->toString() : QString("<notype>"));
    }
    
    //Check if all parameters could be evaluated
    int paramNum = 1;
    bool fail = false;
    for( QList<OverloadResolver::Parameter>::const_iterator it = m_parameters.begin(); it != m_parameters.end(); ++it ) {
      if( !(*it).type ) {
        problem( node, QString("parameter %1 could not be evaluated").arg(paramNum) );
        fail = true;
        paramNum++;
      }
    }
    LOCKDUCHAIN;

    if(declarations.isEmpty() && constructedType) {
      //Default-constructor is used
      m_lastType = AbstractType::Ptr(constructedType.data());
      m_lastInstance = Instance(constructedType->declaration());
      DeclarationPointer decl(constructedType->declaration());
      lock.unlock();
      newUse( oldCurrentUse.node, oldCurrentUse.start_token, oldCurrentUse.end_token, decl );
      return;
    }
    
    //Resolve functions
    DeclarationPointer chosenFunction;
    KDevelop::DUContextPointer ptr(m_currentContext);
    OverloadResolver resolver( ptr );

    if( !fail ) {
      chosenFunction = resolver.resolveList(m_parameters, convert(declarations));
    }

    if( !chosenFunction && !m_strict ) {
      //Because we do not want to rely too much on our understanding of the code, we take the first function instead of totally failing.
      QString params;
      foreach(const OverloadResolver::Parameter& param, m_parameters)
        params += param.toString() + ", ";
      
      QString candidates;
      foreach(DeclarationPointer decl, declarations) {
        if( !decl )
          continue;
        int defaultParamCount = 0;
        if( AbstractFunctionDeclaration* aDec = dynamic_cast<AbstractFunctionDeclaration*>(decl.data()) )
          defaultParamCount = aDec->defaultParameters().count();
        
        candidates += decl->toString() + QString(" default-params: %1").arg(defaultParamCount) + '\n';
      }
      
      problem(node, QString("Could not find a function that matches the parameters. Using first candidate function. Parameters: %1 Candidates: %2").arg(params).arg(candidates));
      fail = true;
    }

    if( fail ) {
      //Since not all parameters could be evaluated, Choose the first function
      chosenFunction = declarations.front();
    }

    m_parameters = oldParams;
    //kDebug(9007) << "Resetting old parameters of size " << oldParams.size();
    
    clearLast();

    if( constructedType ) {
      //Constructor was called
      m_lastType = AbstractType::Ptr(constructedType.data());
      m_lastInstance = Instance(constructedType->declaration());
    } else {
      CppFunctionType* functionType = dynamic_cast<CppFunctionType*>( chosenFunction->abstractType().data() );
      if( !chosenFunction || !functionType ) {
        problem( node, QString( "could not find a matching function for function-call" ) );
      } else {
        m_lastType = functionType->returnType();
        m_lastInstance = Instance(chosenFunction);
      }
    }

    //Re-create the use we have discarded earlier, this time with the correct overloaded function chosen.
    lock.unlock();
    newUse( oldCurrentUse.node, oldCurrentUse.start_token, oldCurrentUse.end_token, chosenFunction );

    if( m_lastType )
      expressionType( node, m_lastType, m_lastInstance );
  }
  
  void ExpressionVisitor::visitSubscriptExpression(SubscriptExpressionAST* node)
  {
    ///@todo create use
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    
    Instance masterInstance = m_lastInstance;
    AbstractType::Ptr masterType = m_lastType;

    if( !masterType || !masterInstance ) {
      problem(node, "Tried subscript-expression on invalid object");
      return;
    }

    {
      LOCKDUCHAIN;

      //If the type the subscript-operator is applied on is a pointer, dereference it
      if( dereferenceLastPointer(node) ) {
        //Make visit the sub-expression, so uses are built
        lock.unlock();
        
        masterInstance = m_lastInstance;
        masterType = m_lastType;
        
        visit(node->subscript); //Visit so uses are built
        clearLast();
        
        m_lastType = masterType;
        m_lastInstance = masterInstance;
        return;
      }
    }

    clearLast();
    
    visit(node->subscript);

    LOCKDUCHAIN;

    KDevelop::DUContextPointer ptr(m_currentContext);
    OverloadResolutionHelper helper(ptr);
    helper.setOperator( OverloadResolver::Parameter(masterType.data(), isLValue( masterType, masterInstance ) ), "[]" );

    helper.setKnownParameters( OverloadResolver::Parameter( m_lastType.data(), isLValue( m_lastType, m_lastInstance ) ) );
    QList<OverloadResolutionFunction> functions = helper.resolve(false);

    if( !functions.isEmpty() )
    {
      CppFunctionType::Ptr function = functions.first().function.declaration()->type<CppFunctionType>();
      
      if( function ) {
        m_lastType = function->returnType();
        m_lastInstance = Instance(true);
      }else{
        clearLast();
        problem(node, QString("Found no subscript-function"));
      }
      
      if( !functions.first().function.isViable() ) {
        problem(node, QString("Found no viable subscript-function, chosen function: %1").arg(functions.first().function.declaration() ? functions.first().function.declaration()->toString() : QString()));
      }
        
    }else{
      clearLast();
      //Do not complain here, because we do not check for builtin operators
      //problem(node, "No fitting operator. found" );
    }
  }
  
  void ExpressionVisitor::visitSizeofExpression(SizeofExpressionAST* node)  {
    LOCKDUCHAIN;
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    m_lastType = AbstractType::Ptr( TypeRepository::self()->integral(CppIntegralType::TypeInt, CppIntegralType::ModifierNone, KDevelop::Declaration::CVNone).data() );
    m_lastInstance = Instance(true);
  }
  
  void ExpressionVisitor::visitCondition(ConditionAST* node)  {
    LOCKDUCHAIN;
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    m_lastType = AbstractType::Ptr( TypeRepository::self()->integral(CppIntegralType::TypeBool, CppIntegralType::ModifierNone, KDevelop::Declaration::CVNone).data() );
    m_lastInstance = Instance(true);
  }
  
  void ExpressionVisitor::visitTypeId(TypeIdAST* type_id)  {
    PushPositiveContext pushContext( m_currentContext, type_id->ducontext );
    visit(type_id->type_specifier);
    visit(type_id->declarator);
  }
  
  void ExpressionVisitor::visitStringLiteral(StringLiteralAST* node)  {
    LOCKDUCHAIN;
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    CppPointerType::Ptr p( new CppPointerType( KDevelop::Declaration::Const) );
    p->setBaseType( AbstractType::Ptr(TypeRepository::self()->integral(CppIntegralType::TypeChar, CppIntegralType::ModifierNone, KDevelop::Declaration::CVNone).data()) );
    
    m_lastType = AbstractType::Ptr( TypeRepository::self()->registerType( AbstractType::Ptr(p.data()) ).data() );
    m_lastInstance = Instance(true);
  }
  
  void ExpressionVisitor::visitIncrDecrExpression(IncrDecrExpressionAST* node)  {

    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    
    ///post-fix increment/decrement like "i++" or "i--"
    ///This does neither change the evaluated value, nor the type(except for overloaded operators)

    if( dynamic_cast<CppIntegralType*>(m_lastType.data()) ) {
      ///Leave the type and its value alone
    } else {
      ///It is not an integral type, try finding an overloaded operator and use the return-value
      QString op = operatorNameFromTokenKind(tokenFromIndex(node->op).kind);
      if( !op.isEmpty() )
      {
        LOCKDUCHAIN;
        KDevelop::DUContextPointer ptr(m_currentContext);
        OverloadResolutionHelper helper(ptr);
        helper.setOperator( OverloadResolver::Parameter(m_lastType.data(), isLValue( m_lastType, m_lastInstance ) ), op );

        //Overloaded postfix operators have one additional int parameter
        static AbstractType::Ptr integer = TypeRepository::self()->registerType( AbstractType::Ptr(new CppConstantIntegralType(CppConstantIntegralType::TypeInt, CppIntegralType::ModifierNone)) );
        helper.setKnownParameters( OverloadResolver::Parameter( integer.data(), false ) );
        
        QList<OverloadResolutionFunction> functions = helper.resolve(false);

        if( !functions.isEmpty() )
        {
          CppFunctionType::Ptr function = functions.first().function.declaration()->type<CppFunctionType>();
          if( functions.first().function.isViable() && function ) {
            m_lastType = function->returnType();
            m_lastInstance = Instance(true);
          }else{
            problem(node, QString("Found no viable function"));
          }

          lock.unlock();
          newUse( node, node->op, node->op+1, functions.first().function.declaration() );
        }else{
          //Do not complain here, because we do not check for builtin operators
          //problem(node, "No fitting operator. found" );
        }
      }
    }

    if( m_lastType )
      expressionType( node, m_lastType, m_lastInstance );
  }
  
  void ExpressionVisitor::visitNewTypeId(NewTypeIdAST* node)  {
    //Return a pointer to the type
    problem(node, "node-type cannot be parsed");
  }
  
  void ExpressionVisitor::visitSimpleDeclaration(SimpleDeclarationAST* node)  {
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    ///Simple type-specifiers like "int" are parsed as SimpleDeclarationAST, so treat them here.
    visit( node->type_specifier );
  }
  
  void ExpressionVisitor::visitDeclarationStatement(DeclarationStatementAST* node)  {
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    ///Simple type-specifiers like "int" are parsed as SimpleDeclarationAST, so treat them here.
    visit( node->declaration );
  }
  void ExpressionVisitor::visitThrowExpression(ThrowExpressionAST* node)  {
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    visit( node->expression );
  }
  void ExpressionVisitor::visitDeleteExpression(DeleteExpressionAST* node)  {
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    visit( node->expression );
  }

  void ExpressionVisitor::visitNewInitializer(NewInitializerAST* node)  {
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
    visit(node->expression);
  }
  
  ///Nodes that are invalid inside an expression:
  void ExpressionVisitor::visitPtrToMember(PtrToMemberAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitOperatorFunctionId(OperatorFunctionIdAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitTypeIdentification(TypeIdentificationAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitUnqualifiedName(UnqualifiedNameAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitOperator(OperatorAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitAccessSpecifier(AccessSpecifierAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitAsmDefinition(AsmDefinitionAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitBaseClause(BaseClauseAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitBaseSpecifier(BaseSpecifierAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitClassSpecifier(ClassSpecifierAST* node)  { problem(node, "node-type cannot be parsed"); }

  void ExpressionVisitor::visitCtorInitializer(CtorInitializerAST* node)  { problem(node, "node-type cannot be parsed"); }
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
  void ExpressionVisitor::visitParameterDeclaration(ParameterDeclarationAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitParameterDeclarationClause(ParameterDeclarationClauseAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitReturnStatement(ReturnStatementAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitSwitchStatement(SwitchStatementAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitTemplateArgument(TemplateArgumentAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitTemplateDeclaration(TemplateDeclarationAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitTemplateParameter(TemplateParameterAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitTryBlockStatement(TryBlockStatementAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitTypeParameter(TypeParameterAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitTypedef(TypedefAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitUsing(UsingAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitUsingDirective(UsingDirectiveAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitWhileStatement(WhileStatementAST* node)  { problem(node, "node-type cannot be parsed"); }
  void ExpressionVisitor::visitWinDeclSpec(WinDeclSpecAST* node)  { problem(node, "node-type cannot be parsed"); }
}

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

#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <parsesession.h>
#include <language/duchain/declaration.h>
#include <language/duchain/functiondefinition.h>
#include <language/duchain/types/identifiedtype.h>
#include <typeinfo>
#include <util/pushvalue.h>
#include "tokens.h"
#include "typebuilder.h"
#include "cpptypes.h"
#include <language/duchain/dumpchain.h>
#include "typeutils.h"
#include "name_visitor.h"
#include "type_visitor.h"
#include "lexer.h"
#include "overloadresolution.h"
#include "cppduchain.h"
#include "overloadresolutionhelper.h"
#include "builtinoperators.h"
#include "qtfunctiondeclaration.h"
#include "missingdeclarationtype.h"
#include "missingdeclarationproblem.h"
#include "dumpchain.h"

//If this is enabled and a type is not found, it is searched again with verbose debug output.
//#define DEBUG_RESOLUTION_PROBLEMS

//If this is enabled, all encounterd problems will be dumped to kDebug
// #define DUMP_PROBLEMS

//If this is enabled, problems will be created when no overloaded function was found for a function-call. This is expensive,
//because the problem report contains a lot of information, and the problem currently appears very often.
//#define DEBUG_FUNCTION_CALLS

// uncomment to get debugging info on ADL - very expensive on parsing
//#define DEBUG_ADL

const int maxExpressionVisitorProblems = 400;

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

QString operatorNameFromTokenKind( quint16 tokenKind )
{
  return token_text(tokenKind);
}

QList<DeclarationPointer> convert( const QList<Declaration*>& list ) {
  QList<DeclarationPointer> ret;
  foreach( Declaration* decl, list )
    ret << DeclarationPointer(decl);
  return ret;
}

QList<Declaration*> convert( const QList<DeclarationPointer>& list ) {
  QList<Declaration*> ret;
  foreach( const DeclarationPointer &decl, list )
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

const Token& ExpressionVisitor::tokenFromIndex( int index ) {
  return m_session->token_stream->token(index);
}


typedef PushValue<AbstractType::Ptr> PushAbstractType;

const TopDUContext* ExpressionVisitor::topContext() const {
  if( m_source ) {
    return m_source;
  }else{
    return m_topContext;
  }
}

bool ExpressionVisitor::isLValue( const AbstractType::Ptr& type, const Instance& instance ) {
  return instance && (instance.declaration || isReferenceType(type));
}

ExpressionVisitor::ExpressionVisitor(ParseSession* session, const KDevelop::TopDUContext* source,
                                     bool strict, bool propagateConstness, bool mapAst)
: m_strict(strict)
, m_memberAccess(false)
, m_skipLastNamePart(false)
, m_mapAst(mapAst)
, m_hadMemberAccess(false)
, m_source(source)
, m_ignore_uses(0)
, m_session(session)
, m_currentContext(0)
, m_topContext(0)
, m_reportRealProblems(false)
, m_propagateConstness(propagateConstness)
, m_handlingFunctionCallOrInit(false)
{
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
  ///WARNING: ::parse can be called recursivly by e.g. the name or type visitor!
  PushValue<const TopDUContext*> pushTopContext(m_topContext, ast->ducontext->topContext());
  visit(ast);
  Q_ASSERT(m_topContext);
  flushUse();
}

void ExpressionVisitor::parseNamePrefix( NameAST* ast ) {
  PushValue<bool> p(m_skipLastNamePart, true);
  parse(ast);
}

void ExpressionVisitor::reportRealProblems(bool report) {
  m_reportRealProblems = report;
}

void ExpressionVisitor::realProblem( const ProblemPointer& problem ) {
  if(m_reportRealProblems && m_problems.size() < maxExpressionVisitorProblems) {
    m_problems << problem;
  }
}

QList<ProblemPointer> ExpressionVisitor::realProblems() const {
  return m_problems;
}

void ExpressionVisitor::problem( AST* node, const QString& str ) {
#ifdef DUMP_PROBLEMS
  kDebug(9007) << "Cpp::ExpressionVisitor problem:" << str;

  kDebug(9007) << "Cpp::ExpressionVisitor dumping the node that created the problem";
  Cpp::DumpChain d;

  d.dump(node, m_session);
#else
  Q_UNUSED(node);
  Q_UNUSED(str);
#endif
}

AbstractType::Ptr ExpressionVisitor::lastType() {
  return m_lastType;
}

ExpressionVisitor::Instance ExpressionVisitor::lastInstance() {
  return m_lastInstance;
}

const DUContext* ExpressionVisitor::currentContext() const
{
  return m_currentContext;
}

/** Find the member in the declaration's du-chain. **/
void ExpressionVisitor::findMember( AST* node, AbstractType::Ptr base, const Identifier& member, bool isConst, bool postProblem ) {
    LOCKDUCHAIN;

    base = realType(base, topContext());

    clearLast();

    isConst |= isConstant(base);

    //Make sure that it is a structure-type, because other types do not have members
    const StructureType::Ptr& structureType = base.cast<StructureType>();

    if( !structureType ) {
      problem( node, QString("findMember called on non-identified or non-structure type \"%1\"").arg(base ? base->toString() : "<type disappeared>") );
      return;
    }

    Declaration* declaration = structureType->declaration(topContext());
    MUST_HAVE(declaration);
    MUST_HAVE(declaration->context());

    DUContext* internalContext = declaration->logicalInternalContext(topContext());

    MUST_HAVE( internalContext );

  m_lastDeclarations = convert(findLocalDeclarations( internalContext, member, topContext() ));


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
    for( QList<DeclarationPointer>::const_iterator it = m_lastDeclarations.constBegin(); it != m_lastDeclarations.constEnd(); ++it ) {
      AbstractType::Ptr t = (*it)->abstractType();
      if( t ) {
        if( (t->modifiers() & AbstractType::ConstModifier) == isConst ) {
          m_lastType = t;
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

        PointerType::Ptr pnt = realType(m_lastType, topContext()).cast<PointerType>();
        if( pnt ) {
/*          kDebug(9007) << "got type:" << pnt->toString();
          kDebug(9007) << "base-type:" << pnt->baseType()->toString();*/

          isConst = isConstant(pnt.cast<AbstractType>());
          //It is a pointer, reduce the pointer-depth by one
          m_lastType = pnt->baseType();
          m_lastInstance = Instance( getDeclaration(m_lastType) );
        } else {
          findMember( node, m_lastType, Identifier("operator->") );
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
          if(m_mapAst) session()->mapCallAstToType(node, m_lastType.cast<FunctionType>());

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


  AbstractType::Ptr ExpressionVisitor::realLastType() const
  {
    LOCKDUCHAIN;
    return AbstractType::Ptr(realType( m_lastType, topContext() ));
  }

  bool ExpressionVisitor::getPointerTarget( AST* node, bool* constant )  {
    if( !m_lastType ) return false;

    AbstractType::Ptr base = realLastType();

    clearLast();

    const PointerType::Ptr& pnt = base.cast<PointerType>();
    if( pnt ) {
      if( constant )
        (*constant) |= (pnt->modifiers() & AbstractType::ConstModifier);
      m_lastType = pnt->baseType();
      m_lastInstance = Instance(getDeclaration(m_lastType));
      return true;
    } else {
      LOCKDUCHAIN;
      QString typeStr;
      if (base) {
        typeStr = base->toString();
      } else {
        typeStr = "<notype>";
      }
      problem(node, QString("Cannot dereference base-type \"%1\"").arg(typeStr) );
      return false;
    }
  }

  Declaration* ExpressionVisitor::getDeclaration( const AbstractType::Ptr& base ) {
    if( !base ) return 0;

    const IdentifiedType* idType = dynamic_cast<const IdentifiedType*>(base.unsafeData());
    if( idType ) {
      LOCKDUCHAIN;
      return idType->declaration(topContext());
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
    Q_ASSERT(m_currentContext); // required later on

    const DUContext* searchInContext = m_currentContext;

    m_hadMemberAccess = m_memberAccess;
    
    CursorInRevision position = m_session->positionAt( m_session->token_stream->position(node->start_token) );
    if( m_currentContext->url() != m_session->m_url ) //.equals( m_session->m_url, KUrl::CompareWithoutTrailingSlash ) )
      position = position.invalid();

    bool isConst = false;

    if( m_memberAccess ) {
      LOCKDUCHAIN;

      m_lastType = realType(m_lastType, topContext());

      isConst |= isConstant(m_lastType);

      //Make sure that it is a structure-type, because other types do not have members
      const StructureType::Ptr& structureType = m_lastType.cast<StructureType>();

      if( !structureType ) {
        problem( node, QString("member searched in non-identified or non-structure type \"%1\"").arg(m_lastType ? m_lastType->toString() : "<type disappeared>") );
        clearLast();
        return;
      }

      Declaration* declaration = structureType->declaration(topContext());
      MUST_HAVE(declaration);
      MUST_HAVE(declaration->context());

      searchInContext = declaration->logicalInternalContext(topContext());

      MUST_HAVE( searchInContext );
    }

    clearLast();

    NameASTVisitor nameV( m_session, this, searchInContext, topContext(), m_currentContext, position.isValid() ? position : searchInContext->range().end, m_memberAccess ? DUContext::DontSearchInParent : DUContext::NoSearchFlags );
    nameV.run(node, m_skipLastNamePart);

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
      ConstantIntegralType::Ptr type(new ConstantIntegralType(IntegralType::TypeBoolean));
      type->setValue<qint64>( identifier == trueIdentifier );
      m_lastType = type.cast<AbstractType>();
      m_lastInstance = Instance( true );
    } else {
      LOCKDUCHAIN;

      m_lastDeclarations = nameV.declarations();

      if( m_lastDeclarations.isEmpty() || !m_lastDeclarations.first().data() ) {

        if(Cpp::isTemplateDependent(m_currentContext) ) {
          if(m_memberAccess || (node->qualified_names && nameV.foundSomething() && Cpp::isTemplateDependent(nameV.foundSomething().data()))) {
          //Do nothing. Within a not instantiated template, we cannot be that sure
          m_lastType.clear();
          return;
          }
        }

        MissingDeclarationType::Ptr missing(new MissingDeclarationType);

        missing->setIdentifier(IndexedTypeIdentifier(nameV.identifier()));
        if(m_memberAccess)
          missing->containerContext = const_cast<DUContext*>(searchInContext);

        missing->searchStartContext = const_cast<DUContext*>(m_currentContext);

        if(m_reportRealProblems && m_problems.size() < maxExpressionVisitorProblems) {
          ProblemPointer problem(new Cpp::MissingDeclarationProblem(missing));
          problem->setSource(KDevelop::ProblemData::SemanticAnalysis);
          CppEditorIntegrator editor(session());
          
          problem->setFinalLocation(DocumentRange(m_currentContext->url(), editor.findRange(node).castToSimpleRange()));
          if(!problem->range().isEmpty() && !editor.findRangeForContext(node->start_token, node->end_token).isEmpty())
            m_problems << problem;
        }
        m_lastType = missing.cast<KDevelop::AbstractType>();

        problem( node, QString("could not find declaration of %1").arg( nameV.identifier().toString() ) );
      } else {
        // by default ignore constness (see below)
        m_lastType = m_lastDeclarations.first()->abstractType();
        // if possible, pick the const-fitting method though
        foreach(const DeclarationPointer& p, m_lastDeclarations) {
          if (p->abstractType() && isConstant(p->abstractType()) == isConst) {
            m_lastType = p->abstractType();
            break;
          }
        }
        if (m_propagateConstness && isConst && m_lastType && !isConstant(m_lastType)) {
          m_lastType->setModifiers(m_lastType->modifiers() | AbstractType::ConstModifier);
        }
        //kDebug(9007) << "found declaration: " << m_lastDeclarations.first()->toString();

        ///If the found declaration declares a type, this is a type-expression and m_lastInstance should be zero.
        ///The declaration declares a type if its abstractType's declaration is that declaration. Else it is an insantiation, and m_lastType should be filled.

        if( m_lastDeclarations.first()->kind() == Declaration::Instance )
          m_lastInstance = Instance( m_lastDeclarations.first() );
        else
          m_lastInstance = Instance(false);

        //A CppTemplateParameterType represents an unresolved template-parameter, so create a DelayedType instead.
        if( m_lastType.cast<CppTemplateParameterType>() )
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
    clearLast();

    switch (node->type) {
      case PrimaryExpressionAST::Literal:
        visit( node->literal );
        break;
      case PrimaryExpressionAST::Name:
        visit( node->name );
        break;
      case PrimaryExpressionAST::SubExpression:
        visit( node->sub_expression );
        break;
      case PrimaryExpressionAST::Statement:
        visit( node->expression_statement );
        break;
      case PrimaryExpressionAST::Token:
        visitExpressionToken( node->token, node );
        break;
    }

    if ( m_lastType ) {
      expressionType( node, m_lastType, m_lastInstance );
    }
  }

  void ExpressionVisitor::visitExpressionToken(uint tokenIndex, AST* node)
  {
    const Token& token(tokenFromIndex(tokenIndex));

    if (token.kind == Token_number_literal) {
      QString num = m_session->token_stream->symbolString(token);

      LOCKDUCHAIN;
      if( num.indexOf('.') != -1 || num.endsWith('f') || num.endsWith('d') ) {
        double val = 0;
        bool ok = false;
        while( !num.isEmpty() && !ok ) {
          val = num.toDouble(&ok);
          num.truncate(num.length()-1);
        }


        if( num.endsWith('f') ) {
          ConstantIntegralType::Ptr type(new ConstantIntegralType(IntegralType::TypeFloat));
          type->setValue<float>((float)val);
          m_lastType = type.cast<AbstractType>();
        } else {
          ConstantIntegralType::Ptr type(new ConstantIntegralType(IntegralType::TypeDouble));
          type->setValue<double>(val);
          m_lastType = type.cast<AbstractType>();
        }
      } else {
        qint64 val = 0;
        uint mod = AbstractType::NoModifiers;

        if( num.endsWith("u") || ( num.length() > 1 && num[1] == 'x' ) )
          mod = AbstractType::UnsignedModifier;

        bool ok = false;
        while( !num.isEmpty() && !ok ) {
          val = num.toLongLong(&ok, 0);
          num.truncate(num.length()-1);
        }

        m_lastType = AbstractType::Ptr(new ConstantIntegralType(IntegralType::TypeInt));
        m_lastType->setModifiers(mod);

        if( mod & AbstractType::UnsignedModifier )
          ConstantIntegralType::Ptr::staticCast(m_lastType)->setValue<quint64>(val);
        else
          ConstantIntegralType::Ptr::staticCast(m_lastType)->setValue<qint64>(val);
      }
      m_lastInstance = Instance(true);

      return;
    } else if(token.kind == Token_char_literal) {
      // char literal e.g. 'x'
      LOCKDUCHAIN;
      ConstantIntegralType::Ptr charType(new ConstantIntegralType(IntegralType::TypeChar));
      if ( token.size == 3 ) {
        charType->setValue<char>( m_session->token_stream->symbolByteArray(token).at(1) );
      } else {
        QByteArray symbol = m_session->token_stream->symbolByteArray(token);
        if (symbol.startsWith('L')) {
          charType->setDataType(IntegralType::TypeWchar_t);
          symbol.right(symbol.size() - 1);
        } else if (symbol.startsWith('u')) {
          charType->setDataType(IntegralType::TypeChar16_t);
          symbol.right(symbol.size() - 1);
        } else if (symbol.startsWith('U')) {
          charType->setDataType(IntegralType::TypeChar32_t);
          symbol.right(symbol.size() - 1);
        }
        if (symbol.size() == 4) {
          if (symbol == "'\\t'") {
            charType->setValue<char>('\t');
          } else if (symbol == "'\\n'") {
            charType->setValue<char>('\n');
          } else if (symbol == "'\\r'") {
            charType->setValue<char>('\r');
          }
        }
      }

      m_lastType = charType.cast<AbstractType>();
      m_lastInstance = Instance( true );
    } else if (token.kind == Token_true || token.kind == Token_false) {
      ///We have a boolean constant, we need to catch that here
      LOCKDUCHAIN;
      ConstantIntegralType::Ptr type(new ConstantIntegralType(IntegralType::TypeBoolean));
      type->setValue<qint64>( token.kind == Token_true );
      m_lastType = type.cast<AbstractType>();
      m_lastInstance = Instance( true );
    } else if( token.kind == Token_this ) {
      LOCKDUCHAIN;

      AbstractType::Ptr thisType;

      const DUContext* context = m_currentContext; //Here we find the context of the function-declaration/definition we're currently in
      while( context->parentContext() && context->type() == DUContext::Other && context->parentContext()->type() == DUContext::Other )
      { //Move context to the top context of type "Other". This is needed because every compound-statement creates a new sub-context.
        context = context->parentContext();
      }

      ///Step 1: Find the function-declaration for the function we are in
      Declaration* functionDeclaration = 0;

      if( context->owner() && dynamic_cast<FunctionDefinition*>(context->owner()) )
        functionDeclaration = static_cast<FunctionDefinition*>(context->owner())->declaration(topContext());

      if( !functionDeclaration && context->owner() )
        functionDeclaration = context->owner();

      if( !functionDeclaration )
      {
        problem(node, "\"this\" used, but no function-declaration could be found");
        return;
      }

      ///Step 2: Find the type of "this" from the function-declaration
      DUContext* classContext = 0;

      if (TemplateDeclaration *templateDecl = dynamic_cast<TemplateDeclaration*>(functionDeclaration))
        if (templateDecl->specializedFrom().data())
          classContext = templateDecl->specializedFrom().data()->context();

      if (!classContext)
        classContext = functionDeclaration->context();

      //Take the type from the classContext
      if( classContext && classContext->type() == DUContext::Class && classContext->owner() )
        thisType = classContext->owner()->abstractType();

      if( !thisType ) {
        problem(node, "\"this\" used in invalid classContext");
        return;
      }

      ///Step 3: Create a pointer-type for the "this" type and return it
      KDevelop::FunctionType::Ptr cppFunction = functionDeclaration->abstractType().cast<KDevelop::FunctionType>();

      if( cppFunction ) {
        PointerType::Ptr thisPointer( new PointerType() );
        thisPointer->setModifiers(cppFunction->modifiers() & (AbstractType::ConstModifier | AbstractType::VolatileModifier));
        thisPointer->setBaseType( thisType );

        m_lastType = thisPointer.cast<AbstractType>();
        m_lastInstance = Instance(true);
        if(m_mapAst) session()->mapCallAstToType(node, cppFunction);
      }else{
        if( context->owner() && context->owner()->abstractType() )
          problem(node, QString("\"this\" used in non-function context of type %1(%2)").arg( "unknown" ) .arg(context->owner()->abstractType()->toString()));
        else
          problem(node, "\"this\" used in non-function context with invalid type");
      }
    } else {
      // TODO: handle nullptr
    }
  }

  /** Translation-units just forward to their encapsulated expression */
  void ExpressionVisitor::visitTranslationUnit(TranslationUnitAST* node)
  {
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

    bool onlyFunctionCalls = false;

    if( !m_lastType ) {
       problem( node, "primary expression returned no type" );
       onlyFunctionCalls = true; //We want to visit function-calls even when the function was not resolved, so we get uses for the arguments
    }
    const ListNode<ExpressionAST*> *it = nodes->toFront(), *end = it;

    int num = 0;
    do
      {
        if( !onlyFunctionCalls || (it->element && it->element->kind == AST::Kind_FunctionCall) )
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

  /** A postfix-expression is a primary expression (or type-specifier)
   * together with a chain of sub-expressions that are applied from left to right
   */

  void ExpressionVisitor::visitPostfixExpression(PostfixExpressionAST* node)
  {
    clearLast();
    
    //First evaluate the primary expression, or the type-specifier,
    //and then pass the result from sub-expression to sub-expression through m_lastType
    
    if( node->type_specifier )
      visit( node->type_specifier );

    if( node->expression )
      visit( node->expression );

    if( !node->sub_expressions )
      return;

    visitSubExpressions( node, node->sub_expressions );
  }

/** A helper-class for evaluating constant unary expressions under different types(int, float, etc.) */
template<class Type>
struct ConstantUnaryExpressionEvaluator {

  Type endValue;

  uint type;
  uint modifier;

  /**
   * Writes the results into endValue, type, and modifier.
   * */
  ConstantUnaryExpressionEvaluator( quint16 tokenKind, const ConstantIntegralType::Ptr& left ) {
    endValue = 0;
    type = left->dataType();
    modifier = left->modifiers();
    evaluateSpecialTokens( tokenKind, left );
    switch( tokenKind ) {
      case '+':
        endValue = +left->value<Type>();
      break;
      case '-':
        endValue = -left->value<Type>();
      break;
      case Token_incr:
        endValue = left->value<Type>()+1;
      break;
      case Token_decr:
        endValue = left->value<Type>()-1;
      break;
    }
  }

  //This function is used to disable some operators on bool and double values
  void evaluateSpecialTokens( quint16 tokenKind, const ConstantIntegralType::Ptr& left ) {
    switch( tokenKind ) {
      case '~':
        endValue = ~left->value<Type>();
      break;
      case '!':
        endValue = !left->value<Type>();
      break;
    }
  }

  AbstractType::Ptr createType() const
  {
    ConstantIntegralType::Ptr ret(new ConstantIntegralType(type));
    ret->setModifiers(modifier);
    ret->setValue<Type>( endValue );
    return ret.cast<AbstractType>();
  }
};

template<>
void ConstantUnaryExpressionEvaluator<double>::evaluateSpecialTokens( quint16 tokenKind, const ConstantIntegralType::Ptr& left ) {
  Q_UNUSED(tokenKind);
  Q_UNUSED(left);
}

template<>
void ConstantUnaryExpressionEvaluator<float>::evaluateSpecialTokens( quint16 tokenKind, const ConstantIntegralType::Ptr& left ) {
  Q_UNUSED(tokenKind);
  Q_UNUSED(left);
}

QString toString(AbstractType::Ptr t) {
  if(!t)
    return "<no type>";
  return t->toString();
}

void ExpressionVisitor::createDelayedType( AST* node , bool expression ) {
  DelayedType::Ptr type(new DelayedType());
  QString id = m_session->stringForNode(node, true);

  //We have  to prevent automatic parsing and splitting by QualifiedIdentifier and Identifier
  Identifier idd;
  idd.setIdentifier(id);

  QualifiedIdentifier ident;
  ident.push(idd);

  ident.setIsExpression( expression );
  type->setIdentifier( IndexedTypeIdentifier(ident) );
  m_lastType = type.cast<AbstractType>();
}

  /**
   *
   * partially have test **/
  void ExpressionVisitor::visitBinaryExpression(BinaryExpressionAST* node)  {
    clearLast();

    ///First resolve left part, then right, then combine
    visit(node->left_expression);

    Instance leftInstance = m_lastInstance;
    AbstractType::Ptr leftType = m_lastType;
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
        m_parameters << OverloadResolver::Parameter(leftType, isLValue( leftType, leftInstance ), leftInstance.declaration.data() );
        m_parameterNodes.append(node->left_expression);

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

          m_parameters << OverloadResolver::Parameter(AbstractType::Ptr(), false);
          m_parameterNodes.append(node->left_expression);
          //LOCKDUCHAIN;
          //kDebug(9007) << "Adding empty from left";
        }
      }
    }

    visit(node->right_expression);

    Instance rightInstance = m_lastInstance;
    AbstractType::Ptr rightType = m_lastType;
    clearLast();

    if( tokenFromIndex(node->op).kind == ',' ) {

      if( rightType && rightInstance) {
        m_parameters << OverloadResolver::Parameter(rightType, isLValue( rightType, rightInstance ), rightInstance.declaration.data() );
        m_parameterNodes.append(node->right_expression);
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

          m_parameters << OverloadResolver::Parameter(AbstractType::Ptr(), false);
          m_parameterNodes.append(node->right_expression);
          //kDebug(9007) << "Adding empty from right";
        }
      }

      clearLast();
      return;
    }

    if(MissingDeclarationType::Ptr missing = leftType.cast<Cpp::MissingDeclarationType>()) {
      if(rightType) {
        Cpp::ExpressionEvaluationResult res;
        res.type = rightType->indexed();
        res.isInstance = rightInstance;
        missing->assigned = res;
      }
      clearLast();
      return;
    }

    if(MissingDeclarationType::Ptr missing = rightType.cast<Cpp::MissingDeclarationType>()) {
      if(leftType) {
        Cpp::ExpressionEvaluationResult res;
        res.type = leftType->indexed();
        res.isInstance = leftInstance;
        missing->convertedTo = res;
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

    if( rightType.cast<DelayedType>() || leftType.cast<DelayedType>() ) {
      m_lastInstance = Instance(true);
      createDelayedType(node);
      return;
    }

    quint16 tokenKind = tokenFromIndex(node->op).kind;

    if(rightType && leftType && rightInstance && leftInstance) {
      LOCKDUCHAIN;
      //Test if there is a builtin operator that can be used. If it is, this will also evaluate the values of constant expressions.
      m_lastType = binaryOperatorReturnType(leftType, rightType, tokenKind);
      m_lastInstance = Instance(true);
    }
    if(!m_lastType) {
      QString op = operatorNameFromTokenKind(tokenFromIndex(node->op).kind);

      bool success = false;
      if( !op.isEmpty() )
      {
        LOCKDUCHAIN;
        OverloadResolutionHelper helper(
          DUContextPointer(const_cast<DUContext*>(m_currentContext)),
          TopDUContextPointer(const_cast<TopDUContext*>(topContext()))
        );
        helper.setFunctionNameForADL(QualifiedIdentifier("operator" + op));
        helper.setOperator( OverloadResolver::Parameter(leftType, isLValue( leftType, leftInstance ), leftInstance.declaration.data() ) );
        helper.setKnownParameters( OverloadResolver::ParameterList( OverloadResolver::Parameter(rightType, isLValue( rightType, rightInstance ), rightInstance.declaration.data() ) ) );
        ViableFunction viable = helper.resolve();

        if( viable.isValid() )
        {
          KDevelop::FunctionType::Ptr function = viable.declaration()->type<KDevelop::FunctionType>();
          if( viable.isViable() && function ) {
            success = true;
            m_lastType = function->returnType();
            m_lastInstance = Instance(viable.declaration());

            lock.unlock();
            newUse( node, node->op, node->op+1, viable.declaration() );

            if(m_mapAst) session()->mapCallAstToType(node, function);
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


    if( m_lastType )
      expressionType( node, m_lastType, m_lastInstance );
  }

  /**
   *
   * Not ready yet */

  void ExpressionVisitor::visitTypeSpecifier(TypeSpecifierAST* ast)
  {
    clearLast();

    TypeASTVisitor comp(m_session, this, m_currentContext, topContext(), m_currentContext);
    comp.run(ast);

    LOCKDUCHAIN;

    QList<DeclarationPointer> decls = comp.declarations();

    m_lastType = comp.type();

    if( !decls.isEmpty() )
    {
      m_lastDeclarations = decls;
//       m_lastType = decls.first()->abstractType(); If we do this, we may lose modifiers and such

      if( decls.first()->kind() == Declaration::Type )
        m_lastInstance = Instance(false);
      else
        ///Allow non-types, because we sometimes don't know whether something is a type or not, and it may get parsed as a type.
        m_lastInstance = Instance(decls.first());

      if( m_lastType.cast<CppTemplateParameterType>() )
        createDelayedType(ast, false);
    } else {
      problem(ast, "Could not resolve type");
#ifdef DEBUG_RESOLUTION_PROBLEMS
      //Run the ast-visitor in debug mode

      ++m_ignore_uses;
      TypeASTVisitor comp2(m_session, this, m_currentContext, topContext(), true);
      comp2.run(ast);
      --m_ignore_uses;
#endif
    }
  }

  void ExpressionVisitor::visitSimpleTypeSpecifier(SimpleTypeSpecifierAST* node)
  {
    clearLast();

    TypeASTVisitor tvisitor(m_session, this, m_currentContext, topContext(), m_currentContext);
    tvisitor.run(node);
    m_lastType = tvisitor.type();
    m_lastDeclarations = tvisitor.declarations();
    m_lastInstance = Instance(false);
  }

  void ExpressionVisitor::visitInitDeclarator(InitDeclaratorAST* node)
  {
    if(node->declarator)
    {
      // apply pointer ops to lvalue type
      visitNodes(this, node->declarator->ptr_ops);
      CppClassType::Ptr constructedType;
      if (!m_lastType || !isPointerType(m_lastType)) {
        // Do not blindly dereference, esp. for 'foo* f = new foo;' expressions.
        // Note how computeConstructedType only takes the declaration in
        // m_lastDeclarations into account.
        // Thus, if the lvalue is a pointer we definitely should not get a ClassType.
        constructedType = computeConstructedType();
      }

      //Build constructor uses (similar to visitFunctionCall)

      AbstractType::Ptr oldLastType = m_lastType;
      Instance oldInstance = m_lastInstance;
      QList< DeclarationPointer > declarations = m_lastDeclarations;

      clearLast();

      bool fail = true;

      size_t token = node->start_token;
      //NOTE: we might have an initializer in a class for pure virtual methods,
      //      but we just ignore that. See also TestDUChain::testForwardDeclaration4
      if(node->initializer && m_currentContext->type() != DUContext::Class)
      {
        if(node->initializer->expression && !node->initializer->initializer_clause)
        {
          token = node->initializer->start_token;
          fail = !buildParametersFromExpression(node->initializer->expression);
        } else if(!node->initializer->expression && node->initializer->initializer_clause && constructedType)
        { // report operator= use in i.e.: foo = bar;
          token = node->initializer->start_token;
          fail = !buildParametersFromExpression(node->initializer->initializer_clause);
          declarations.clear();
          LOCKDUCHAIN;
          if ( ClassDeclaration* cdec = dynamic_cast<ClassDeclaration*>(constructedType->declaration(m_source)) ) {
            // constructors are handled automatically in the overload resultion
            declarations << DeclarationPointer(cdec);
            ///TODO: global operator= functions, for now only class members are handled
            static const Identifier opEq("operator=");
            foreach(Declaration* dec, cdec->internalContext()->findDeclarations(opEq)) {
              declarations << DeclarationPointer(dec);
            }
          }
        } else if (!node->initializer->expression && !node->initializer->initializer_clause) {
          // ctor without parameters, i.e.: foo();
          token = node->initializer->start_token;
          fail = false;
        }
      }

      if(fail || !constructedType) {
        DefaultVisitor::visitInitDeclarator(node);
        return;
      }

      DeclarationPointer chosenFunction;
      {
        LOCKDUCHAIN;

        OverloadResolver resolver(
          DUContextPointer(const_cast<DUContext*>(m_currentContext)),
          TopDUContextPointer(const_cast<TopDUContext*>(topContext())),
          OverloadResolver::NonConst,
          oldInstance
        );

        if( !fail )
          chosenFunction = resolver.resolveList(m_parameters, convert(declarations));
        else if(!declarations.isEmpty() && !m_strict)
          chosenFunction = declarations.first();
      }

      if(chosenFunction) {
        newUse( node , token, token+1, chosenFunction );
        if(m_mapAst) session()->mapCallAstToType(node, chosenFunction->type<FunctionType>());
      }
    }else{
      DefaultVisitor::visitInitDeclarator(node);
    }
  }

  void ExpressionVisitor::visitInitializerClause(InitializerClauseAST* node)
  {
    DefaultVisitor::visitInitializerClause(node);
    if( m_lastType ) {
      m_parameters << OverloadResolver::Parameter( m_lastType, isLValue( m_lastType, m_lastInstance ), m_lastInstance.declaration.data() );
      m_parameterNodes.append(node);
    }
  }

  //Used to parse pointer-depth and cv-qualifies of types in new-expessions and casts
  void ExpressionVisitor::visitDeclarator(DeclaratorAST* node)  {
#if 0
    if( !m_lastType ) {
      problem(node, "Declarator used without type");
      return;
    }

    if( m_lastInstance ) {
      problem(node, "Declarator used on an instance instead of a type");
      return;
    }
    #endif

    AbstractType::Ptr oldLastType = m_lastType;
    Instance oldLastInstance = m_lastInstance;

    visit(node->sub_declarator);
//     visit(node->id);
    visit(node->bit_expression);
    visitNodes(this, node->array_dimensions);

    visit(node->parameter_declaration_clause);
    visit(node->exception_spec);
    {
    LOCKDUCHAIN;
    if( node->array_dimensions && oldLastType ) {
      ArrayType::Ptr p( new ArrayType() );
      p->setElementType( oldLastType );

      m_lastType = p.cast<AbstractType>();
      m_lastInstance = Instance(false);
    }else{
      m_lastType = oldLastType;
      m_lastInstance = oldLastInstance;
    }
    }
    visitNodes(this, node->ptr_ops);
  }

  void ExpressionVisitor::visitNewDeclarator(NewDeclaratorAST* node)  {
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

    visit(node->ptr_op);
  }

  void ExpressionVisitor::visitCppCastExpression(CppCastExpressionAST* node)  {
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

  void ExpressionVisitor::visitTypeIDOperator(TypeIDOperatorAST* node) {
    clearLast();
    // report uses
    visit( node->expression );
    visit( node->typeId );
    clearLast();

    m_lastInstance = Instance(true);

    {
      DUChainReadLocker lock;
      foreach(Declaration* dec, m_currentContext->findDeclarations(QualifiedIdentifier("::std::type_info"))) {
        if (dec->abstractType().cast<StructureType>()) {
          m_lastType = dec->abstractType();
          break;
        }
      }
      if (!m_lastType) {
        problem(node, "Could not find std::type_info, must #include <typeinfo> before using typeid");
        return;
      }
    }

    if( m_lastType )
      expressionType( node, m_lastType, m_lastInstance );

    visitSubExpressions( node, node->sub_expressions );
  }

  //Used to parse pointer-depth and cv-qualifies of types in new-expessions and casts
  void ExpressionVisitor::visitPtrOperator(PtrOperatorAST* node) {
    if( !m_lastType )
      problem(node, "Pointer-operator used without type");

    if( m_lastInstance )
      problem(node, "Pointer-operator used on an instance instead of a type");


    ///pointer-to-member
    if(node->op==0){
      PtrToMemberType::Ptr p( new PtrToMemberType() );
      p->setBaseType( m_lastType );
      p->setModifiers(TypeBuilder::parseConstVolatile(m_session, node->cv));
      visit( node->mem_ptr->class_type );
      p->setClassType( m_lastType );
      m_lastType = p.cast<AbstractType>();
    } else {
      int op = m_session->token_stream->kind(node->op);

      if(op == '*') {
        PointerType::Ptr p( new PointerType() );
        p->setBaseType( m_lastType );
        p->setModifiers(TypeBuilder::parseConstVolatile(m_session, node->cv));

        m_lastType = p.cast<AbstractType>();
      } else {
        ReferenceType::Ptr p( new ReferenceType() );
        p->setBaseType( m_lastType );
        p->setModifiers(TypeBuilder::parseConstVolatile(m_session, node->cv));
        if (op == Token_and)
          p->setIsRValue(true);

        m_lastType = p.cast<AbstractType>();
      }
    }
    m_lastInstance = Instance(false);
  }

  /**
   *
   * Has test */
  void ExpressionVisitor::visitCastExpression(CastExpressionAST* node)  {
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
    clearLast();
    visit( node->expression );
    clearLast();

    CppClassType::Ptr constructedType;


    //Visit declarator and type-specifier, which should build the type
    if( node->type_id ) {
      visit(node->type_id->type_specifier);
      constructedType = computeConstructedType();
      visit(node->type_id->declarator);
    } else if( node->new_type_id ) {
      visit(node->new_type_id->type_specifier);
      constructedType = computeConstructedType();
      visit(node->new_type_id->new_declarator);
    }

    if( m_lastType )
    {
      LOCKDUCHAIN;
      ///@todo cv-qualifiers
      PointerType::Ptr p( new PointerType() );
      p->setBaseType( m_lastType );

      m_lastType = p.cast<AbstractType>();

      m_lastInstance = Instance(true);

      if( m_lastType )
        expressionType( node, m_lastType, m_lastInstance );
    }else{
      problem(node, "Could not resolve type");
    }

    AbstractType::Ptr lastType = m_lastType;
    Instance instance = m_lastInstance;

    if(node->new_initializer) {

      //Build constructor uses (similar to visitFunctionCall)
      //Largely a copy of visitInitDeclarator()

      AbstractType::Ptr oldLastType = m_lastType;
      Instance oldInstance = m_lastInstance;
      QList< DeclarationPointer > declarations = m_lastDeclarations;

      clearLast();

      bool fail = !buildParametersFromExpression(node->new_initializer->expression);

      size_t token = node->new_initializer->start_token;

      DeclarationPointer chosenFunction;
      {
        LOCKDUCHAIN;

        OverloadResolver resolver(
          DUContextPointer(const_cast<DUContext*>(m_currentContext)),
          TopDUContextPointer(const_cast<TopDUContext*>(topContext())),
          OverloadResolver::NonConst,
          oldInstance
        );

        if( !fail )
          chosenFunction = resolver.resolveList(m_parameters, convert(declarations));
        else if(!declarations.isEmpty() && !m_strict)
          chosenFunction = declarations.first();
      }

      if(chosenFunction) {
        newUse( node , token, token+1, chosenFunction );
        if(m_mapAst) session()->mapCallAstToType(node, chosenFunction->type<FunctionType>());
      }
    }

    m_lastType = lastType;
    m_lastInstance = instance;
  }

  /**
   *
   * have test */
  void ExpressionVisitor::visitConditionalExpression(ConditionalExpressionAST* node)
  {
    //Also visit the not interesting parts, so they are evaluated
    clearLast();

    visit(node->condition);


    if( m_lastType.cast<DelayedType>() ) {
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
      if( const ConstantIntegralType::Ptr& condition = conditionType.cast<ConstantIntegralType>() ) {
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
    visitIndependentNodes(node->statements);
  }

  /**
   * have test */

  void ExpressionVisitor::visitExpressionOrDeclarationStatement(ExpressionOrDeclarationStatementAST* node)  {
    //visit(node->declaration);
    visit(node->expression);

    if( m_lastType )
      expressionType( node, m_lastType, m_lastInstance );
  }

  bool ExpressionVisitor::dereferenceLastPointer()
  {
    if( PointerType::Ptr pt = realLastType().cast<PointerType>() )
    {
      //Dereference
      m_lastType = pt->baseType();
      m_lastInstance.isInstance = true;
      return true;
    }else if( ArrayType::Ptr pt = realLastType().cast<ArrayType>() ) {
      m_lastType = pt->elementType();
      m_lastInstance.isInstance = true;
      return true;
    }else{
      return false;
    }
  }

  /**
   * partially have test */
  void ExpressionVisitor::visitUnaryExpression(UnaryExpressionAST* node)
  {
    clearLast();

    visit(node->expression);

    if( !m_lastInstance || !m_lastType ) {
      clearLast();
      problem(node, "Tried to evaluate unary expression on a non-instance item" );
      return;
    }

    if( m_lastType.cast<DelayedType>() ) {
      //Store the expression so it's evaluated later
      m_lastInstance = Instance(true);
      createDelayedType(node);
      return;
    }

    switch( tokenFromIndex(node->op).kind ) {
    case '*':
    {
      LOCKDUCHAIN;
      if( dereferenceLastPointer() ) {
      } else {
        //Get return-value of operator*
        findMember(node, m_lastType, Identifier("operator*") );
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
      m_lastType = increasePointerDepth(m_lastType);
      //m_lastInstance will be left alone as it was before. A pointer is not identified, and has no declaration.
    }
    break;
    default:
    {
      const IntegralType::Ptr& integral = m_lastType.cast<IntegralType>();
      if( integral ) {
        //The type of integral types does not change on unary operators
        //Eventually evaluate the value of constant integral types
        const ConstantIntegralType::Ptr& constantIntegral = integral.cast<ConstantIntegralType>();

        if( constantIntegral ) {

          switch( constantIntegral->dataType() ) {
            case IntegralType::TypeFloat:
            {
              ConstantUnaryExpressionEvaluator<float> evaluator( tokenFromIndex(node->op).kind, constantIntegral );
              m_lastType = evaluator.createType();
              break;
            }
            case IntegralType::TypeDouble:
            {
              ConstantUnaryExpressionEvaluator<double> evaluator( tokenFromIndex(node->op).kind, constantIntegral );
              m_lastType = evaluator.createType();
              break;
            }
            default:
              if( constantIntegral->modifiers() & AbstractType::UnsignedModifier ) {
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
          OverloadResolutionHelper helper(
            DUContextPointer(const_cast<DUContext*>(m_currentContext)),
            TopDUContextPointer(const_cast<TopDUContext*>(topContext()))
          );
          helper.setFunctionNameForADL( QualifiedIdentifier("operator" + op) );
          helper.setOperator( OverloadResolver::Parameter(m_lastType, isLValue( m_lastType, m_lastInstance ), m_lastInstance.declaration.data() ) );

          //helper.setKnownParameters( OverloadResolver::Parameter(rightType, isLValue( rightType, rightInstance ), rightInstance.declaration.data() ) );
          ViableFunction viable = helper.resolve();

          if( viable.isValid() )
          {
            KDevelop::FunctionType::Ptr function = viable.declaration()->type<KDevelop::FunctionType>();
            if( viable.isViable() && function ) {
              m_lastType = function->returnType();
              m_lastInstance = Instance(true);

              lock.unlock();

              newUse( node, node->op, node->op+1, viable.declaration() );
              if(m_mapAst) session()->mapCallAstToType(node, function);
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

    const FunctionType::Ptr& f = m_lastType.cast<FunctionType>();
    if( !f ) {
      LOCKDUCHAIN;
      problem(node, QString("cannot get return-type of type %1, it is not a function-type").arg(m_lastType->toString()));
      m_lastType = 0;
      m_lastInstance = Instance();
      return;
    }

    m_lastType = f->returnType();
    //Just keep the function instance, set in findMember(..)
  }


  CppClassType::Ptr ExpressionVisitor::computeConstructedType()
  {
    CppClassType::Ptr constructedType;

    if(!m_lastInstance) {
      LOCKDUCHAIN;
      if(m_lastDeclarations.isEmpty() && m_lastType && !m_lastInstance) {
        IdentifiedType* idType = dynamic_cast<IdentifiedType*>(m_lastType.unsafeData());
        if(idType) {
          Declaration* decl = idType->declaration(m_source);
          if(decl)
            m_lastDeclarations << DeclarationPointer(decl);
        }
      }

      if( !m_lastDeclarations.isEmpty() && m_lastDeclarations.first().data() && m_lastDeclarations.first()->kind() == Declaration::Type && (constructedType = unAliasedType(m_lastDeclarations.first()->logicalDeclaration(topContext())->abstractType()).cast<CppClassType>()) ) {

        if( constructedType && constructedType->declaration(topContext()) && constructedType->declaration(topContext())->internalContext() )
        {
          Declaration* constructedDecl = constructedType->declaration(topContext());

          //Replace a type with its constructros if there is constructors available, so overload-resolution can happen
          m_lastDeclarations = convert(constructedDecl->internalContext()->findLocalDeclarations( constructedDecl->identifier(), constructedDecl->internalContext()->range().end, topContext(), AbstractType::Ptr(), DUContext::OnlyFunctions ));
        }
      }
    }

    return constructedType;
  }

  bool ExpressionVisitor::buildParametersFromDeclaration(ParameterDeclarationClauseAST* node, bool store)
  {
    if(store) {
      m_parameters.clear();
      m_parameterNodes.clear();
    }

    if(node->parameter_declarations)
    {
      const ListNode<ParameterDeclarationAST*>
        *it = node->parameter_declarations->toFront(),
        *end = it;

      do
        {
          //Just to make sure we build the uses. This problem only appears if we mis-parse a declarator as a parameter declaration
          if(it->element->declarator && it->element->declarator->array_dimensions)
          {
            const ListNode< ExpressionAST* >* itt = it->element->declarator->array_dimensions->toFront(), *end2 = itt;
            do{
              visit(it->element->declarator->array_dimensions->element);
            }while(itt != end2);
          }

          visit(it->element->type_specifier);

          if(it->element->declarator) {
            ///@todo Eventually build constructor uses for mis-parsed sub-declarators or parameter-declaration-clauses
            if(it->element->declarator->sub_declarator && it->element->declarator->sub_declarator->id)
            {
              //Special handling is required: Things that really are initializers are treated as declarators in a mis-parsed ParameterDeclarationClause
              visitName(it->element->declarator->sub_declarator->id);
            }else if(it->element->declarator->parameter_declaration_clause)
            {
              buildParametersFromDeclaration(it->element->declarator->parameter_declaration_clause, false);
            }
          }
          visit(it->element->expression);
          if(store) {
            m_parameters.append( OverloadResolver::Parameter( m_lastType, isLValue( m_lastType, m_lastInstance ), m_lastInstance.declaration.data() ) );
            m_parameterNodes.append(it->element);
          }
          it = it->next;
        }
      while (it != end);
    }

    bool fail = false;

    if(store) {
      //Check if all parameters could be evaluated
      int paramNum = 1;
      for( QList<OverloadResolver::Parameter>::const_iterator it = m_parameters.constBegin(); it != m_parameters.constEnd(); ++it ) {
        if( !(*it).type ) {
          problem( node, QString("parameter %1 could not be evaluated").arg(paramNum) );
          fail = true;
          paramNum++;
        }
      }
    }

    return !fail;
  }

  bool ExpressionVisitor::buildParametersFromExpression(AST* expression)
  {
    /**
     * Evaluate the function-argument types. Those are represented a little strangely:
     * expression contains them, using recursive binary expressions
     * */

    m_parameters.clear();
    m_parameterNodes.clear();

    if(!expression)
      return true;

    visit(expression);

    //Check if all parameters could be evaluated
    int paramNum = 1;
    bool fail = false;
    for( QList<OverloadResolver::Parameter>::const_iterator it = m_parameters.constBegin(); it != m_parameters.constEnd(); ++it ) {
      if( !(*it).type ) {
        problem( expression, QString("parameter %1 could not be evaluated").arg(paramNum) );
        fail = true;
        paramNum++;
      }
    }

    return !fail;
  }

  void ExpressionVisitor::handleFunctionCallOrInit(AST* node, ExpressionAST* arguments) {
    PushValue<bool> handler(m_handlingFunctionCallOrInit, true);
    /**
     * If a class name was found, get its constructors.
     * */
    AbstractType::Ptr oldLastType = m_lastType;

    CppClassType::Ptr constructedType;

    if(!m_lastInstance) {
      constructedType = computeConstructedType();
    }else{
      if(m_lastDeclarations.isEmpty() && m_lastType) {
        LOCKDUCHAIN;
        //The function-call operator may also be applied on instances that don't have an explicit declaration, like return-values
        AbstractType::Ptr type = realType(m_lastType);
        IdentifiedType* identified = dynamic_cast<IdentifiedType*>(type.unsafeData());
        if(identified) {
          DeclarationPointer decl(identified->declaration(m_source));
          if(decl) {
            m_lastDeclarations << decl;
          }
        }
      }
    }

    QList<DeclarationPointer> declarations = m_lastDeclarations;
    Instance oldInstance = m_lastInstance;

    QList<OverloadResolver::Parameter> oldParams = m_parameters;
    KDevVarLengthArray<AST*> oldParameterNodes = m_parameterNodes;

    //Backup the current use and invalidate it, we will update and create it after overload-resolution
    CurrentUse oldCurrentUse = m_currentUse;
    m_currentUse.isValid = false;

    clearLast();

    bool fail = !buildParametersFromExpression(arguments);

    LOCKDUCHAIN;

    DeclarationPointer chosenFunction;
    OverloadResolutionHelper helper(
      DUContextPointer(const_cast<DUContext*>(m_currentContext)),
      TopDUContextPointer(const_cast<TopDUContext*>(topContext()))
    );
    helper.setConstness(TypeUtils::isConstant(oldLastType) ? OverloadResolver::Const : OverloadResolver::NonConst);

    MissingDeclarationType::Ptr missing;
    
    Declaration* dec = declarations.isEmpty() ? 0 : declarations.first().data();
    if( !dec ) {
      missing = oldLastType.cast<Cpp::MissingDeclarationType>();
      if (missing) {
        // Eventually use ADL lookup to find the missing declaration
        if (!fail && !missing->containerContext.isValid()) {
          helper.setFunctionNameForADL(missing->identifier().identifier().identifier());
        }
        missing->arguments = m_parameters;
        missing->isFunction = true;
      }
    }else{
      // Eventually use ADL
      if(!m_hadMemberAccess)
        helper.setFunctionNameForADL(QualifiedIdentifier(dec->identifier()));
    }
    
    ViableFunction viable;

    //Resolve functions normally
    if( !fail && !chosenFunction )
    {
      helper.setFunctions(convert(declarations));
      helper.setKnownParameters(m_parameters);
      viable = helper.resolve( !(bool)constructedType );
      if(viable.isValid())
        chosenFunction = viable.declaration();
    }
    
    if( !chosenFunction && constructedType )
    {
      //Default-constructor is used
      m_lastType = constructedType.cast<AbstractType>();
      DeclarationPointer decl(constructedType->declaration(topContext()));
      m_lastInstance = Instance(decl.data());
      m_lastDeclarations.clear();
      lock.unlock();
      if(oldCurrentUse.isValid) {
        newUse( oldCurrentUse.node, oldCurrentUse.start_token, oldCurrentUse.end_token, decl );
      }
      flushUse();
      m_parameterNodes = oldParameterNodes;
      m_parameters = oldParams;
      return;
    }
    
    if( !chosenFunction && !m_strict ) {
      //Because we do not want to rely too much on our understanding of the code, we take the first function instead of totally failing.
#ifdef DEBUG_FUNCTION_CALLS
      QString params;
      foreach(const OverloadResolver::Parameter& param, m_parameters)
        params += param.toString() + ", ";

      QString candidates;
      foreach(const DeclarationPointer &decl, declarations) {
        if( !decl.data() )
          continue;
        int defaultParamCount = 0;
        if( AbstractFunctionDeclaration* aDec = dynamic_cast<AbstractFunctionDeclaration*>(decl.data()) )
          defaultParamCount = aDec->defaultParametersSize();

        candidates += decl->toString() + QString(" default-params: %1").arg(defaultParamCount) + '\n';
      }

      problem(node, QString("Could not find a function that matches the parameters. Using first candidate function. Parameters: %1 Candidates: %2").arg(params).arg(candidates));
#endif
      fail = true;
    }

    if( fail && !declarations.isEmpty() && !chosenFunction ) {
      //Since not all parameters could be evaluated, Choose the first function
      chosenFunction = declarations.front();
    }
    
    if(missing && viable.isValid())
    {
      // Remove the MissingDeclarationProblem which has been created alongside MissingDeclarationType
      for(int idx = m_problems.size()-1; idx >= 0; --idx)
      {
        const ProblemPointer& prob = m_problems[idx];
        const MissingDeclarationProblem* pMissing = dynamic_cast<const MissingDeclarationProblem*>(prob.constData());
        if (pMissing && pMissing->type == missing )
        {
          m_problems.removeAt(idx);
          break;
        }
      }
    }
    
    clearLast();

    if( constructedType ) {
      //Constructor was called
      m_lastType = constructedType.cast<AbstractType>();
      m_lastInstance = Instance(constructedType->declaration(topContext()));
    } else if (chosenFunction) {
      KDevelop::FunctionType::Ptr functionType = chosenFunction->abstractType().cast<KDevelop::FunctionType>();
      if( !functionType ) {
        problem( node, QString( "could not find a matching function for function-call" ) );
      } else {
        m_lastType = functionType->returnType();
        m_lastInstance = Instance(chosenFunction);
      }
    } else {
      problem( node, QString( "could not find a matching function for function-call" ) );
      if(missing)
        m_lastType = missing.cast<AbstractType>(); // Forward the MissingType, as it will be used for assistants.
    }
    
    if(chosenFunction)
      if(m_mapAst) session()->mapCallAstToType(node, chosenFunction->type<FunctionType>());

    static IndexedString functionCallOperatorIdentifier("operator()");

    bool createUseOnParen = (bool)chosenFunction && (constructedType || chosenFunction->identifier().identifier() == functionCallOperatorIdentifier);
    //Re-create the use we have discarded earlier, this time with the correct overloaded function chosen.
    lock.unlock();

    if(createUseOnParen) {
      if(oldCurrentUse.isValid)
        newUse( oldCurrentUse.node, oldCurrentUse.start_token, oldCurrentUse.end_token, oldCurrentUse.declaration );

      newUse( node , node->start_token, node->start_token+1, chosenFunction );
    }else if(oldCurrentUse.isValid) {
      newUse( oldCurrentUse.node, oldCurrentUse.start_token, oldCurrentUse.end_token, chosenFunction );
    }

    m_parameterNodes = oldParameterNodes;
    m_parameters = oldParams;

    flushUse();

    if( m_lastType )
      expressionType( node, m_lastType, m_lastInstance );
  }

  void ExpressionVisitor::visitFunctionCall(FunctionCallAST* node)
  {
    handleFunctionCallOrInit(node, node->arguments);
  }

  void ExpressionVisitor::visitBracedInitList(BracedInitListAST* node)
  {
    if (m_handlingFunctionCallOrInit) {
      DefaultVisitor::visitBracedInitList(node);
      return;
    }
    handleFunctionCallOrInit(node, node);
  }

  void ExpressionVisitor::visitSignalSlotExpression(SignalSlotExpressionAST* node) {

    //So uses for the argument-types are built
    LOCKDUCHAIN;

    putStringType();

    if(m_parameters.isEmpty())
      return;

    DUContext* container = 0;///@todo check whether signal/slot match, warn if not.

    StructureType::Ptr slotStructure = TypeUtils::targetType(TypeUtils::matchingClassPointer(qObjectPtrType(), TypeUtils::realType(m_parameters.back().type, m_topContext), m_topContext), m_topContext).cast<StructureType>();
    if(slotStructure)
      container = slotStructure->internalContext(m_topContext);

    if(!container) {
      Declaration* klass = Cpp::localClassFromCodeContext(m_currentContext);
      if(klass)
        container = klass->internalContext();
    }

    if(!container) {
      lock.unlock();
      problem(node, QString("No signal/slot container"));
      return;
    }

    if(!node->name) {
      problem(node, QString("Bad signal/slot"));
      return;
    }

    {
      CursorInRevision position = container->range().end;
      lock.unlock();
      //This builds the uses
      NameASTVisitor nameV( m_session, this, container, topContext(), m_currentContext, position );
      nameV.run(node->name, true);
      lock.lock();
    }


    CppEditorIntegrator editor(session());
    QByteArray tokenByteArray = editor.tokensToByteArray(node->name->id, node->name->end_token);

    QByteArray sig;
    if(node->name->end_token-1 >= node->name->id+2) {
      sig = QMetaObject::normalizedSignature( editor.tokensToByteArray(node->name->id+1, node->name->end_token) );
      sig = sig.mid(1, sig.length()-2);
    }

    Identifier id(m_session->token_stream->symbol(node->name->id));

    if(!id.isEmpty()) {
      foreach(Declaration* decl, container->findDeclarations(id, CursorInRevision::invalid(), m_topContext, (DUContext::SearchFlags)(DUContext::DontSearchInParent | DUContext::NoFiltering))) {
        QtFunctionDeclaration* qtFunction = dynamic_cast<QtFunctionDeclaration*>(decl);
        if(qtFunction) {
            ///Allow incomplete matching between the specified signature and the real signature, as Qt allows it.
            ///@todo: For signals, we should only allow it when at least as many arguments are specified as in the slot declaration.
            ///@todo: For slots, we should only allow it if the parameter has a default argument.
            int functionSigLength = qtFunction->normalizedSignature().length();
            const char* functionSig = qtFunction->normalizedSignature().c_str();

            if(functionSigLength >= sig.length() &&
               strncmp(functionSig, sig.data(), sig.length()) == 0 &&
               (sig.isEmpty() || functionSigLength == sig.length() || functionSig[sig.length()] == ' ' || functionSig[sig.length()] == ','))
            {
              //Match
              lock.unlock();
              newUse( node, node->name->id, node->name->id+1, DeclarationPointer(qtFunction) );
              return;
            }
        }
      }
    }
  }

  void ExpressionVisitor::visitSubscriptExpression(SubscriptExpressionAST* node)
  {
    ///@todo create use
    Instance masterInstance = m_lastInstance;
    AbstractType::Ptr masterType = m_lastType;

    if( !masterType || !masterInstance ) {
      problem(node, "Tried subscript-expression on invalid object");
      return;
    }

    {
      LOCKDUCHAIN;

      //If the type the subscript-operator is applied on is a pointer, dereference it
      if( dereferenceLastPointer() ) {
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

    OverloadResolutionHelper helper(
      DUContextPointer(const_cast<DUContext*>(m_currentContext)),
      TopDUContextPointer(const_cast<TopDUContext*>(topContext()))
    );
    helper.setConstness(isConstant(masterType) ? OverloadResolver::Const : OverloadResolver::NonConst);
    helper.setFunctionNameForADL( QualifiedIdentifier("operator[]") );
    helper.setOperator( OverloadResolver::Parameter(masterType, isLValue( masterType, masterInstance ), masterInstance.declaration.data() ) );

    helper.setKnownParameters( OverloadResolver::Parameter( m_lastType, isLValue( m_lastType, m_lastInstance ), m_lastInstance.declaration.data() ) );
    ViableFunction viable = helper.resolve();

    if( viable.isValid() )
    {
      KDevelop::FunctionType::Ptr function = viable.declaration()->type<KDevelop::FunctionType>();

      if( function ) {
        m_lastType = function->returnType();
        m_lastInstance = Instance(true);
        if(m_mapAst) session()->mapCallAstToType(node, function);
      }else{
        clearLast();
        problem(node, QString("Found no subscript-function"));
      }

      if( !viable.isViable() ) {
        problem(node, QString("Found no viable subscript-function, chosen function: %1").arg(viable.declaration() ? viable.declaration()->toString() : QString()));
      }

    }else{
      clearLast();
      //Do not complain here, because we do not check for builtin operators
      //problem(node, "No fitting operator. found" );
    }
  }

  void ExpressionVisitor::visitSizeofExpression(SizeofExpressionAST* node)  {
    visit(node->type_id);
    visit(node->expression);
    LOCKDUCHAIN;
    m_lastType = AbstractType::Ptr( new KDevelop::IntegralType(IntegralType::TypeInt) );
    m_lastInstance = Instance(true);
  }

  void ExpressionVisitor::visitCondition(ConditionAST* /*node*/)
  {
    LOCKDUCHAIN;
    m_lastType = AbstractType::Ptr( new KDevelop::IntegralType(IntegralType::TypeBoolean) );
    m_lastInstance = Instance(true);
  }

  void ExpressionVisitor::visitTypeId(TypeIdAST* type_id)  {
    visit(type_id->type_specifier);
    visit(type_id->declarator);
  }

  AbstractType::Ptr ExpressionVisitor::qObjectPtrType() const {
    CppClassType::Ptr p(new CppClassType());
    p->setDeclarationId( DeclarationId(QualifiedIdentifier("QObject")) );
    PointerType::Ptr pointer(new PointerType);
    pointer->setBaseType(p.cast<AbstractType>());
    return pointer.cast<AbstractType>();
  }

  void ExpressionVisitor::putStringType() {
    IntegralType::Ptr i(new KDevelop::IntegralType(IntegralType::TypeChar));
    i->setModifiers(AbstractType::ConstModifier);

    PointerType::Ptr p( new PointerType() );

    p->setBaseType( i.cast<AbstractType>() );

    m_lastType = p.cast<AbstractType>();
    m_lastInstance = Instance(true);
  }


  void ExpressionVisitor::visitStringLiteral(StringLiteralAST* /*node*/) {
    LOCKDUCHAIN;
    ///TODO: proper support for wchar_t, char16_t and char32_t strings
    putStringType();
  }

  void ExpressionVisitor::visitIncrDecrExpression(IncrDecrExpressionAST* node)  {
    ///post-fix increment/decrement like "i++" or "i--"
    ///This does neither change the evaluated value, nor the type(except for overloaded operators)

    if( m_lastType.cast<IntegralType>() ) {
      ///Leave the type and its value alone
    } else {
      ///It is not an integral type, try finding an overloaded operator and use the return-value
      QString op = operatorNameFromTokenKind(tokenFromIndex(node->op).kind);
      if( !op.isEmpty() )
      {
        LOCKDUCHAIN;
        OverloadResolutionHelper helper(
          DUContextPointer(const_cast<DUContext*>(m_currentContext)),
          TopDUContextPointer(const_cast<TopDUContext*>(topContext()))
        );
        helper.setFunctionNameForADL( QualifiedIdentifier("operator" + op) );
        helper.setOperator( OverloadResolver::Parameter(m_lastType, isLValue( m_lastType, m_lastInstance ), m_lastInstance.declaration.data() ) );

        //Overloaded postfix operators have one additional int parameter
        static AbstractType::Ptr integer = AbstractType::Ptr(new ConstantIntegralType(IntegralType::TypeInt));
        helper.setKnownParameters( OverloadResolver::Parameter( integer, false ) );

        ViableFunction viable = helper.resolve();

        if( viable.isValid() )
        {
          KDevelop::FunctionType::Ptr function = viable.declaration()->type<KDevelop::FunctionType>();
          if( viable.isViable() && function ) {
            m_lastType = function->returnType();
            m_lastInstance = Instance(true);
            if(m_mapAst) session()->mapCallAstToType(node, function);
          }else{
            problem(node, QString("Found no viable function"));
          }

          lock.unlock();
          newUse( node, node->op, node->op+1, viable.declaration() );
        }else{
          //Do not complain here, because we do not check for builtin operators
          //problem(node, "No fitting operator. found" );
        }
      }
    }

    if( m_lastType )
      expressionType( node, m_lastType, m_lastInstance );
  }

#if 0
  void ExpressionVisitor::visitNewTypeId(NewTypeIdAST* node)  {
    //Return a pointer to the type
    problem(node);
  }
  #endif

  void ExpressionVisitor::visitSimpleDeclaration(SimpleDeclarationAST* node)  {
    ///Simple type-specifiers like "int" are parsed as SimpleDeclarationAST, so treat them here.
    ///Also we use this to parse constructor uses
    visit(node->type_specifier);
    QList< DeclarationPointer > oldLastDecls = m_lastDeclarations;
    AbstractType::Ptr oldLastType = m_lastType;
    Instance oldLastInstance = m_lastInstance;

    if(node->init_declarators)
    {
      const ListNode<InitDeclaratorAST*>
        *it = node->init_declarators->toFront(),
        *end = it;

      do
        {
          //Make sure each init-declarator gets the same type assigned
          m_lastDeclarations = oldLastDecls;
          m_lastType = oldLastType;
          m_lastInstance = oldLastInstance;

          visit(it->element);
          it = it->next;
        }
      while (it != end);
    }

    visit(node->win_decl_specifiers);
  }

  void ExpressionVisitor::visitDeclarationStatement(DeclarationStatementAST* node)  {
    ///Simple type-specifiers like "int" are parsed as SimpleDeclarationAST, so treat them here.
    visit( node->declaration );
  }

  void ExpressionVisitor::visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST* node)  {
    //Happens as template-parameter
    visit(node->name);
    ///@todo respect const etc.
  }

  void ExpressionVisitor::visitMemInitializer(MemInitializerAST* node)  {
    {
      LOCKDUCHAIN;
      Declaration* klass = localClassFromCodeContext(m_currentContext);
      if(klass)
        m_lastType = klass->abstractType();
    }

    m_memberAccess = true;
    visit(node->initializer_id);
    m_memberAccess = false;

    AbstractType::Ptr itemType = m_lastType;
    Instance oldLastInstance = m_lastInstance;
    QList< DeclarationPointer > declarations = m_lastDeclarations;
    if (buildParametersFromExpression(node->expression)) {
      // build use for the ctor of the base class, see also visitInitDeclarator
      DeclarationPointer chosenFunction;
      {
        LOCKDUCHAIN;

        OverloadResolver resolver(
          DUContextPointer(const_cast<DUContext*>(m_currentContext)),
          TopDUContextPointer(const_cast<TopDUContext*>(topContext())),
          OverloadResolver::NonConst,
          oldLastInstance
        );
        chosenFunction = resolver.resolveList(m_parameters, convert(declarations));
      }

      if (chosenFunction) {
        uint token = node->initializer_id->end_token;
        newUse( node , token, token+1, chosenFunction );

        if(m_mapAst) session()->mapCallAstToType(node, chosenFunction->type<FunctionType>());
      }
    }

    visit(node->expression);
    TypePtr< MissingDeclarationType > missingDeclType = itemType.cast<MissingDeclarationType>();

    if(m_lastType && missingDeclType) {
        Cpp::ExpressionEvaluationResult res;
        res.type = m_lastType->indexed();
        res.isInstance = m_lastInstance;
        missingDeclType->assigned = res;
    }
  }

  void ExpressionVisitor::visitLambdaExpression(LambdaExpressionAST *node)
  {
    DefaultVisitor::visitLambdaExpression(node);
    FunctionType* type = new FunctionType;
    if (node->declarator && node->declarator->parameter_declaration_clause) {
      if (buildParametersFromDeclaration(node->declarator->parameter_declaration_clause)) {
        foreach(const OverloadResolver::Parameter& param, m_parameters) {
          type->addArgument(param.type);
        }
      }
    }
    if (node->declarator && node->declarator->trailing_return_type) {
      visit(node->declarator->trailing_return_type);
      type->setReturnType(m_lastType);
    }
    if (!type->returnType()) {
      ///TODO: if body consists of only a single return statement, use that type as return type
      type->setReturnType(AbstractType::Ptr(new IntegralType(IntegralType::TypeVoid)));
    }
    m_lastType = AbstractType::Ptr( type );
    m_lastInstance = Instance(true);
  }

  void ExpressionVisitor::visit(AST* node)
  {
    if (!node) {
      return;
    }
    PushPositiveValue<const DUContext*> pushContext(m_currentContext, node->ducontext);
    Visitor::visit(node);
  }
}

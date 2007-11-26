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

#include "codecompletioncontext.h"
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <ducontext.h>
#include <duchain.h>
#include <classfunctiondeclaration.h>
#include <duchainlock.h>
#include "stringhelpers.h"
#include "cppduchain/cppduchain.h"
#include "cppduchain/typeutils.h"
#include "cppduchain/overloadresolution.h"
#include "cppduchain/viablefunctions.h"
#include "cppduchain/environmentmanager.h"
#include "cpptypes.h"
#include "safetycounter.h"
#include "templatedeclaration.h"
#include "cpplanguagesupport.h"
#include "parser/rpp/pp-engine.h"
#include "parser/rpp/preprocessor.h"
#include "parser/rpp/pp-environment.h"
#include "parser/rpp/pp-macro.h"
#include <iproblem.h>

#define LOCKDUCHAIN     DUChainReadLocker lock(DUChain::lock())

//#define DEBUG

#ifdef DEBUG
#define ifDebug(x) x
#else
#define ifDebug(x)
#endif

using namespace Cpp;
using namespace KDevelop;

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

typedef PushValue<int> IntPusher;

/**
 * Preprocess the given string using the macros from given EnvironmentFile up to the given line
 * If line is -1, all macros are respected.
 * This is a quite slow operation, because thousands of macros need to be shuffled around.
 * 
 * @todo maybe implement a version of rpp::Environment that directly works on EnvironmentFile,
 * without needing to copy all macros.
 * */
QString preprocess( const QString& text, const Cpp::EnvironmentFilePointer& file, int line ) {

  rpp::Preprocessor preprocessor;
  rpp::pp pp(&preprocessor);

  {
    LOCKDUCHAIN;
/*    kDebug(9007) << "defined macros: " << file->definedMacros().size();*/
    //Copy in all macros from the file
    for( MacroSet::Macros::const_iterator it = file->definedMacros().macros().begin(); it != file->definedMacros().macros().end(); ++it ) {
      if( line == -1 || line > (*it).sourceLine || !(file->url().equals( KUrl((*it).file) ) ) ) {
        pp.environment()->setMacro( new rpp::pp_macro( *it ) );
/*        kDebug(9007) << "adding macro " << (*it).name.str();*/
      } else {
/*        kDebug(9007) << "leaving macro " << (*it).name.str();*/
      }
    }
/*    kDebug(9007) << "used macros: " << file->usedMacros().size();*/
    for( MacroSet::Macros::const_iterator it = file->usedMacros().macros().begin(); it != file->usedMacros().macros().end(); ++it ) {
      if( line == -1 || line > (*it).sourceLine || !(file->url().equals( KUrl((*it).file) ) ) ) {
        pp.environment()->setMacro( new rpp::pp_macro( *it ) );
/*        kDebug(9007) << "adding macro " << (*it).name.str();*/
      } else {
/*        kDebug(9007) << "leaving macro " << (*it).name.str();*/
      }
    }
  }

  QString ret = pp.processFile("anonymous", rpp::pp::Data, text);
  pp.environment()->cleanup();
  
  return ret;
}

///Extracts the last line from the given string
QString extractLastLine(const QString& str) {
  int prevLineEnd = str.lastIndexOf('\n');
  if(prevLineEnd != -1)
    return str.mid(prevLineEnd+1);
  else
    return str;
}

bool CodeCompletionContext::isValid() const {
  return m_valid;
}

int CodeCompletionContext::depth() const {
  return m_depth;
}

int completionRecursionDepth = 0;

CodeCompletionContext::CodeCompletionContext(DUContextPointer context, const QString& text, int depth, const QStringList& knownArgumentExpressions, int line ) : m_memberAccessOperation(NoMemberAccess), m_valid(true), m_text(text), m_depth(depth),  m_knownArgumentExpressions(knownArgumentExpressions), m_duContext(context), m_contextType(Normal), m_parentContext(0)
{
  IntPusher( completionRecursionDepth, completionRecursionDepth+1 );

  if( depth > 10 ) {
    log( "CodeCompletionContext::CodeCompletionContext: too much recursion" );
    m_valid = false;
    return;
  }
  
  if( completionRecursionDepth > 10 ) {
    log( "CodeCompletionContext::CodeCompletionContext: too much recursion" );
    m_valid = false;
    return;
  }
  //log( "Computing context for " + text );

  {
    //Since include-file completion has nothing in common with the other completion-types, process it within a separate function
    QString lineText = extractLastLine(m_text).trimmed();
    if(lineText.startsWith("#include")) {
      processIncludeDirective(lineText);
      return;
    }
  }
  
  m_valid = isValidPosition();
  if( !m_valid ) {
    log( "position not valid for code-completion" );
    return;
  }

  ifDebug( log( "non-processed text: " + m_text ); )
   preprocessText( line );

   m_text = Utils::clearComments( m_text );
   m_text = Utils::clearStrings( m_text );
   m_text = Utils::stripFinalWhitespace( m_text );

  ifDebug( log( "processed text: " + m_text ); )

  ///@todo template-parameters

  ///First: find out what kind of completion we are dealing with

  if( m_text.endsWith( ';' ) || m_text.endsWith('}') || m_text.endsWith('{') ) {
    ///We're at the beginning of a new statement. General completion is valid.
    return;
  }

    if( m_text.endsWith('.') ) {
    m_memberAccessOperation = MemberAccess;
    m_text = m_text.left( m_text.length()-1 );
  }

  if( m_text.endsWith("->") ) {
    m_memberAccessOperation = ArrowMemberAccess;
    m_text = m_text.left( m_text.length()-2 );
  }

  if( m_text.endsWith("::") ) {
    m_memberAccessOperation = StaticMemberChoose; //We need to decide later whether it is a MemberChoose
    m_text = m_text.left( m_text.length()-2 );
  }

  if( endsWithOperator( m_text ) ) {
    if( depth == 0 ) {
      //The first context should never be a function-call context, so make this a NoMemberAccess context and the parent a function-call context.
      m_parentContext = new CodeCompletionContext( m_duContext, m_text, depth+1 );
      return;
    }
    m_memberAccessOperation = FunctionCallAccess;
    m_contextType = BinaryOperatorFunctionCall;
    m_operator = getEndOperatorFunction(m_text);
    m_text = m_text.left( m_text.length() - getEndOperator(m_text).length() );
  }

  if( m_text.endsWith('(') ) {
    if( depth == 0 ) {
      //The first context should never be a function-call context, so make this a NoMemberAccess context and the parent a function-call context.
      m_parentContext = new CodeCompletionContext( m_duContext, m_text, depth+1 );
      return;
    }
    m_contextType = FunctionCall;
    m_memberAccessOperation = FunctionCallAccess;
    m_text = m_text.left( m_text.length()-1 );

    ///Compute the types of the argument-expressions
    ExpressionParser expressionParser;

    for( QStringList::const_iterator it = m_knownArgumentExpressions.begin(); it != m_knownArgumentExpressions.end(); ++it )
      m_knownArgumentTypes << expressionParser.evaluateExpression( (*it).toUtf8(), m_duContext );
  }

  ///Now find out where the expression starts

  /**
   * Possible cases:
   * a = exp; - partially handled
   * ...
   * return exp;
   * emit exp;
   * throw exp;
   * new Class;
   * a=function(exp
   * a = exp(
   * ClassType instance
   *
   * What else?
   *
   * When the left and right part are only separated by a whitespace,
   * expressionAt returns both sides
   * */

  int start_expr = Utils::expressionAt( m_text, m_text.length() );

  m_expression = m_text.mid(start_expr).trimmed();
  
  QString expressionPrefix = Utils::stripFinalWhitespace( m_text.left(start_expr) );

  ifDebug( log( "expressionPrefix: " + expressionPrefix ); )
    
  ///Handle recursive contexts(Example: "ret = function1(param1, function2(" )
  if( expressionPrefix.endsWith('(') || expressionPrefix.endsWith(',') ) {
    log( QString("Recursive function-call: Searching parent-context in \"%1\"").arg(expressionPrefix) );
    //Our expression is within a function-call. We need to find out the possible argument-types we need to match, and show an argument-hint.

    //Find out which argument-number this expression is, and compute the beginning of the parent function-call(parentContextLast)
    QStringList otherArguments;
    int parentContextEnd = expressionPrefix.length();
    
    Utils::skipFunctionArguments( expressionPrefix, otherArguments, parentContextEnd );

    QString parentContextText = expressionPrefix.left(parentContextEnd);

    log( QString("This argument-number: %1 Building parent-context from \"%2\"").arg(otherArguments.size()).arg(parentContextText) );
    m_parentContext = new CodeCompletionContext( m_duContext, parentContextText, depth+1, otherArguments );
  }

  ///Handle overridden binary operator-functions
  if( endsWithOperator(expressionPrefix) ) {
    log( QString( "Recursive operator: creating parent-context with \"%1\"" ).arg(expressionPrefix) );
    m_parentContext = new CodeCompletionContext( m_duContext, expressionPrefix, depth+1 );
  }

  ///Now care about m_expression. It may still contain keywords like "new "

  bool isEmit = false, isReturn = false, isThrow = false;

  QString expr = m_expression.trimmed();

  if( expr.startsWith("emit") )  {
    isEmit = true; //When isEmit is true, we should filter the result so only signals are left
    expr = expr.right( expr.length() - 4 );
  }
  if( expr.startsWith("return") )  {
    isReturn = true; //When isReturn is true, we should match the result against the return-type of the current context-function
    expr = expr.right( expr.length() - 6 );
  }
  if( expr.startsWith("throw") )  {
    isThrow = true;
    expr = expr.right( expr.length() - 5 );
  }

  ExpressionParser expressionParser/*(false, true)*/;

  ifDebug( kDebug(9007) << "expression: " << expr; )
  
  if( !expr.trimmed().isEmpty() ) {
    m_expressionResult = expressionParser.evaluateExpression( expr.toUtf8(), m_duContext );
    ifDebug( kDebug(9007) << "expression result: " << m_expressionResult.toString(); )
    if( !m_expressionResult.isValid() ) {
      if( m_memberAccessOperation != StaticMemberChoose ) {
        log( QString("expression \"%1\" could not be evaluated").arg(expr) );
        m_valid = false;
        return;
      } else {
        //It may be an access to a namespace, like "MyNamespace::".
        //The "MyNamespace" can not be evaluated, still we can give some completions.
        return;
      }
    }
  }

  switch( m_memberAccessOperation ) {

    case NoMemberAccess:
    {
      if( !expr.trimmed().isEmpty() ) {
        //This should never happen, because the position-cursor should be chosen at the beginning of a possible completion-context(not in the middle of a string)
        log( QString("Cannot complete \"%1\" because there is an expression without an access-operation" ).arg(expr) );
        m_valid  = false;
      } else {
        //Do nothing. We do not have a completion-container, which means that a global completion should be done.
      }
    }
    break;
    case MemberChoose:
    case StaticMemberChoose:
    {
      ///@todo Check whether it is a MemberChoose
    }
    case ArrowMemberAccess:
    {
      LOCKDUCHAIN;
      //Dereference a pointer
      AbstractType::Ptr containerType = m_expressionResult.type;
      CppPointerType* pnt = dynamic_cast<CppPointerType*>(TypeUtils::realType(containerType));
      if( !pnt ) {
        IdentifiedType* idType = dynamic_cast<IdentifiedType*>(TypeUtils::realType(containerType));
        if( idType ) {
          if( idType->declaration() && idType->declaration()->internalContext() ) {
            QList<Declaration*> operatorDeclarations = idType->declaration()->internalContext()->findLocalDeclarations(QualifiedIdentifier("operator->"));
            if( !operatorDeclarations.isEmpty() ) {
              ///@todo care about const
              m_expressionResult.allDeclarations = operatorDeclarations;
              CppFunctionType* function = dynamic_cast<CppFunctionType*>( operatorDeclarations.front()->abstractType().data() );

              if( function ) {
                m_expressionResult.type = function->returnType();
                m_expressionResult.instance = ExpressionVisitor::Instance(true);
              } else {
                  log( QString("arrow-operator of class is not a function: %1").arg(containerType ? containerType->toString() : QString("null") ) );
              }
            } else {
              log( QString("arrow-operator on type without operator* member: %1").arg(containerType ? containerType->toString() : QString("null") ) );
            }
          } else {
            log( QString("arrow-operator on type without declaration and context: %1").arg(containerType ? containerType->toString() : QString("null") ) );
          }
        } else {
          log( QString("arrow-operator on invalid type: %1").arg(containerType ? containerType->toString() : QString("null") ) );
          m_expressionResult = ExpressionEvaluationResult();
        }
      }

      if( pnt ) {
        ///@todo what about const in pointer?
        m_expressionResult.type = pnt->baseType();
        m_expressionResult.instance = ExpressionVisitor::Instance(true);
      }
    }
    case MemberAccess:
    {
      if( expr.trimmed().isEmpty() ) {
        log( "Expression was empty, cannot complete" );
        m_valid = false;
      }

      //The result of the expression is stored in m_expressionResult, so we're fine
    }
    break;
    case FunctionCallAccess:
      processFunctionCallAccess();
    break;
  }
}

void CodeCompletionContext::processFunctionCallAccess() {
  ///Generate a list of all found functions/operators, together with each a list of optional prefixed parameters

  ///All the variable argument-count management in the following code is done to treat global operator-functions equivalently to local ones. Those take an additional first argument.

  LOCKDUCHAIN;
  
  OverloadResolutionHelper helper(m_duContext);
  
  if( m_contextType == BinaryOperatorFunctionCall ) {

    if( !m_expressionResult.instance ) {
      log( "tried to apply an operator to a non-instance: " + m_expressionResult.toString() );
      m_valid = false;
      return;
    }

    helper.setOperator(OverloadResolver::Parameter(m_expressionResult.type.data(), m_expressionResult.isLValue()), m_operator);
  } else {
    ///Simply take all the declarations that were found by the expression-parser
    
    helper.setFunctions(m_expressionResult.allDeclarations);
  }

  OverloadResolver::ParameterList knownParameters;
  foreach( ExpressionEvaluationResult result, m_knownArgumentTypes )
    knownParameters.parameters << OverloadResolver::Parameter( result.type.data(), result.isLValue() );
  
  helper.setKnownParameters(knownParameters);
  
  m_functions = helper.resolve(true);

//   if( declarations.isEmpty() ) {
//     log( QString("no list of function-declarations was computed for expression \"%1\"").arg(m_expression) );
//     return;
//   }
}

void CodeCompletionContext::processIncludeDirective(QString line)
{
  if(line.count('"') == 2 || line.endsWith('>'))
    return; //We are behind a complete include-directive

  kDebug(9007) << "include line: " << line;
  line = line.mid(8).trimmed(); //Strip away the #include
  kDebug(9007) << "trimmed include line: " << line;

  if(!line.startsWith('<') && !line.startsWith('"'))
    return; //We are not behind the beginning of a path-specification

  bool local = false;
  if(line.startsWith('"'))
    local = true;
  
  line = line.mid(1);

  kDebug(9007) << "extract prefix from " << line;
  //Extract the prefix-path
  KUrl u(line);
  u.setFileName(QString());
  QString prefixPath = u.path();
  kDebug(9007) << "extracted prefix " << prefixPath;
  
  LOCKDUCHAIN;
  if(!m_duContext)
    return;
#ifndef TEST_COMPLETION
  m_includeItems = CppLanguageSupport::self()->allFilesInIncludePath(m_duContext->url(), local, prefixPath);
#endif
  m_valid = true;
  m_memberAccessOperation = IncludeListAccess;
}

const CodeCompletionContext::FunctionList& CodeCompletionContext::functions() const {
  return m_functions;
}

QList<Cpp::IncludeItem> CodeCompletionContext::includeItems() const {
  return m_includeItems;
}

ExpressionEvaluationResult CodeCompletionContext::memberAccessContainer() const {
  return m_expressionResult;
}

QList<DUContext*> CodeCompletionContext::memberAccessContainers() const {
  QList<DUContext*> ret;
  
  if( memberAccessOperation() == StaticMemberChoose && m_duContext ) {
    //Locate all namespace-instances we will be completing from
    ret += m_duContext->findContexts(DUContext::Class, QualifiedIdentifier(m_expression)); 
    ret += m_duContext->findContexts(DUContext::Namespace, QualifiedIdentifier(m_expression)); ///@todo respect position
  }

  if(m_expressionResult.isValid() ) {
    const IdentifiedType* idType = dynamic_cast<const IdentifiedType*>( TypeUtils::targetType(m_expressionResult.type.data()) );
    if( idType && idType->declaration() ) {
      DUContext* ctx = TypeUtils::getInternalContext( idType->declaration() );
      if( ctx )
        ret << ctx;
      else {
        //Print some debug-output
        kDebug(9007) << "Could not get internal context from" << m_expressionResult.type->toString();
        kDebug(9007) << "Declaration" << idType->declaration()->toString() << idType->declaration()->isForwardDeclaration();
        if( Cpp::TemplateDeclaration* tempDeclaration = dynamic_cast<Cpp::TemplateDeclaration*>(idType->declaration()) ) {
          if( tempDeclaration->instantiatedFrom() ) {
            kDebug(9007) << "instantiated from" << dynamic_cast<Declaration*>(tempDeclaration->instantiatedFrom())->toString() << dynamic_cast<Declaration*>(tempDeclaration->instantiatedFrom())->isForwardDeclaration();
            kDebug(9007) << "internal context" << dynamic_cast<Declaration*>(tempDeclaration->instantiatedFrom())->internalContext();
          }
        }
        
      }
    }
  }
  
  return ret;
}


CodeCompletionContext::~CodeCompletionContext() {
}

void CodeCompletionContext::log( const QString& str ) const {
  kDebug(9007) << "CodeCompletionContext:" << str;
}

bool CodeCompletionContext::isValidPosition() {
  if( m_text.isEmpty() )
    return true;
  //If we are in a string or comment, we should not complete anything
  QString markedText = Utils::clearComments(m_text, '$');
  markedText = Utils::clearStrings(markedText,'$');

  if( markedText[markedText.length()-1] == '$' ) {
    //We are within a comment or string
    kDebug(9007) << "code-completion position is invalid, marked text: \n\"" << markedText << "\"\n unmarked text:\n" << m_text << "\n";
    return false;
  }
  return true;
}

QString originalOperator( const QString& str ) {
  if( str == "[" )
    return "[]";
  return str;
}

QString CodeCompletionContext::getEndOperator( const QString& str ) const {
  static QStringList allowedOperators = QString("++ + -- += -= *= /= %= ^= &= |= << >> >>= <<= == != <= >= && || [ - * / % & | = < >" ).split( ' ', QString::SkipEmptyParts );

  for( QStringList::const_iterator it = allowedOperators.begin(); it != allowedOperators.end(); ++it )
    if( str.endsWith(*it) )
      return *it;
  return QString();
}

QString CodeCompletionContext::getEndOperatorFunction( const QString& str ) const {
  return originalOperator( getEndOperator( str ) );
}

bool CodeCompletionContext::endsWithOperator( const QString& str ) const {
  return !getEndOperator(str).isEmpty();
}

QList<KDevelop::AbstractType::Ptr> CodeCompletionContext::additionalMatchTypes() const {
  QList<KDevelop::AbstractType::Ptr> ret;
  if( m_operator == "=" && m_expressionResult.isValid() && m_expressionResult.instance ) {
    //Conversion to the left operand-type
    ret << m_expressionResult.type;
  }
  return ret;
}

void CodeCompletionContext::preprocessText( int line ) {
  
  LOCKDUCHAIN;
  
  if( m_duContext ) {
  m_text = preprocess( m_text, Cpp::EnvironmentFilePointer( dynamic_cast<Cpp::EnvironmentFile*>(m_duContext->topContext()->parsingEnvironmentFile().data()) ), line );
  }else{
    kWarning() << "error: no ducontext";
  }
}

CodeCompletionContext::MemberAccessOperation CodeCompletionContext::memberAccessOperation() const {
  return m_memberAccessOperation;
}

CodeCompletionContext* CodeCompletionContext::parentContext() {
  return m_parentContext.data();
}

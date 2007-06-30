/*
   Copyright (C) 2007 David Nolden <user@host.de>
   (where user = david.nolden.kdevelop, host = art-master)

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

#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <ducontext.h>
#include <duchain.h>
#include "codecompletioncontext.h"
#include "stringhelpers.h"
#include "duchainbuilder/cppduchain.h"
#include "expressionparser/overloadresolution.h"
#include "expressionparser/typeutils.h"
#include "expressionparser/viablefunctions.h"
#include "cpptypes.h"
#include "safetycounter.h"
#include <classfunctiondeclaration.h>
#include <duchainlock.h>

#define LOCKDUCHAIN     DUChainReadLocker lock(DUChain::lock())

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

CodeCompletionContext::Function::Function() : matchedArguments(0) {
}

CodeCompletionContext::Function::Function( int _matchedArguments, const ViableFunction& _viable ) : matchedArguments(_matchedArguments), function(_viable) {
}

CodeCompletionContext::operator bool() const {
  return m_valid;
}


CodeCompletionContext::CodeCompletionContext(DUContext * context, const KTextEditor::Cursor& position, KTextEditor::View* view, bool firstContext, const QStringList& knownArgumentExpressions ) : m_memberAccessOperation(NoMemberAccess), m_valid(true), m_knownArgumentExpressions(knownArgumentExpressions), m_duContext(context), m_position(position), m_view(view), m_document(view->document()), m_contextType(Normal), m_parentContext(0)
{
  static int depth = 0;

  IntPusher( depth, depth+1 );
  
  if( depth > 10 ) {
    log( "CodeCompletionContext::CodeCompletionContext: too much recursion" );
    m_valid = false;
    return;
  }
  log( "Computing context" );
  //Step 1: Find the statement we're in
  if(!m_document) {
    log( "no text-document" );
    return;
  }

  KTextEditor::Range range;
  {
    LOCKDUCHAIN;
    range = KTextEditor::Range(context->textRange().start(), position);
    
    m_text = m_document->text(range);
    m_textStartPosition = context->textRange().start();
  }

  //Now text is the range that ends with the statement we are analyzing
  if( m_text.isEmpty() ) {
    log("no text for context");
    return;
  }

  m_valid = isValidPosition();
  if( !m_valid ) {
    log( "position not valid for code-completion" );
    return;
  }

  log( "non-processed text: " + m_text );
  preprocessText();
  
  m_text = Utils::clearComments( m_text );
  m_text = Utils::clearStrings( m_text );
  m_text = Utils::stripFinalWhitespace( m_text );

  //text = Utils::reduceWhiteSpace(text);
  
  log( "processed text: " + m_text );

  ///@todo template-parameters
  
  ///First: find out what kind of completion we are dealing with

  if( m_text.endsWith( ";" ) || m_text.endsWith("}") || m_text.endsWith("{") ) {
    ///We're at the beginning of a new statement. General completion is valid.
    return;
  }
  
  if( m_text.endsWith(".") ) {
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
    if( firstContext ) {
      //The first context should never be a function-call context, so make this a NoMemberAccess context and the parent a function-call context.
      m_parentContext = new CodeCompletionContext( m_duContext, position, m_view, false );
      return;
    }
    m_memberAccessOperation = FunctionCallAccess;
    m_contextType = BinaryOperatorFunctionCall;
    m_operator = getEndOperator(m_text);
    m_text = m_text.left( m_text.length() - m_operator.length() );
  }
  
  if( m_text.endsWith("(") ) {
    if( firstContext ) {
      //The first context should never be a function-call context, so make this a NoMemberAccess context and the parent a function-call context.
      m_parentContext = new CodeCompletionContext( m_duContext, position, m_view, false );
      return;
    }
    m_contextType = FunctionCall;
    m_memberAccessOperation = FunctionCallAccess;
    m_text = m_text.left( m_text.length()-1 );
    
    ///Compute the types of the argument-expressions
    ExpressionParser expressionParser;

    for( QStringList::const_iterator it = m_knownArgumentExpressions.begin(); it != m_knownArgumentExpressions.end(); ++it )
      m_knownArgumentTypes << expressionParser.evaluateType( (*it).toUtf8(), m_duContext );
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

  int start_expr = Utils::expressionAt( m_text, m_text.length()-1 );

  m_expression = m_text.mid(start_expr).trimmed();

  QString expressionPrefix = m_text.left(start_expr);
  while( expressionPrefix.endsWith(" ") )
    expressionPrefix = expressionPrefix.left( expressionPrefix.size() - 1 );

  ///Handle recursive contexts(Example: "ret = function1(param1, function2(" )
  if( expressionPrefix.endsWith("(") || expressionPrefix.endsWith(",") ) {
    log( QString("Recursive function-call: Searching parent-context in \"%1\"").arg(expressionPrefix) );
    //Our expression is within a function-call. We need to find out the possible argument-types we need to match, and show an argument-hint.
    int parentContextEnd = expressionPrefix.length();
    //Find out which argument-number this expression is

    int parentExpressionStart = parentContextEnd;
    QStringList otherArguments;
    Utils::skipFunctionArguments( expressionPrefix, otherArguments, parentExpressionStart );
        
    log( QString("This argument-number: %1 Building parent-context from \"%2\"").arg(otherArguments.size()).arg(expressionPrefix.left(parentExpressionStart)) );
    m_parentContext = new CodeCompletionContext( m_duContext, mapTextToDocument(parentExpressionStart), m_view, false, otherArguments );
  }

  ///Handle overridden binary operator-functions
  if( endsWithOperator(expressionPrefix) ) {
    log( QString( "Recursive operator: creating parent-context with \"%1\"" ).arg(expressionPrefix) );
    m_parentContext = new CodeCompletionContext( m_duContext, mapTextToDocument(expressionPrefix.length()), m_view, false );
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

  ExpressionParser expressionParser;
  
  if( !expr.trimmed().isEmpty() ) {
    m_expressionResult = expressionParser.evaluateType( expr.toUtf8(), m_duContext );
    if( !(*m_expressionResult) ) {
      log( QString("expression \"%1\" could not be evaluated").arg(expr) );
      m_valid = false;
      return;
    }
  }

  switch( m_memberAccessOperation ) {

    case NoMemberAccess:
    {
      if( !expr.trimmed().isEmpty() ) {
        //This should never happen, because the position-cursor should be chosen at the beginning of a possible completion-context(not in the middle of a string)
        log( "Cannot complete \"%1\" because there is an expression without an access-operation" );
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
    case MemberAccess:
    case ArrowMemberAccess:
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

  ///All the variable argument-count management in the following code is done to treat global operator-functions equivalently to local ones. They take a first parameter.


  typedef QPair<OverloadResolver::ParameterList, Declaration*> DeclarationWithArgument;

  QList< DeclarationWithArgument > declarations; //Declarations are paired with the optional first argument for the declared functions

  if( m_contextType == BinaryOperatorFunctionCall ) {

    if( !m_expressionResult->instance ) {
      log( "tried to apply an operator to a non-instance" );
      m_valid = false;
      return;
    }

    QualifiedIdentifier identifier( "operator"+m_operator );
    LOCKDUCHAIN;
    ///Search for member operators
    AbstractType::Ptr real( TypeUtils::realType(m_expressionResult->type.data()) );
    if( dynamic_cast<CppClassType*>( real.data() ) )
    {
      IdentifiedType* idType = dynamic_cast<IdentifiedType*>( real.data() );
      if( idType ) {
        DUContext* ctx = TypeUtils::getInternalContext( idType->declaration() );
        if( ctx ) {
          QList<Declaration*> decls = Cpp::findLocalDeclarations( ctx, identifier );
          foreach( Declaration* decl, decls )
            declarations << DeclarationWithArgument( OverloadResolver::ParameterList(), decl );
        } else {
          log( "no internal context found" );
        }
      } else {
          log( "type is not identified" );
      }
    }
    ///Search for static global operators
    QList<Declaration*> decls = m_duContext->findDeclarations(identifier);
    foreach( Declaration* decl, decls ) {
      FunctionType* fun = dynamic_cast<FunctionType*>( decl->abstractType().data() );
      if( fun && fun->arguments().size() == 2 )
        declarations << DeclarationWithArgument( OverloadResolver::Parameter(m_expressionResult->type.data(), m_expressionResult->isLValue()), decl );
    }
  } else {
    ///Simply take all the declarations that were found by the expression-parser
    foreach( Declaration* decl, m_expressionResult->allDeclarations )
      declarations << DeclarationWithArgument( OverloadResolver::ParameterList(), decl ); //Insert with argument-offset zero
  }
  if( declarations.isEmpty() ) {
    log( QString("no list of function-declarations was computed for expression \"%1\"").arg(m_expression) );
    return;
  }

  QMap<Declaration*, int> m_argumentCountMap; //Maps how many pre-defined arguments where given to which function
  foreach( const DeclarationWithArgument& decl, declarations )
    m_argumentCountMap[decl.second] = decl.first.parameters.size();

  OverloadResolver resolv( m_duContext );
  OverloadResolver::ParameterList knownParameters;
  foreach( ExpressionEvaluationResult::Ptr result, m_knownArgumentTypes )
    knownParameters.parameters << OverloadResolver::Parameter( result->type.data(), result->isLValue() );

  QList< ViableFunction > viableFunctions = resolv.resolveListPartial( knownParameters, declarations );
  foreach( const ViableFunction& function, viableFunctions ) {
    if( function.declaration() && function.declaration()->abstractType() ) {
      m_functions << Function( m_argumentCountMap[function.declaration()] + knownParameters.parameters.size(), function );
    }
  }
}

const CodeCompletionContext::FunctionList& CodeCompletionContext::functions() const {
  return m_functions;
}

ExpressionEvaluationResult::Ptr CodeCompletionContext::memberAccessContainer() const {
  return m_expressionResult;
}

CodeCompletionContext::~CodeCompletionContext() {
}

void CodeCompletionContext::log( const QString& str ) const {
  kDebug() << "CodeCompletionContext: " << str << endl;
}

bool CodeCompletionContext::isValidPosition() {
  //If we are in a string or comment, we should not complete anything
  QString markedText = Utils::clearComments(m_text, '$');
  markedText = Utils::clearStrings(markedText,'$');

  if( markedText[markedText.length()-1] == '$' ) {
    //We are within a comment or string
    return false;
  }
  return true;
}

KTextEditor::Cursor CodeCompletionContext::mapTextToDocument(int pos) const {
  KTextEditor::Cursor ret = m_position;
  ret.setColumn( ret.column() + pos );
  SafetyCounter s(100);
  while( ret.column() > m_document->lineLength(ret.line()) && s ) {
    ret.setColumn( ret.column() - m_document->lineLength(ret.line()) + 1  );
    ret.setLine( ret.line() + 1 );
  }
  if( !s ) {
    log("mapTextToDocument strange error");
    return m_position;
  }
  
  return ret;
}

QString CodeCompletionContext::getEndOperator( const QString& str ) const {
  static QStringList allowedOperators = QString("++ + -- += -= *= /= %= ^= &= |= << >> >>= <<= == != <= >= && || [ - * / % & | = < >" ).split( " ", QString::SkipEmptyParts );
  
  for( QStringList::const_iterator it = allowedOperators.begin(); it != allowedOperators.end(); ++it )
    if( str.endsWith(*it) )
      return *it;
  return QString::null;
}

bool CodeCompletionContext::endsWithOperator( const QString& str ) const {
  return !getEndOperator(str).isEmpty();
}

QList<KDevelop::AbstractType::Ptr> CodeCompletionContext::additionalMatchTypes() const {
  QList<KDevelop::AbstractType::Ptr> ret;
  if( m_operator == "=" && m_expressionResult && m_expressionResult->type && m_expressionResult->instance ) {
    //Conversion to the left operand-type
    ret << m_expressionResult->type;
  }
  return ret;
}

void CodeCompletionContext::preprocessText() {
  ///@todo implement, preprocess m_text in m_duContext at m_position
  ///All macros can be found in the context's LexedFile, in definedMacros() together with usedMacros()
  ///Important: mapTextToDocument needs to stay functional
}

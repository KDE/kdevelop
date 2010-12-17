/*
   Copyright 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include "context.h"
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <klocalizedstring.h>

#include <interfaces/idocumentcontroller.h>

#include <language/interfaces/ilanguagesupport.h>
#include <language/duchain/ducontext.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/namespacealiasdeclaration.h>
#include <language/duchain/classfunctiondeclaration.h>
#include <language/duchain/functiondefinition.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/stringhelpers.h>
#include <language/duchain/safetycounter.h>
#include <language/interfaces/iproblem.h>
#include <util/pushvalue.h>

#include "../cppduchain/cppduchain.h"
#include "../cppduchain/typeutils.h"
#include "../cppduchain/overloadresolution.h"
#include "../cppduchain/viablefunctions.h"
#include "../cppduchain/environmentmanager.h"
#include "../cppduchain/cpptypes.h"
#include "../stringhelpers.h"
#include "../cppduchain/templatedeclaration.h"
#include "../cpplanguagesupport.h"
#include "../cpputils.h"
#include "../cppduchain/environmentmanager.h"
#include "../cppduchain/cppduchain.h"

#include "cppdebughelper.h"
#include "missingincludeitem.h"
#include "implementationhelperitem.h"
#include <qtfunctiondeclaration.h>
#include "missingincludemodel.h"
#include <templateparameterdeclaration.h>
#include <language/duchain/classdeclaration.h>
#include "qpropertydeclaration.h"
#include "model.h"

// #define ifDebug(x) x

#define LOCKDUCHAIN     DUChainReadLocker lock(DUChain::lock())
#include <cpputils.h>
#include <interfaces/ilanguage.h>

///Created statically as this object must be a child of the main thread
CppUtils::ReplaceCurrentAccess accessReplacer;

QString lastLines(QString str, int count = 40) {
  QStringList lines = str.split("\n");
  if(lines.count() < count)
    return str;
  else
    return QStringList(lines.mid(lines.count()-count, count)).join("\n");
}

///If this is enabled, KDevelop corrects wrong member access operators like "." on a pointer automatically
const bool assistAccessType = true;
///If this is enabled, no chain of useless argument-hints for binary operators is created.
const bool noMultipleBinaryOperators = true;
///Whether only items that are allowed to be accessed should be shown
const bool doAccessFiltering = true;
// #ifdef TEST_COMPLETION
// //Stub implementation that does nothing
// QList<KDevelop::CompletionTreeItemPointer> missingIncludeCompletionItems(QString /*expression*/, QString /*displayTextPrefix*/, Cpp::ExpressionEvaluationResult /*expressionResult*/, KDevelop::DUContext* /*context*/, int /*argumentHintDepth*/, bool /*needInstance*/) {
//   return QList<KDevelop::CompletionTreeItemPointer>();
// }
// #endif

QStringList binaryArithmeticOperators = QString("+ - * / % ^ & | < >" ).split( ' ', QString::SkipEmptyParts );

QStringList binaryModificationOperators = QString("+= -= *= /= %= ^= &= |= =" ).split( ' ', QString::SkipEmptyParts );

QStringList arithmeticComparisonOperators = QString("!= <= >= < >" ).split( ' ', QString::SkipEmptyParts );

QStringList allOperators = QString("++ + -- += -= *= /= %= ^= &= |= << >> >>= <<= == != <= >= && || [ - * / % & | = < >" ).split( ' ', QString::SkipEmptyParts );

//Whether identifiers starting with "__" or "_Uppercase" and that are not declared in the current file should be excluded from the code completion
const bool excludeReservedIdentifiers = true;

using namespace KDevelop;

namespace Cpp {

///@todo move these together with those from expressionvisitor into an own file, or make them unnecessary
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

QList<Declaration*> convert( const QList<DeclarationId>& decls, uint count, TopDUContext* top ) {

  QList<Declaration*> ret;
  for(uint a = 0; a < count; ++a) {
    Declaration* d = decls[a].getDeclaration(top);
    if(d)
      ret << d;
  }

  return ret;
}

typedef PushValue<int> IntPusher;

///Extracts the last line from the given string
QString extractLastLine(const QString& str) {
  int prevLineEnd = str.lastIndexOf('\n');
  if(prevLineEnd != -1)
    return str.mid(prevLineEnd+1);
  else
    return str;
}

bool isPrefixKeyword(QString str) {
  return str == "new" || str == "return" || str == "else" || str == "throw" || str == "delete" || str == "emit" || str == "Q_EMIT" || str == "case";
}

int completionRecursionDepth = 0;

///Removes the given word from before the expression if it is there. Returns whether it was found + removed.
bool removePrefixWord(QString& expression, QString word) {
  if(expression.left(word.size()+1).trimmed() == word) {
    if(expression.size() >= word.size()+1)
      expression = expression.mid(word.size()+1);
    else
      expression.clear();
    return true;
  }
  return false;
}

CodeCompletionContext::CodeCompletionContext(KDevelop::DUContextPointer context, const QString& text, const QString& followingText, const KDevelop::CursorInRevision& position, int depth, const QStringList& knownArgumentExpressions, int line )
  : KDevelop::CodeCompletionContext(context, text, position, depth),
    m_memberAccessOperation(NoMemberAccess),
    m_knownArgumentExpressions(knownArgumentExpressions),
    m_contextType(Normal),
    m_pointerConversionsBeforeMatching(0),
    m_useStoredItems(false),
    m_onlyShow(ShowAll),
    m_isConstructorCompletion(false),
    m_doAccessFiltering(doAccessFiltering)
{
#ifndef TEST_COMPLETION  
  MissingIncludeCompletionModel::self().stop();
#endif
  
  if(m_duContext) {
    LOCKDUCHAIN;
    if(!m_duContext)
      return;

    if((m_duContext->type() == DUContext::Class || m_duContext->type() == DUContext::Namespace || m_duContext->type() == DUContext::Global)) {
      m_onlyShow = ShowTypes;
      ifDebug( kDebug() << "Only showing types"; )
    }

    Declaration* classDecl = Cpp::localClassFromCodeContext(m_duContext.data());
    if(classDecl) {
      ifDebug( kDebug() << "local class:" << classDecl->qualifiedIdentifier().toString(  ); )
      m_localClass = DUContextPointer(classDecl->internalContext());
    }
  }

  m_followingText = followingText;

  if(depth > 10) {
    kDebug() << "too much recursion";
    m_valid = false;
    return;
  }

  {
    //Since include-file completion has nothing in common with the other completion-types, process it within a separate function
    QString lineText = extractLastLine(m_text).trimmed();
    if(lineText.startsWith("#")) {
      processIncludeDirective(lineText);
      return;
    }
  }

  m_valid = isValidPosition();
  if( !m_valid ) {
    kDebug() << "position not valid for code-completion";
    return;
  }

  ifDebug( kDebug() << "non-processed text: " + m_text; )
  if(depth == 0) {
    preprocessText( line );
    m_text = clearComments( m_text );
  }
    
  ifDebug( kDebug() << "preprocessed text: " + m_text; )

   m_text = stripFinalWhitespace( m_text );
   m_text = lastLines(m_text);

  ifDebug( kDebug() << QString("depth %1").arg(depth) + " end of processed text: " + m_text; )

  if(doConstructorCompletion())
    return;

  ///First: find out what kind of completion we are dealing with

  if( (m_text.endsWith( ':' ) && !m_text.endsWith("::")) || m_text.endsWith( ';' ) || m_text.endsWith('}') || m_text.endsWith('{') || m_text.endsWith(')') ) {
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

  if( m_text.endsWith('(') || m_text.endsWith( '<' ) ) {
    if( depth == 0 ) {
      //The first context should never be a function-call context, so make this a NoMemberAccess context and the parent a function-call context.
      m_parentContext = new CodeCompletionContext( m_duContext, m_text, QString(), m_position, 1 );
      m_text.clear();
    }else{
      ExpressionParser expressionParser;
      
      if(m_text.endsWith('(')) {
        m_contextType = FunctionCall;
        m_memberAccessOperation = FunctionCallAccess;
        m_text = m_text.left( m_text.length()-1 );
      }else{
        //We need to check here whether this really is a template access, or whether it is a "smaller" operator,
        //which is handled below
        int start_expr = Utils::expressionAt( m_text, m_text.length()-1 );

        QString expr = m_text.mid(start_expr, m_text.length() - start_expr - 1).trimmed();
        
        Cpp::ExpressionEvaluationResult result = expressionParser.evaluateExpression(expr.toUtf8(), m_duContext);
        if(result.isValid() && (!result.isInstance || result.type.type<FunctionType>()) && !result.type.type<DelayedType>()) {
          m_memberAccessOperation = TemplateAccess;
          m_text = m_text.left( m_text.length()-1 );
        }
      }

      ///Compute the types of the argument-expressions

      for( QStringList::const_iterator it = m_knownArgumentExpressions.constBegin(); it != m_knownArgumentExpressions.constEnd(); ++it )
        m_knownArgumentTypes << expressionParser.evaluateExpression( (*it).toUtf8(), m_duContext );
    }
  }else if( endsWithOperator( m_text ) && (m_memberAccessOperation != StaticMemberChoose || !m_text.trimmed().endsWith(">"))) {
    if( depth == 0 ) {
      //The first context should never be a function-call context, so make this a NoMemberAccess context and the parent a function-call context.
      m_parentContext = new CodeCompletionContext( m_duContext, m_text, QString(), m_position, depth+1 );
      m_text.clear();
    }else{
      m_memberAccessOperation = FunctionCallAccess;
      m_contextType = BinaryOperatorFunctionCall;
      m_operator = getEndFunctionOperator(m_text);
      m_text = m_text.left( m_text.length() - getEndOperator(m_text).length() );
    }
  }

  ///Eventually take preceding "*" and/or "&" operators and use them for pointer depth conversion of the completion items
  while(parentContext() && parentContext()->m_memberAccessOperation == FunctionCallAccess && parentContext()->m_contextType == BinaryOperatorFunctionCall && parentContext()->m_expression.isEmpty()) {
    if(parentContext()->m_operator == "*") {
      --m_pointerConversionsBeforeMatching;
      setParentContext(parentContext()->m_parentContext);
      continue;
    }
    if(parentContext()->m_operator == "&") {
      ++m_pointerConversionsBeforeMatching;
      setParentContext(parentContext()->m_parentContext);
      continue;
    }
    break;
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
   * delete exp;
   * a=function(exp
   * a = exp(
   * ClassType instance(
   * Type instance =
   *
   * What else?
   *
   * When the left and right part are only separated by a whitespace,
   * expressionAt returns both sides
   * */

  int start_expr = Utils::expressionAt( m_text, m_text.length() );

  m_expression = m_text.mid(start_expr).trimmed();
  ifDebug( kDebug() << "expression: " + m_expression; )

  if(m_expression == "else")
    m_expression = QString();
  
  if(m_expression == "const" || m_expression == "typedef") {
    //The cursor is behind a "const .."
    m_onlyShow = ShowTypes;
    m_expression = QString();
  }
  
  if(m_expression == ":" || m_expression == "public" || m_expression == "protected" || m_expression == "private" || m_expression == "virtual") {
    m_onlyShow = ShowTypes;
    m_expression = QString();
  }
  
  if(m_expression == "emit" || m_expression == "Q_EMIT")  {
    m_onlyShow = ShowSignals;
    m_expression = QString();
  }
  
  m_isDeclarationTypePrefix = false;

  QString expressionPrefix = stripFinalWhitespace( m_text.left(start_expr) );

  ifDebug( kDebug() << "expressionPrefix: " + expressionPrefix; )

  ///Handle constructions like "ClassType instance("
  if(!expressionPrefix.isEmpty() && (expressionPrefix.endsWith('>') || expressionPrefix[expressionPrefix.length()-1].isLetterOrNumber() || expressionPrefix[expressionPrefix.length()-1] == '_')) {
    int newExpressionStart = Utils::expressionAt(expressionPrefix, expressionPrefix.length());
    if(newExpressionStart >= 0) {
      QString newExpression = expressionPrefix.mid(newExpressionStart).trimmed();
      QString newExpressionPrefix = stripFinalWhitespace( expressionPrefix.left(newExpressionStart) );
      if(!isPrefixKeyword(newExpression)) {
        if(newExpressionPrefix.isEmpty() || newExpressionPrefix.endsWith(';') || newExpressionPrefix.endsWith('{') || newExpressionPrefix.endsWith('}')) {
          ifDebug( kDebug(9007) << "skipping expression" << m_expression << "and setting new expression" << newExpression; )
          m_expression = newExpression;
          expressionPrefix = newExpressionPrefix;
          m_isDeclarationTypePrefix = true;
        }
      }
    }
  }
  
  ///Handle recursive contexts(Example: "ret = function1(param1, function2(" )
  if( (expressionPrefix.endsWith( '<' ) || expressionPrefix.endsWith('(') || expressionPrefix.endsWith(',') || expressionPrefix.endsWith(':')) 
      && getEndOperator(expressionPrefix).length() < 2 ) { //Do not try a function-call for example with "<<", as that may lead to a quadratic expansion together with the operator expansion
    //Our expression is within a function-call. We need to find out the possible argument-types we need to match, and show an argument-hint.

    //Find out which argument-number this expression is, and compute the beginning of the parent function-call(parentContextLast)
    QStringList otherArguments;
    int parentContextEnd = expressionPrefix.length();

    skipFunctionArguments( expressionPrefix, otherArguments, parentContextEnd );

    QString parentContextText = expressionPrefix.left(parentContextEnd);
    
    //Prevent useless endless recursion
    if(depth == 0 || parentContextText != m_text)
    {
      kDebug() << QString("Recursive function-call: Searching parent-context in \"%1\"").arg(expressionPrefix);
      kDebug() << QString("This argument-number: %1 Building parent-context from \"%2\"").arg(otherArguments.size()).arg(parentContextText);
      m_parentContext = new CodeCompletionContext( m_duContext, parentContextText, QString(), m_position, depth+1, otherArguments );
    }else{
      ifDebug( kDebug() << "not following at depth" << depth << " because:" << parentContextText << "==" << m_text; )
    }
  }

  ///Handle signal/slot access
  if(depth == 0 && m_parentContext && parentContext()->memberAccessOperation() == FunctionCallAccess) {
    LOCKDUCHAIN;
    if(!m_duContext)
      return;
    
    bool needSignal = parentContext()->m_expression == "SIGNAL" || parentContext()->m_expression == "Q_SIGNAL";
    bool needSlot = parentContext()->m_expression == "SLOT" || parentContext()->m_expression == "Q_SLOT";
    if(needSignal || needSlot) {
      m_onlyShow = needSignal ? ShowSignals : ShowSlots;
      
      //Remove the SIGNAL / SLOT function context that was added
      setParentContext(KSharedPtr<KDevelop::CodeCompletionContext>(parentContext()->parentContext()));
    }
    
    if(m_parentContext && parentContext()->memberAccessOperation() == FunctionCallAccess && m_expression.isEmpty()) {
      foreach(const Cpp::OverloadResolutionFunction &function, parentContext()->functions()) {
        if(function.function.declaration() && (function.function.declaration()->qualifiedIdentifier().toString() == "QObject::connect" || function.function.declaration()->qualifiedIdentifier().toString() == "QObject::disconnect")) {
          FunctionType::Ptr funType = function.function.declaration()->type<FunctionType>();
          if(funType && funType->arguments().size() > function.matchedArguments && funType->arguments().size() > 2) {
            if(function.matchedArguments == 1 && parentContext()->m_knownArgumentTypes.size() >= 1) {
              ///Pick a signal from the class pointed to in the earlier element
              m_memberAccessOperation = SignalAccess;
            }else if(funType->arguments()[function.matchedArguments] && funType->arguments()[function.matchedArguments]->toString() == "const char*") {
              m_memberAccessOperation = SlotAccess;

              if(parentContext()->m_knownArgumentExpressions.size() > 1) {
                QString connectedSignal = parentContext()->m_knownArgumentExpressions[1];
                
                int skipSignal = 0;
                if(connectedSignal.startsWith("SIGNAL"))
                  skipSignal = 7;
                if(connectedSignal.startsWith("Q_SIGNAL"))
                  skipSignal = 9;
                
                if(skipSignal && connectedSignal.endsWith(")") && connectedSignal.length() > skipSignal+1) {
                  connectedSignal = connectedSignal.mid(skipSignal);
                  connectedSignal = connectedSignal.left(connectedSignal.length()-1);
                  //Now connectedSignal is something like myFunction(...), and we want the "...".
                  QPair<Identifier, QByteArray> signature = Cpp::qtFunctionSignature(connectedSignal.toUtf8());
                  m_connectedSignalIdentifier = signature.first;
                  m_connectedSignalNormalizedSignature = signature.second;
                }
              }
            }

            if(m_memberAccessOperation == SignalAccess || m_memberAccessOperation == SlotAccess) {
              if(function.matchedArguments == 2) {
                //The function that does not take the target-argument is being used
                if(Declaration* klass = Cpp::localClassFromCodeContext(m_duContext.data()))
                  m_expressionResult.type = klass->indexedType();
              }else if(parentContext()->m_knownArgumentTypes.size() >= function.matchedArguments && function.matchedArguments != 0) {
                m_expressionResult = parentContext()->m_knownArgumentTypes[function.matchedArguments-1];
                m_expressionResult.type = TypeUtils::targetType(TypeUtils::matchingClassPointer(funType->arguments()[function.matchedArguments-1], m_expressionResult.type.abstractType(), m_duContext->topContext()), m_duContext->topContext())->indexed();
              }
              break;
            }
          }
        }
      }
    }
  }
  
  if(depth == 0 && parentContext() && parentContext()->memberAccessOperation() == TemplateAccess) {
    LOCKDUCHAIN;
    //This also happens in cases like "for(int a = 0; a < |", so test whether the previous expression is an instance.
    if(parentContext()->m_expressionResult.isValid() && !parentContext()->m_expressionResult.isInstance)
      m_onlyShow = ShowTypes;
  }

  ///Handle overridden binary operator-functions
  if( endsWithOperator(expressionPrefix) || expressionPrefix.endsWith("return") ) {
    if(depth == 0 || m_text != expressionPrefix) {
      kDebug() << QString( "Recursive operator: creating parent-context with \"%1\"" ).arg(expressionPrefix);
      m_parentContext = new CodeCompletionContext( m_duContext, expressionPrefix, QString(), m_position, depth+1 );
    }else{
      ifDebug( kDebug() << "not following operator at depth" << depth << " because:" << expressionPrefix  << "==" << m_text; )
    }
    
  }

  ///Now care about m_expression. It may still contain keywords like "new "

  bool isThrow = false;

  QString expr = m_expression.trimmed();

  removePrefixWord(expr, "emit");
  removePrefixWord(expr, "Q_EMIT");
  
  if( removePrefixWord(expr, "return") )  {
    if(!expr.isEmpty() || depth == 0) {
      //Create a new context for the "return"
      m_parentContext = new CodeCompletionContext( m_duContext, "return", QString(), m_position, depth+1 );
    }else{
      m_memberAccessOperation = ReturnAccess;
    }
  }
  if( removePrefixWord(expr, "case") )  {
    if(!expr.isEmpty() || depth == 0) {
      //Create a new context for the "case"
      m_parentContext = new CodeCompletionContext( m_duContext, "case", QString(), m_position, depth+1 );
    }
  }
  if( removePrefixWord(expr, "delete") )  {
    QRegExp bracketRE("^\\s*\\[\\s*\\]");
    if (expr.contains(bracketRE))
      expr = expr.remove(bracketRE);
    
    if(!expr.isEmpty() || depth == 0) {
      //Create a new context for the "delete"
      // TODO add brackets if necessary?
      m_parentContext = new CodeCompletionContext( m_duContext, "delete", QString(), m_position, depth+1 );
    }else{
      //m_memberAccessOperation = DeleteAccess;
    }
  }
  if( removePrefixWord(expr, "throw") )  {
    isThrow = true;
  }
  if( removePrefixWord(expr, "new") ) {
    m_onlyShow = ShowTypes;
    m_pointerConversionsBeforeMatching = 1;
  }
  
  ExpressionParser expressionParser/*(false, true)*/;

  ifDebug( kDebug(9007) << "expression: " << expr; )

  if( !expr.trimmed().isEmpty() && !m_expressionResult.isValid() ) {
    LOCKDUCHAIN;
    
    if(!m_isDeclarationTypePrefix && m_memberAccessOperation != NoMemberAccess) {
      m_expressionResult = expressionParser.evaluateExpression( expr.toUtf8(), m_duContext );
    }else{
      m_expressionResult = expressionParser.evaluateType( expr.toUtf8(), m_duContext );
      m_expressionResult.isInstance = true;
      ifDebug( kDebug() << "is expression type prefix"; )
    }
    
    ifDebug( kDebug(9007) << "expression result: " << m_expressionResult.toString(); )
    if( !m_expressionResult.isValid() ) {
      if( m_memberAccessOperation != StaticMemberChoose ) {
        kDebug() << QString("expression \"%1\" could not be evaluated").arg(expr);
        if(m_memberAccessOperation == FunctionCallAccess || m_memberAccessOperation == TemplateAccess)
          m_functionName = m_expression; //Keep the context valid, so missing-include completion can happen
        else
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
      //Either there's no expression, which means Global completion, and nothing need be done.
      //Or there is an expression, which means only implementationhelperitems are wanted
      if (!expr.isEmpty())
      {
        if (isImplementationHelperValid())
          m_onlyShow = ShowImplementationHelpers;
        else
          m_valid = false;
      }
    }
    break;
    case ArrowMemberAccess:
    {
      LOCKDUCHAIN;
      if(!m_duContext)
        return;
      
      //Dereference a pointer
      AbstractType::Ptr containerType = m_expressionResult.type.abstractType();
      PointerType::Ptr pnt = TypeUtils::realType(containerType, m_duContext->topContext()).cast<PointerType>();
      if( !pnt ) {
        AbstractType::Ptr realContainer = TypeUtils::realType(containerType, m_duContext->topContext());
        IdentifiedType* idType = dynamic_cast<IdentifiedType*>(realContainer.unsafeData());
        if( idType ) {
          Declaration* idDecl = idType->declaration(m_duContext->topContext());
          if( idDecl && idDecl->internalContext() ) {
            bool declarationIsConst = false;
            if (containerType->modifiers() & AbstractType::ConstModifier || idDecl->abstractType()->modifiers() & AbstractType::ConstModifier)
              declarationIsConst = true;

            QList<Declaration*> operatorDeclarations = Cpp::findLocalDeclarations(idDecl->internalContext(), Identifier("operator->"), m_duContext->topContext());
            if( !operatorDeclarations.isEmpty() ) {
              // TODO use Cpp::isAccessible on operator functions for more correctness?
              foreach(Declaration* decl, operatorDeclarations)
                m_expressionResult.allDeclarationsList().append(decl->id());
              
              FunctionType::Ptr function;
              foreach (Declaration* decl, operatorDeclarations) {
                FunctionType::Ptr f2 = decl->abstractType().cast<FunctionType>();
                const bool operatorIsConst = f2->modifiers() & AbstractType::ConstModifier;
                if (operatorIsConst == declarationIsConst) {
                  // Best match
                  function = f2;
                  break;
                } else if (operatorIsConst && !function) {
                  // Const result where non-const is ok, accept and keep looking
                  function = f2;
                }
              }

              if( function ) {
                m_expressionResult.type = function->returnType()->indexed();
                m_expressionResult.isInstance = true;
              } else {
                  kDebug() << QString("arrow-operator of class is not a function, or is non-const where the object being accessed is const: %1").arg(containerType ? containerType->toString() : QString("null") );
              }
            } else {
              kDebug() << QString("arrow-operator on type without operator-> member: %1").arg(containerType ? containerType->toString() : QString("null") );
              if(idDecl->internalContext()->type() == DUContext::Class)
                replaceCurrentAccess("->", ".");
            }
          } else {
            kDebug() << QString("arrow-operator on type without declaration and context: %1").arg(containerType ? containerType->toString() : QString("null") );
          }
        } else {
          kDebug() << QString("arrow-operator on invalid type: %1").arg(containerType ? containerType->toString() : QString("null") );
          m_expressionResult = ExpressionEvaluationResult();
        }
      }

      if( pnt ) {
        ///@todo what about const in pointer?
        m_expressionResult.type = pnt->baseType()->indexed();
        m_expressionResult.isInstance = true;
      }
    }
    case MemberChoose:
    case StaticMemberChoose:
    case MemberAccess:
    {
      if( expr.trimmed().isEmpty() ) {
        kDebug() << "Expression was empty, cannot complete";
        m_valid = false;
      }

      //The result of the expression is stored in m_expressionResult, so we're fine

      ///Additional step: Check whether we're accessing a declaration that is not available, and eventually allow automatically adding an #include
      LOCKDUCHAIN;
      if(!m_duContext)
        return;
      
      AbstractType::Ptr type = m_expressionResult.type.abstractType();
      if(type && m_duContext) {
        DelayedType::Ptr delayed = type.cast<DelayedType>();
#ifndef TEST_COMPLETION // hmzzz ?? :)
        if(delayed && delayed->kind() == DelayedType::Unresolved) {
          eventuallyAddGroup(i18n("Not Included"), 1000, missingIncludeCompletionItems(m_expression, m_followingText.trimmed() + ": ", m_expressionResult, m_duContext.data(), 0, true));
        }
#endif
        if(type.cast<PointerType>())
          replaceCurrentAccess(".", "->");
      }else{
        kDebug() << "No type for expression";
      }
    }
    break;
    case FunctionCallAccess:
      processFunctionCallAccess();
    break;
    default:
      //Nothing to do for now
    break;
  }
}

bool CodeCompletionContext::doConstructorCompletion() {
  QString text = m_text.trimmed();
  LOCKDUCHAIN;

  if(!m_duContext)
    return false;

  QStringList hadItems;
  
  text = text.trimmed();
  ifDebug( kDebug() << "text:" << text; )

  //Jump over all initializers
  while(!text.isEmpty() && text.endsWith(',')) {
    text = text.left(text.length()-1).trimmed();
    //Skip initializer expression
    int start_expr = Utils::expressionAt( text, text.length() );
    QString skip = text.mid(start_expr, text.length() - start_expr);
    
    if(skip.contains('('))
      hadItems << skip.left(skip.indexOf('(')).trimmed();
    
    text = text.left(start_expr).trimmed();
  }
  
  if(!text.trimmed().endsWith(':'))
    return false;

  text = text.left(text.length()-1).trimmed();
  //Now we have the declaration in text
  ifDebug( kDebug() << "should be decl.:" << text; )
  if(!text.endsWith(')'))
    return false;
  
  int argumentsStart = text.length()-1;
  QStringList arguments;
  skipFunctionArguments(text, arguments, argumentsStart);
  if(argumentsStart <= 0)
    return false;
  
  int identifierStart = Utils::expressionAt( text, argumentsStart-1 );
  if(identifierStart < 0 || identifierStart == argumentsStart)
    return false;
  
  m_text = QString();
  
  QualifiedIdentifier id(text.mid(identifierStart, argumentsStart-1-identifierStart));
  if(id.isEmpty())
    return false;
  id = id.left(id.count()-1);
  
  DUContext* container = 0;
  
  if(!id.isEmpty()) {
    //Find the class
    QList< KDevelop::Declaration* > decls = m_duContext->findDeclarations(id);
    if(decls.isEmpty()) {
      ifDebug( kDebug() << "did not find class declaration for" << id.toString(); )
      return false;
    }
    container = decls[0]->logicalInternalContext(m_duContext->topContext());
  }else if(m_duContext->parentContext() && m_duContext->parentContext()->type() == DUContext::Class && m_duContext->parentContext()->owner()) {
    container = m_duContext->parentContext();
  }
  
  if(!container)
    return false;
  
  m_onlyShow = ShowVariables;
  m_isConstructorCompletion = true;
  m_memberAccessOperation = MemberAccess;
  m_doAccessFiltering = false;
  m_useStoredItems = true;
  
  QSet<QString> hadItemsSet = hadItems.toSet();
  
  QList<CompletionTreeItemPointer> items;
  
  int pos = 1000;
  bool initializedNormalItems = false;
  
  //Pre-compute the items
  foreach(Declaration* decl, container->localDeclarations(m_duContext->topContext())) {
    ClassMemberDeclaration* classMem = dynamic_cast<ClassMemberDeclaration*>(decl);
    
    if(decl->kind() == Declaration::Instance && !decl->isFunctionDeclaration() && classMem && !classMem->isStatic()) {
      if(!hadItemsSet.contains(decl->identifier().toString())) {
        items << CompletionTreeItemPointer(new NormalDeclarationCompletionItem( DeclarationPointer(decl), KDevelop::CodeCompletionContext::Ptr(this), pos ));
        ++pos;
      }else{
        initializedNormalItems = true;
      }
    }
  }

  if(!initializedNormalItems) {
    //Only offer constructor initializations before variables were initialized
    pos = 0;
    foreach(const DUContext::Import& import, container->importedParentContexts()) {
      DUContext* ctx = import.context(m_duContext->topContext());
      if(ctx && ctx->type() == DUContext::Class && ctx->owner()) {
          items.insert(pos, CompletionTreeItemPointer(new NormalDeclarationCompletionItem( DeclarationPointer(ctx->owner()), KDevelop::CodeCompletionContext::Ptr(this), pos )));
          ++pos;
      }
    }
  }
  
  
  eventuallyAddGroup(i18n("Initialize"), 0, items);
  
  return true;
  ///Step 1: Skip to the ':', to find the back of the function declaration. On the way, all expressions need to be constructor decls.
}

QList< Cpp::ExpressionEvaluationResult > CodeCompletionContext::knownArgumentTypes() const {
  return m_knownArgumentTypes;
}

bool CodeCompletionContext::isConstructorInitialization() {
  return m_isConstructorCompletion;
}

CodeCompletionContext::AdditionalContextType CodeCompletionContext::additionalContextType() const {
  return m_contextType;
}

void CodeCompletionContext::processFunctionCallAccess() {
  ///Generate a list of all found functions/operators, together with each a list of optional prefixed parameters

  ///All the variable argument-count management in the following code is done to treat global operator-functions equivalently to local ones. Those take an additional first argument.

  LOCKDUCHAIN;
  if(!m_duContext)
    return;

  OverloadResolutionHelper helper( m_duContext, TopDUContextPointer(m_duContext->topContext()) );

  if( m_contextType == BinaryOperatorFunctionCall ) {

    if( !m_expressionResult.isInstance ) {
      kDebug() << "tried to apply an operator to a non-instance: " + m_expressionResult.toString();
      m_valid = false;
      return;
    }

    helper.setOperator(OverloadResolver::Parameter(m_expressionResult.type.abstractType(), m_expressionResult.isLValue()));

    m_functionName = "operator"+m_operator;
    
  } else {
    ///Simply take all the declarations that were found by the expression-parser

    helper.setFunctions(convert(m_expressionResult.allDeclarations, m_expressionResult.allDeclarationsSize(), m_duContext->topContext()));

    if(m_expressionResult.allDeclarationsSize()) {
      Declaration* decl = m_expressionResult.allDeclarations[0].getDeclaration(m_duContext->topContext());
      if(decl)
        m_functionName = decl->identifier().toString();
    }
  }
  
  if( m_contextType == BinaryOperatorFunctionCall || m_expression == m_functionName )
    helper.setFunctionNameForADL( QualifiedIdentifier(m_functionName) );
  
  OverloadResolver::ParameterList knownParameters;
  foreach( const ExpressionEvaluationResult &result, m_knownArgumentTypes )
    knownParameters.parameters << OverloadResolver::Parameter( result.type.abstractType(), result.isLValue() );

  helper.setKnownParameters(knownParameters);

  m_functions = helper.resolveToList(true);

  if(m_contextType == BinaryOperatorFunctionCall) {
    //Filter away all global binary operators that do not have the first argument matched
    QList< Function > oldFunctions = m_functions;
    m_functions.clear();
    foreach(const Function& f, oldFunctions) {
      if(f.matchedArguments == 1 && !f.function.isViable())
        continue;
      else
        m_functions << f;
    }
  }
  
//   if( declarations.isEmpty() ) {
//     kDebug() << QString("no list of function-declarations was computed for expression \"%1\"").arg(m_expression);
//     return;
//   }
}

void CodeCompletionContext::processIncludeDirective(QString line)
{
  if(line.count('"') == 2 || line.endsWith('>'))
    return; //We are behind a complete include-directive

  //The include-directive has not been fully opened
//   if(!line.count('"') && !line.count("<"))
//     return; 

  int endOfInclude = CppUtils::findEndOfInclude(line);
  if(endOfInclude == -1)
    return;
  
  //Strip away #include
  line = line.mid(endOfInclude).trimmed();
  
  kDebug(9007) << "include line: " << line;
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

  QString prefixPath;
  if(line.contains('/')) {
    u.setFileName(QString());
    prefixPath = u.toLocalFile();
  }
  kDebug(9007) << "extracted prefix " << prefixPath;

  LOCKDUCHAIN;
  if(!m_duContext)
    return;
#ifndef TEST_COMPLETION
  m_includeItems = CppUtils::allFilesInIncludePath(KUrl(m_duContext->url().str()), local, prefixPath);
#endif
  m_valid = true;
  m_memberAccessOperation = IncludeListAccess;
}

const CodeCompletionContext::FunctionList& CodeCompletionContext::functions() const {
  return m_functions;
}

QString CodeCompletionContext::functionName() const {
  return m_functionName;
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
    QList< Declaration* > decls = m_duContext->findDeclarations(QualifiedIdentifier(m_expression)); ///@todo respect position

    // qlist does not provide convenient stable iterators
    std::list<Declaration*> worklist(decls.begin(), decls.end());
    for (std::list<Declaration*>::iterator it = worklist.begin(); it != worklist.end(); ++it) {
      Declaration * decl = *it;
      if((decl->kind() == Declaration::Namespace || dynamic_cast<ClassDeclaration*>(decl))  && decl->internalContext())
        ret << decl->internalContext();
      else if (decl->kind() == Declaration::NamespaceAlias) {
        NamespaceAliasDeclaration * aliasDecl = dynamic_cast<NamespaceAliasDeclaration*>(decl);
        if (aliasDecl) {
          QList<Declaration*> importedDecls = m_duContext->findDeclarations(aliasDecl->importIdentifier()); ///@todo respect position
          std::copy(importedDecls.begin(), importedDecls.end(),
                    std::back_inserter(worklist));
        }
      }
    }
  }

  if(m_expressionResult.isValid() ) {
    AbstractType::Ptr expressionTarget = TypeUtils::targetType(m_expressionResult.type.abstractType(), m_duContext->topContext());
    const IdentifiedType* idType = dynamic_cast<const IdentifiedType*>( expressionTarget.unsafeData() );
      Declaration* idDecl = 0;
    if( idType && (idDecl = idType->declaration(m_duContext->topContext())) ) {
      DUContext* ctx = idDecl->logicalInternalContext(m_duContext->topContext());
      if( ctx ){
        if(ctx->type() != DUContext::Template) //Forward-declared template classes have a template-context assigned. Those should not be searched.
          ret << ctx;
      }else {
        //Print some debug-output
        kDebug(9007) << "Could not get internal context from" << m_expressionResult.type.abstractType()->toString();
        kDebug(9007) << "Declaration" << idDecl->toString() << idDecl->isForwardDeclaration();
        if( Cpp::TemplateDeclaration* tempDeclaration = dynamic_cast<Cpp::TemplateDeclaration*>(idDecl) ) {
          if( tempDeclaration->instantiatedFrom() ) {
            kDebug(9007) << "instantiated from" << dynamic_cast<Declaration*>(tempDeclaration->instantiatedFrom())->toString() << dynamic_cast<Declaration*>(tempDeclaration->instantiatedFrom())->isForwardDeclaration();
            kDebug(9007) << "internal context" << dynamic_cast<Declaration*>(tempDeclaration->instantiatedFrom())->internalContext();
          }
        }

      }
    }
  }
  
//   foreach(DUContext* context, ret) {
//     kDebug() << "member-access container:" << context->url().str() << context->range().textRange() << context->scopeIdentifier(true).toString();
//   }

  return ret;
}



KDevelop::IndexedType CodeCompletionContext::applyPointerConversionForMatching(KDevelop::IndexedType type, bool fromLValue) const {
  if(!m_duContext)
    return KDevelop::IndexedType();
  
  if(m_pointerConversionsBeforeMatching == 0)
    return type;
  AbstractType::Ptr t = type.abstractType();
  if(!t)
    return KDevelop::IndexedType();

  //Can only take addresses of lvalues
  if(m_pointerConversionsBeforeMatching > 1 || (m_pointerConversionsBeforeMatching && !fromLValue))
    return IndexedType();
  
  if(m_pointerConversionsBeforeMatching > 0) {
    for(int a = 0; a < m_pointerConversionsBeforeMatching; ++a) {
      
      t = TypeUtils::increasePointerDepth(t);
      if(!t)
        return IndexedType();
    }
  }else{
    for(int a = m_pointerConversionsBeforeMatching; a < 0; ++a) {
      t = TypeUtils::decreasePointerDepth(t, m_duContext->topContext());
      if(!t)
        return IndexedType();
    }
  }
  
  return t->indexed();
}

CodeCompletionContext::~CodeCompletionContext() {
}

bool CodeCompletionContext::isValidPosition() {
  if( m_text.isEmpty() )
    return true;
  //If we are in a string or comment, we should not complete anything
  QString markedText = clearComments(m_text, '$');
  markedText = clearStrings(markedText,'$');

  if( markedText[markedText.length()-1] == '$' ) {
    //We are within a comment or string
    kDebug(9007) << "code-completion position is invalid, marked text: \n\"" << markedText << "\"\n unmarked text:\n" << m_text << "\n";
    return false;
  }
  return true;
}


bool CodeCompletionContext::isImplementationHelperValid()
{
  if (!parentContext() && (m_duContext->type() == DUContext::Namespace || m_duContext->type() == DUContext::Global))
    return true;
  else
    return false;
}

QString originalOperator( const QString& str ) {
  if( str == "[" )
    return "[]";
  return str;
}

QString CodeCompletionContext::getEndOperator( const QString& str ) const {

  for( QStringList::const_iterator it = allOperators.constBegin(); it != allOperators.constEnd(); ++it )
    if( str.endsWith(*it) )
      return *it;
  return QString();
}

QString CodeCompletionContext::getEndFunctionOperator( const QString& str ) const {
  return originalOperator( getEndOperator( str ) );
}

bool CodeCompletionContext::endsWithOperator( const QString& str ) const {
  return !getEndOperator(str).isEmpty();
}

// QList<KDevelop::AbstractType::Ptr> CodeCompletionContext::additionalMatchTypes() const {
//   QList<KDevelop::AbstractType::Ptr> ret;
//   if( m_operator == "=" && m_expressionResult.isValid() && m_expressionResult.isInstance ) {
//     //Conversion to the left operand-type
//     ret << m_expressionResult.type.type();
//   }
//   return ret;
// }

static TopDUContext* proxyContextForUrl(KUrl url)
{
  QList< ILanguage* > languages = ICore::self()->languageController()->languagesForUrl(url);
  foreach(ILanguage* language, languages)
  {
    if(language->languageSupport())
      return language->languageSupport()->standardContext(url, true);
  }
  
  return 0;
}

void CodeCompletionContext::preprocessText( int line ) {

  LOCKDUCHAIN;

  QSet<IndexedString> disableMacros;
  disableMacros.insert(IndexedString("SIGNAL"));
  disableMacros.insert(IndexedString("SLOT"));
  disableMacros.insert(IndexedString("emit"));
  disableMacros.insert(IndexedString("Q_EMIT"));
  disableMacros.insert(IndexedString("Q_SIGNAL"));
  disableMacros.insert(IndexedString("Q_SLOT"));
  
  if( m_duContext ) {
    
    // Use the proxy-context if possible, because that one contains most of the macros if existent
    TopDUContext* useTopContext = proxyContextForUrl(m_duContext->url().toUrl());
    if(!useTopContext)
      useTopContext = m_duContext->topContext();
    
    m_text = preprocess( m_text,  dynamic_cast<Cpp::EnvironmentFile*>(useTopContext->parsingEnvironmentFile().data()), line, disableMacros );
  }else{
    kWarning() << "error: no ducontext";
  }
}

CodeCompletionContext::MemberAccessOperation CodeCompletionContext::memberAccessOperation() const {
  return m_memberAccessOperation;
}

CodeCompletionContext* CodeCompletionContext::parentContext() {
  return static_cast<CodeCompletionContext*>(KDevelop::CodeCompletionContext::parentContext());
}

void getOverridable(DUContext* base, DUContext* current, QMap< QPair<IndexedType, IndexedString>, KDevelop::CompletionTreeItemPointer >& overridable, CodeCompletionContext::Ptr completionContext, int depth = 0) {
  if(!current)
    return;
  
  foreach(Declaration* decl, current->localDeclarations()) {
    ClassFunctionDeclaration* classFun = dynamic_cast<ClassFunctionDeclaration*>(decl);
    // one can only override the direct parent's ctor
    if(classFun && (classFun->isVirtual() || (depth == 0 && classFun->isConstructor()))) {
      QPair<IndexedType, IndexedString> key = qMakePair(classFun->indexedType(), classFun->identifier().identifier());
      if(base->owner()) {
        if(classFun->isConstructor() || classFun->isDestructor())
          key.second = base->owner()->identifier().identifier();
        if(classFun->isDestructor())
          key.second = IndexedString("~" + key.second.str());
      }
      if(!overridable.contains(key) && base->findLocalDeclarations(KDevelop::Identifier(key.second), CursorInRevision::invalid(), 0, key.first.abstractType(), KDevelop::DUContext::OnlyFunctions).isEmpty())
        overridable.insert(key, KDevelop::CompletionTreeItemPointer(new ImplementationHelperItem(ImplementationHelperItem::Override, DeclarationPointer(decl), completionContext, (classFun && classFun->isAbstract()) ? 1 : 2)));
    }
  }

  foreach(const DUContext::Import &import, current->importedParentContexts())
    getOverridable(base, import.context(base->topContext()), overridable, completionContext, depth + 1);
}

// #ifndef TEST_COMPLETION

QList< KSharedPtr< KDevelop::CompletionTreeElement > > CodeCompletionContext::ungroupedElements() {
  return m_storedUngroupedItems;
}

QList<CompletionTreeItemPointer> CodeCompletionContext::completionItems(bool& shouldAbort, bool fullCompletion) {
    LOCKDUCHAIN;
    QList<CompletionTreeItemPointer> items;

    if(!m_duContext || !m_valid)
      return items;

    typedef QPair<Declaration*, int> DeclarationDepthPair;

    bool ignoreParentContext = false;

    if(!m_storedItems.isEmpty() || m_useStoredItems) {
      items = m_storedItems;
    }else{
      switch(memberAccessOperation()) {
        case MemberAccess:
        case ArrowMemberAccess:
        case StaticMemberChoose:
        case MemberChoose:
          if( memberAccessContainer().isValid() ||memberAccessOperation() == Cpp::CodeCompletionContext::StaticMemberChoose )
          {
            bool typeIsConst = false;
            AbstractType::Ptr expressionTarget = TypeUtils::targetType(m_expressionResult.type.abstractType(), m_duContext->topContext());
            if (expressionTarget && (expressionTarget->modifiers() & AbstractType::ConstModifier))
              typeIsConst = true;
            
            QSet<QualifiedIdentifier> hadNamespaceDeclarations; //Used to show only one namespace-declaration per namespace
            QList<DUContext*> containers = memberAccessContainers();
            ifDebug( kDebug() << "got" << containers.size() << "member-access containers"; )
            if( !containers.isEmpty() ) {
              QSet<DUContext*> had;
              foreach(DUContext* ctx, containers) {
                if(had.contains(ctx)) //We need this so we don't process the same container twice
                  continue;
                had.insert(ctx);

                if (shouldAbort)
                  return items;
                ifDebug( kDebug() << "container:" << ctx->scopeIdentifier(true).toString(); )

                foreach( const DeclarationDepthPair& decl, Cpp::hideOverloadedDeclarations( ctx->allDeclarations(ctx->range().end, m_duContext->topContext(), false ) ) ) {
                  //If we have StaticMemberChoose, which means A::Bla, show only static members, except if we're within a class that derives from the container
                  ClassMemberDeclaration* classMember = dynamic_cast<ClassMemberDeclaration*>(decl.first);

                  if(classMember && !filterDeclaration(classMember, ctx, typeIsConst))
                    continue;
                  else if(!filterDeclaration(decl.first, ctx))
                    continue;

                  if (memberAccessOperation() == MemberAccess || memberAccessOperation() == ArrowMemberAccess) {
                    // Don't allow constructors to be accessed with . or ->
                    if (ClassFunctionDeclaration* classFun = dynamic_cast<ClassFunctionDeclaration*>(classMember))
                      if (classFun->isConstructor())
                       continue;
                  }
                  
                  if(decl.first->kind() == Declaration::Namespace) {
                    QualifiedIdentifier id = decl.first->qualifiedIdentifier();
                    if(hadNamespaceDeclarations.contains(id))
                      continue;
                    
                    hadNamespaceDeclarations.insert(id);
                  }

                  if(memberAccessOperation() != Cpp::CodeCompletionContext::StaticMemberChoose) {
                    if(decl.first->kind() != Declaration::Instance && decl.first->kind() != Declaration::Alias)
                      continue;
                    if(decl.first->abstractType().cast<EnumeratorType>())
                      continue; //Skip enumerators
                  }else{
                    ///@todo what NOT to show on static member choose? Actually we cannot hide all non-static functions, because of function-pointers
                  }

                  if(!decl.first->identifier().isEmpty())
                    items << CompletionTreeItemPointer( new NormalDeclarationCompletionItem( DeclarationPointer(decl.first), KDevelop::CodeCompletionContext::Ptr(this), decl.second ) );
                }
              }
            } else {
              ifDebug( kDebug() << "missing-include completion for" << m_expression << m_expressionResult.toString(); )
                eventuallyAddGroup(i18n("Not Included Container"), 700, missingIncludeCompletionItems(m_expression, QString(), m_expressionResult, m_duContext.data(), 0, true ));
            }
          }
          break;
        case ReturnAccess:
          {
            DUContext* functionContext = m_duContext.data();
            while(functionContext && !functionContext->owner())
              functionContext = functionContext->parentContext();
            if(functionContext && functionContext->owner()) {
              FunctionType::Ptr funType = functionContext->owner()->type<FunctionType>();
              if(funType) {
                if(funType->returnType()) {
                  items << CompletionTreeItemPointer( new TypeConversionCompletionItem( "return " + funType->returnType()->toString(), funType->returnType()->indexed(), depth(), KSharedPtr <Cpp::CodeCompletionContext >(this) ) );
                }
              }
            }
          }
        break;
        case TemplateAccess:
          {
            AbstractType::Ptr type = m_expressionResult.type.abstractType();
            IdentifiedType* identified = dynamic_cast<IdentifiedType*>(type.unsafeData());
            Declaration* decl = 0;
            if(identified)
              decl = identified->declaration( m_duContext->topContext());
            if(!decl && !m_expressionResult.allDeclarations.isEmpty())
              decl = m_expressionResult.allDeclarations[0].getDeclaration(m_duContext->topContext());
            if(decl) {
              NormalDeclarationCompletionItem* item = new NormalDeclarationCompletionItem( KDevelop::DeclarationPointer(decl),  KSharedPtr <KDevelop::CodeCompletionContext >(this), 0, 0 );
              item->m_isTemplateCompletion = true;
              items << CompletionTreeItemPointer( item );
            }else{
              items += missingIncludeCompletionItems(m_expression, QString(), m_expressionResult, m_duContext.data(), depth(), true );
            }
          }
          break;
        case FunctionCallAccess:
          {
            ifDebug( kDebug() << "functionCallAccess" << functions().count() << m_expression; )
            
            uint max = MoreArgumentHintsCompletionItem::resetMaxArgumentHints(!fullCompletion);
            
            //Don't show annoying empty argument-hints
/*            if(parentContext->m_contextType != BinaryOperatorFunctionCall && parentContext->functions().size() == 0)
              break;*/
            if(functions().count() == 0 && additionalContextType() != Cpp::CodeCompletionContext::BinaryOperatorFunctionCall) {
              items += missingIncludeCompletionItems(m_expression, QString(), m_expressionResult, m_duContext.data(), depth(), true );
            }else if(!functions().isEmpty()) {
              int num = 0;
              foreach( const Cpp::CodeCompletionContext::Function &function, functions() ) {
                if (num == max) {
                  if(fullCompletion)
                  {
                    //When there are too many overloaded functions, do not show them all
                    CompletionTreeItemPointer item( new MoreArgumentHintsCompletionItem( KSharedPtr <KDevelop::CodeCompletionContext >(this), i18ncp("Here, overload is used as a programming term.  This string is used to display how many overloaded versions there are of the function whose name is the second argument.", "1 more overload of %2 (show more)", "%1 more overloads of %2 (show more)", functions().count() - num, functionName()), num ) );
                    items.push_front(item);
                  }else if(!items.isEmpty()) {
                    NormalDeclarationCompletionItem* last = dynamic_cast<NormalDeclarationCompletionItem*>(items.back().data());
                    if(last->declaration())
                    {
//                       last->alternativeText = i18n("(%1 more) ", functions().count() - num) + last->declaration()->identifier().toString();
//                       last->useAlternativeText = true;
                    }
                  }
                  break;
                }

                items << CompletionTreeItemPointer( new NormalDeclarationCompletionItem( function.function.declaration(), KSharedPtr <KDevelop::CodeCompletionContext >(this), 0, num ) );
                ++num;
              }
            }
            
            if(additionalContextType() == Cpp::CodeCompletionContext::BinaryOperatorFunctionCall) {
              //Argument-hints for builtin operators
              AbstractType::Ptr type = m_expressionResult.type.abstractType();
              if(m_expressionResult.isValid() && m_expressionResult.isInstance && type) {
                IntegralType::Ptr integral = type.cast<IntegralType>();

                if(!integral && (arithmeticComparisonOperators.contains(m_operator) || binaryArithmeticOperators.contains(m_operator))) {
                  ///There is one more chance: If the type can be converted to an integral type, C++ will convert it first, and then
                  ///apply its builtin operators
                  integral = IntegralType::Ptr(new IntegralType(KDevelop::IntegralType::TypeInt));
                  TypeConversion conv(m_duContext->topContext());
                  if(!conv.implicitConversion(m_expressionResult.type, integral->indexed()))
                    integral = IntegralType::Ptr(); //No conversion possible
                }
                
                if( m_operator == "[]" && (type.cast<KDevelop::ArrayType>() || type.cast<KDevelop::PointerType>())) {
                  IntegralType::Ptr t(new IntegralType(IntegralType::TypeInt));
                  t->setModifiers(IntegralType::UnsignedModifier);
                  QString showName = "operator []";
                  items << CompletionTreeItemPointer( new TypeConversionCompletionItem( showName, t->indexed(), depth(), KSharedPtr <Cpp::CodeCompletionContext >(this) ) );
                }

                if( m_operator == "=" || integral ) {
                  ///Conversion to the left operand-type, builtin operators on integral types
                  IndexedType useType = integral ? integral->indexed() : m_expressionResult.type;
                  QString showName = functionName();
                  if(useType.abstractType())
                    showName = useType.abstractType()->toString() + " " + m_operator;

                  if(useType == m_expressionResult.type && m_expressionResult.allDeclarations.size() == 1) {
                    Declaration* decl = m_expressionResult.allDeclarations[0].getDeclaration(m_duContext->topContext());
                    if(decl)
                      showName = decl->toString() + " " + m_operator;
                  }

                  items << CompletionTreeItemPointer( new TypeConversionCompletionItem( showName, useType, depth(), KSharedPtr <Cpp::CodeCompletionContext >(this) ) );
                }
              }

//                 items.back()->asItem<NormalDeclarationCompletionItem>()->alternativeText = functionName();
            }else if(m_expressionResult.isValid() && m_expressionResult.type.abstractType() && (!m_expressionResult.isInstance || m_isDeclarationTypePrefix) && !m_expressionResult.type.type<FunctionType>()) {
              //Eventually add a builtin copy-constructor if a type is being constructed
              bool hasCopyConstructor = false;
              
              //Search for a copy-constructor within the class
              CppClassType::Ptr classType = m_expressionResult.type.type<CppClassType>();
              if(classType) {
                Declaration* decl = classType->declaration(m_duContext->topContext());
                if(decl) {
                  AbstractType::Ptr constClassType = classType->indexed().abstractType();
                  constClassType->setModifiers(AbstractType::ConstModifier);
                  
                  ReferenceType::Ptr argumentType(new ReferenceType);
                  argumentType->setBaseType(constClassType);
                  
                  DUContext* ctx = decl->internalContext();
                  if(ctx) {
                    QList<Declaration*> constructors = ctx->findLocalDeclarations(decl->identifier());
                    foreach(Declaration* constructor, constructors) {
                      FunctionType::Ptr funType = constructor->type<FunctionType>();
                      if(funType && !funType->returnType() && funType->arguments().size() == 1) {
                        if(funType->arguments()[0]->equals(argumentType.constData()))
                          hasCopyConstructor = true;
                      }
                    }
                  }
                }
              }
              
              if(!hasCopyConstructor && m_knownArgumentExpressions.isEmpty()) {
                  QString showName = m_expressionResult.type.abstractType()->toString() + "(";
                  items << CompletionTreeItemPointer( new TypeConversionCompletionItem( showName, m_expressionResult.type, depth(), KSharedPtr <Cpp::CodeCompletionContext >(this) ) );
              }
            }
          }
          break;
        case IncludeListAccess:
          //m_storedItems is used for include-list access
          {
            //Include-file completion
            int cnt = 0;
            QList<KDevelop::IncludeItem> allIncludeItems = includeItems();
            foreach(const KDevelop::IncludeItem& includeItem, allIncludeItems) {
              if (shouldAbort)
                return items;

              items << CompletionTreeItemPointer( new IncludeFileCompletionItem(includeItem) );
              ++cnt;
            }
            kDebug(9007) << "Added " << cnt << " include-files to completion-list";
          }
          break;
        case SignalAccess:
        case SlotAccess:
        {
        KDevelop::IndexedDeclaration connectedSignal;
        if(!m_connectedSignalIdentifier.isEmpty()) {
          ///Create an additional argument-hint context that shows information about the signal we connect to
          if(parentContext() && parentContext()->m_knownArgumentTypes.count() > 1 && parentContext()->m_knownArgumentTypes[0].type.isValid()) {
            StructureType::Ptr signalContainerType = TypeUtils::targetType(parentContext()->m_knownArgumentTypes[0].type.abstractType(), m_duContext->topContext()).cast<StructureType>();
           if(signalContainerType) {
//             kDebug() << "searching signal in container" << signalContainerType->toString() << m_connectedSignalIdentifier.toString();
               Declaration* signalContainer = signalContainerType->declaration(m_duContext->topContext());
              if(signalContainer && signalContainer->internalContext()) {
                IndexedString signature(m_connectedSignalNormalizedSignature);
                foreach(const DeclarationDepthPair &decl, signalContainer->internalContext()->allDeclarations( CursorInRevision::invalid(), m_duContext->topContext(), false )) {
                  if(decl.first->identifier() == m_connectedSignalIdentifier) {
                    if(QtFunctionDeclaration* classFun = dynamic_cast<QtFunctionDeclaration*>(decl.first)) {
                      if(classFun->isSignal() && classFun->normalizedSignature() == signature) {
                        //Match
                        NormalDeclarationCompletionItem* item = new NormalDeclarationCompletionItem( DeclarationPointer(decl.first), KDevelop::CodeCompletionContext::Ptr(parentContext()), decl.second + 50);
                        item->useAlternativeText = true;
                        m_connectedSignal = IndexedDeclaration(decl.first);
                        item->alternativeText = i18n("Connect to %1 (%2)", decl.first->qualifiedIdentifier().toString(), QString::fromUtf8(m_connectedSignalNormalizedSignature) );
                        item->m_isQtSignalSlotCompletion = true;
                        items << CompletionTreeItemPointer(item);
                        connectedSignal = IndexedDeclaration(decl.first);
                      }
                    }
                  }
                }
              }
            }
          }
        }
        if( memberAccessContainer().isValid() ) {
          QList<CompletionTreeItemPointer> signalSlots;
          ///Collect all slots/signals to show
          AbstractType::Ptr type = memberAccessContainer().type.abstractType();
          IdentifiedType* identified = dynamic_cast<IdentifiedType*>(type.unsafeData());
          if(identified) {
            Declaration* decl = identified->declaration(m_duContext->topContext());
            if(decl && decl->internalContext() /*&& Cpp::findLocalDeclarations(decl->internalContext(), Identifier("QObject"), m_duContext->topContext()).count()*/) { //hacky test whether it's a QObject
              ///@todo Always allow this when the class is within one of the open projects. Problem: The project lookup is not threadsafe
              if(connectedSignal.isValid() && m_localClass.data() == decl->internalContext()) { ///Create implementation-helper to add a slot
                signalSlots << CompletionTreeItemPointer(new ImplementationHelperItem(ImplementationHelperItem::CreateSignalSlot, DeclarationPointer(connectedSignal.data()), CodeCompletionContext::Ptr(this)));
              }
              
              foreach(const DeclarationDepthPair &candidate, decl->internalContext()->allDeclarations(CursorInRevision::invalid(), m_duContext->topContext(), false) ) {
                if(QtFunctionDeclaration* classFun = dynamic_cast<QtFunctionDeclaration*>(candidate.first)) {
                  if((classFun->isSignal() && m_onlyShow != ShowSlots) || (memberAccessOperation() == SlotAccess && classFun->isSlot() && filterDeclaration(classFun))) {
                    NormalDeclarationCompletionItem* item = new NormalDeclarationCompletionItem( DeclarationPointer(candidate.first), KDevelop::CodeCompletionContext::Ptr(this), candidate.second );
                    item->m_isQtSignalSlotCompletion = true;
                    if(!m_connectedSignalIdentifier.isEmpty()) {
                      item->m_fixedMatchQuality = 0;
                      //Compute a match-quality, by comparing the strings
                      QByteArray thisSignature = classFun->normalizedSignature().byteArray();
                      if(m_connectedSignalNormalizedSignature.startsWith(thisSignature) || (m_connectedSignalNormalizedSignature.isEmpty() && thisSignature.isEmpty())) {
                        QByteArray remaining = m_connectedSignalNormalizedSignature.mid(thisSignature.length());
                        int remainingElements = remaining.split(',').count();
                        if(remaining.isEmpty())
                          item->m_fixedMatchQuality = 10;
                        else if(remainingElements < 4)
                          item->m_fixedMatchQuality  = 6 - remainingElements;
                        else
                          item->m_fixedMatchQuality = 2;
                      }
                    }else{
                      item->m_fixedMatchQuality = 10;
                    }
                    signalSlots << CompletionTreeItemPointer( item );
                  }
                }
              }
              
              eventuallyAddGroup(i18n("Signals/Slots"), 10, signalSlots);
            }
          }
        }
        }
        //Since there is 2 connect() functions, the third argument may be a slot as well as a QObject*, so also
        //give normal completion items
        if(parentContext() && parentContext()->m_knownArgumentExpressions.size() != 2)
          break;
        default:
          if(depth() == 0 && (m_onlyShow == ShowAll || m_onlyShow == ShowTypes))
            standardAccessCompletionItems(items);
          break;
      }
    }

    if(!ignoreParentContext 
      && (fullCompletion || (Cpp::useArgumentHintInAutomaticCompletion() && depth() == 0))
      && m_parentContext
      && (!noMultipleBinaryOperators || m_contextType != BinaryOperatorFunctionCall || parentContext()->m_contextType != BinaryOperatorFunctionCall))
      items = parentContext()->completionItems( shouldAbort, fullCompletion ) + items;

    if(depth() == 0) {
      //Eventually add missing include-completion in cases like SomeNamespace::NotIncludedClass|
      if(memberAccessOperation() == StaticMemberChoose) {
#ifndef TEST_COMPLETION  
        MissingIncludeCompletionModel::self().startWithExpression(m_duContext, m_expression + "::", m_followingText.trimmed());
#endif
      }

      if(m_duContext->type() == DUContext::Class && !parentContext()) {
        //Show override helper items
        QMap< QPair<IndexedType, IndexedString>, KDevelop::CompletionTreeItemPointer > overridable;
        foreach(const DUContext::Import &import, m_duContext->importedParentContexts())
        {
          DUContext* ctx = import.context(m_duContext->topContext());
          if(ctx)
            getOverridable(m_duContext.data(), ctx, overridable, Ptr(this));
        }
        
        if(!overridable.isEmpty()) {
          eventuallyAddGroup(i18n("Virtual Override"), 0, overridable.values());
        }
      }

      if(isImplementationHelperValid()) {
        if(m_onlyShow != ShowVariables && !m_isConstructorCompletion && 
           (m_memberAccessOperation == NoMemberAccess || m_memberAccessOperation == StaticMemberChoose)) {
          QList<CompletionTreeItemPointer> helpers = getImplementationHelpers();
          if(!helpers.isEmpty()) {
            eventuallyAddGroup(i18n("Implement Function"), 0, helpers);
          }
        }
      }
    }

    return items;
}

QList<CompletionTreeItemPointer> CodeCompletionContext::getImplementationHelpers() {
  QList<CompletionTreeItemPointer> ret;
#ifndef TEST_COMPLETION
  TopDUContext* searchInContext = m_duContext->topContext();
  
  if(searchInContext)
    ret += getImplementationHelpersInternal(m_duContext->scopeIdentifier(true), searchInContext);
  
  if(!CppUtils::isHeader( searchInContext->url().toUrl() )) {
    KUrl headerUrl = CppUtils::sourceOrHeaderCandidate( searchInContext->url().toUrl(), true );
    searchInContext = ICore::self()->languageController()->language("C++")->languageSupport()->standardContext(headerUrl);
  }

  if(searchInContext)
    ret += getImplementationHelpersInternal(m_duContext->scopeIdentifier(true), searchInContext);
 
#endif
  return ret;
}

QList<CompletionTreeItemPointer> CodeCompletionContext::getImplementationHelpersInternal(QualifiedIdentifier minimumScope, DUContext* context) {
  QList<CompletionTreeItemPointer> ret;

  foreach(Declaration* decl, context->localDeclarations()) {
    ClassFunctionDeclaration* classFun = dynamic_cast<ClassFunctionDeclaration*>(decl);
    AbstractFunctionDeclaration* funDecl = dynamic_cast<AbstractFunctionDeclaration*>(decl);
    if(funDecl  && !decl->range().isEmpty() && (!classFun || (!classFun->isAbstract() && !classFun->isSignal())) && !decl->isDefinition() && !FunctionDefinition::definition(decl) && decl->qualifiedIdentifier().toString().startsWith(minimumScope.toString()))
      ret << KDevelop::CompletionTreeItemPointer(new ImplementationHelperItem(ImplementationHelperItem::CreateDefinition, DeclarationPointer(decl), KSharedPtr<CodeCompletionContext>(this)));
  }

  foreach(DUContext* child, context->childContexts())
    if(child->type() == DUContext::Namespace || child->type() == DUContext::Class || child->type() == DUContext::Helper)
      ret += getImplementationHelpersInternal(minimumScope, child);
  return ret;
}

QualifiedIdentifier CodeCompletionContext::requiredPrefix(Declaration* decl) const {
  QualifiedIdentifier worstCase = decl->context()->scopeIdentifier(true);
  if(!m_duContext)
    return worstCase;
  QualifiedIdentifier currentPrefix;

  while(1) {
    QList<Declaration*> found = m_duContext->findDeclarations( currentPrefix + decl->identifier() );
    if(found.contains(decl))
      return currentPrefix;

    if(currentPrefix.count() >= worstCase.count()) {
      return worstCase;
    }else {
      currentPrefix.push(worstCase.at(currentPrefix.count()));
    }
  }
}

QList< KSharedPtr< KDevelop::CompletionTreeItem > > CodeCompletionContext::specialItemsForArgumentType(TypePtr< KDevelop::AbstractType > type) {
  QList< KSharedPtr< KDevelop::CompletionTreeItem > > items;
  if(EnumerationType::Ptr enumeration = TypeUtils::realType(type, m_duContext->topContext()).cast<EnumerationType>()) {
    Declaration* enumDecl = enumeration->declaration(m_duContext->topContext());
    if(enumDecl && enumDecl->internalContext()) {

      QualifiedIdentifier prefix = requiredPrefix(enumDecl);

      DUContext* enumInternal = enumDecl->internalContext();
      foreach(Declaration* enumerator, enumInternal->localDeclarations()) {
        QualifiedIdentifier id = prefix + enumerator->identifier();
        items << CompletionTreeItemPointer(new NormalDeclarationCompletionItem( DeclarationPointer(enumerator), KDevelop::CodeCompletionContext::Ptr(this), 0 ));
        static_cast<NormalDeclarationCompletionItem*>(items.back().data())->alternativeText = id.toString();
        static_cast<NormalDeclarationCompletionItem*>(items.back().data())->useAlternativeText = true;
      }
    }
  }
  return items;
}

void CodeCompletionContext::standardAccessCompletionItems(QList<CompletionTreeItemPointer>& items) {
  //Normal case: Show all visible declarations
  typedef QPair<Declaration*, int> DeclarationDepthPair;
  QSet<QualifiedIdentifier> hadNamespaceDeclarations;

  bool typeIsConst = false;
  if (Declaration* func = Cpp::localFunctionFromCodeContext(m_duContext.data())) {
    if (func->abstractType() && (func->abstractType()->modifiers() & AbstractType::ConstModifier))
      typeIsConst = true;
  }

  QList<DeclarationDepthPair> decls = m_duContext->allDeclarations(m_duContext->type() == DUContext::Class ? m_duContext->range().end : m_position, m_duContext->topContext());

  //Collect the contents of unnamed namespaces
  QList<Declaration*> unnamed = m_duContext->findDeclarations(QualifiedIdentifier(unnamedNamespaceIdentifier().identifier()), m_position);
  foreach(Declaration* ns, unnamed)
    if(ns->kind() == Declaration::Namespace && ns->internalContext())
      decls += ns->internalContext()->allDeclarations(m_position, m_duContext->topContext(), false);

  if(m_duContext) {
    //Collect the Declarations from all "using namespace" imported contexts
    QList<Declaration*> imports = m_duContext->findDeclarations( globalImportIdentifier(), m_position, 0, DUContext::NoFiltering );

    QSet<QualifiedIdentifier> ids;
    foreach(Declaration* importDecl, imports) {
      NamespaceAliasDeclaration* aliasDecl = dynamic_cast<NamespaceAliasDeclaration*>(importDecl);
      if(aliasDecl) {
        ids.insert(aliasDecl->importIdentifier());
      }else{
        kDebug() << "Import is not based on NamespaceAliasDeclaration";
      }
    }
    
    QualifiedIdentifier ownNamespaceScope = Cpp::namespaceScopeComponentFromContext(m_duContext->scopeIdentifier(true), m_duContext.data(), m_duContext->topContext());
    if(!ownNamespaceScope.isEmpty())
      for(int a = 1; a <= ownNamespaceScope.count(); ++a)
        ids += ownNamespaceScope.left(a);

    foreach(const QualifiedIdentifier &id, ids) {
      QList<Declaration*> importedContextDecls = m_duContext->findDeclarations( id );
      foreach(Declaration* contextDecl, importedContextDecls) {
        if(contextDecl->kind() != Declaration::Namespace || !contextDecl->internalContext())
          continue;
        DUContext* context = contextDecl->internalContext();
        
        if(context->range().contains(m_duContext->range()) && context->url() == m_duContext->url())
          continue; //If the context surrounds the current one, the declarations are visible through allDeclarations(..).
        foreach(Declaration* decl, context->localDeclarations()) {
          if(filterDeclaration(decl))
            decls << qMakePair(decl, 1200);
        }
      }
    }
  }

  QList<DeclarationDepthPair> oldDecls = decls;
  decls.clear();
  
  //Remove pure function-definitions before doing overload-resolution, so they don't hide their own declarations.
  foreach( const DeclarationDepthPair& decl, oldDecls )
    if(!dynamic_cast<FunctionDefinition*>(decl.first) || !static_cast<FunctionDefinition*>(decl.first)->hasDeclaration()) {
      if(decl.first->kind() == Declaration::Namespace) {
        QualifiedIdentifier id = decl.first->qualifiedIdentifier();
        if(hadNamespaceDeclarations.contains(id))
          continue;
        
        hadNamespaceDeclarations.insert(id);
      }
      
      if(filterDeclaration(decl.first, 0, true, typeIsConst)) {
        decls << decl;
      }
    }
    
  decls = Cpp::hideOverloadedDeclarations(decls);

  foreach( const DeclarationDepthPair& decl, decls )
    items << CompletionTreeItemPointer( new NormalDeclarationCompletionItem(DeclarationPointer(decl.first), KDevelop::CodeCompletionContext::Ptr(this), decl.second ) );

  ///Eventually show additional specificly known items for the matched argument-type, like for example enumerators for enum types
  CodeCompletionContext* parent = parentContext();
  if(parent) {
    if(parent->memberAccessOperation() == FunctionCallAccess) {
      foreach(const Cpp::OverloadResolutionFunction& function, parent->functions()) {
        if(function.function.isValid() && function.function.isViable() && function.function.declaration()) {
          //uint parameterNumber = parent->m_knownArgumentExpressions.size() + function.matchedArguments;
          Declaration* functionDecl = function.function.declaration().data();
          if(functionDecl->type<FunctionType>()->arguments().count() > function.matchedArguments) {
            items += specialItemsForArgumentType(functionDecl->type<FunctionType>()->arguments()[function.matchedArguments]);
          }
        }
      }
    }
  }

  ///Eventually add a "this" item
  DUContext* functionContext = m_duContext.data();
  if(m_onlyShow != ShowSignals && m_onlyShow != ShowSlots && m_onlyShow != ShowTypes) {
    while(functionContext && functionContext->type() == DUContext::Other && functionContext->parentContext() && functionContext->parentContext()->type() == DUContext::Other)
      functionContext = functionContext->parentContext();
  }

  ClassFunctionDeclaration* classFun = dynamic_cast<ClassFunctionDeclaration*>(DUChainUtils::declarationForDefinition(functionContext->owner(), m_duContext->topContext()));
  
  if(classFun && !classFun->isStatic() && classFun->context()->owner()
              && m_onlyShow != ShowSignals && m_onlyShow != ShowSlots && m_onlyShow != ShowTypes)
  {
    AbstractType::Ptr classType = classFun->context()->owner()->abstractType();
    if(classFun->abstractType()->modifiers() & AbstractType::ConstModifier)
      classType->setModifiers((AbstractType::CommonModifiers)(classType->modifiers() | AbstractType::ConstModifier));
    PointerType::Ptr thisPointer(new PointerType());
    thisPointer->setModifiers(AbstractType::ConstModifier);
    thisPointer->setBaseType(classType);
    KSharedPtr<TypeConversionCompletionItem> item( new TypeConversionCompletionItem("this", thisPointer->indexed(), 0, KSharedPtr <Cpp::CodeCompletionContext >(this)) );
    item->setPrefix(thisPointer->toString());
    QList<CompletionTreeItemPointer> lst;
    lst += CompletionTreeItemPointer(item.data());
    eventuallyAddGroup(i18n("C++ Builtin"), 800, lst);
  }

  //Eventually add missing include-completion in cases like NotIncludedClass|
//   if(!m_followingText.trimmed().isEmpty()) {
//     uint oldItemCount = items.count();
//     items += missingIncludeCompletionItems(totalExpression, m_followingText.trimmed() + ": ", ExpressionEvaluationResult(), m_duContext.data(), 0);
#ifndef TEST_COMPLETION
    MissingIncludeCompletionModel::self().startWithExpression(m_duContext, QString(), m_followingText.trimmed());
#endif
//     kDebug() << QString("added %1 missing-includes for %2").arg(items.count()-oldItemCount).arg(totalExpression);
//   }
  
  eventuallyAddGroup(i18n("C++ Builtin"), 800, keywordCompletionItems());
}

bool CodeCompletionContext::visibleFromWithin(KDevelop::Declaration* decl, DUContext* currentContext) {
  if(!decl || !currentContext)
    return false;
  if(currentContext->imports(decl->context()))
    return true;
  
  return visibleFromWithin(decl, currentContext->parentContext());
}

/**
 * see @p type as function type and try to get it's return type as IntegralType data type.
 */
static inline int getIntegralReturnType(const AbstractType::Ptr& type)
{
  if (!type)
    return -1;
  const FunctionType::Ptr funcType = type.cast<FunctionType>();
  if (!funcType || !funcType->returnType())
    return -1;
  const IntegralType::Ptr intType = funcType->returnType().cast<IntegralType>();
  if (!intType)
    return -1;
  return intType->dataType();
}

bool  CodeCompletionContext::filterDeclaration(Declaration* decl, DUContext* declarationContext, bool dynamic, bool typeIsConst) {
  if(!decl)
    return true;

  if(dynamic_cast<TemplateParameterDeclaration*>(decl) && !visibleFromWithin(decl, m_duContext.data()))
    return false;
  
  static IndexedIdentifier friendIdentifier(Identifier("friend"));
  
  if(decl->indexedIdentifier().isEmpty()) //Filter out nameless declarations
    return false;
  
  if(decl->indexedIdentifier() == friendIdentifier || decl->indexedIdentifier() == Cpp::unnamedNamespaceIdentifier())
    return false;
  
  if(excludeReservedIdentifiers)
  {
    //Exclude identifiers starting with "__" or "_Uppercase"
    IndexedString str = decl->indexedIdentifier().identifier().identifier();
    const char* cstr = str.c_str();
    if(str.length() > 2 && cstr[0] == '_' && (cstr[1] == '_' || QChar(cstr[1]).isUpper()) && decl->url() != m_duContext->url())
      return false;
  }

  if(ClassDeclaration* cDecl = dynamic_cast<ClassDeclaration*>(decl)) {
    ///TODO: indexedIdentifier().isEmpty() should be fixed for this case...
    if (cDecl->classType() == ClassDeclarationData::Struct && cDecl->identifier().toString().isEmpty()) {
      // filter anonymous structs
      return false;
    }
  }

  if(m_onlyShow == ShowTypes && decl->kind() != Declaration::Type && decl->kind() != Declaration::Namespace
     && decl->kind() != Declaration::NamespaceAlias )
    return false;
  
  if(m_onlyShow == ShowVariables && (decl->kind() != Declaration::Instance || decl->isFunctionDeclaration()))
    return false;
  
  if(m_onlyShow == ShowImplementationHelpers)
    return false; //Implementation helpers don't come here
    
  if(m_onlyShow == ShowSignals || m_onlyShow == ShowSlots) {
    Cpp::QtFunctionDeclaration* qtFunction = dynamic_cast<Cpp::QtFunctionDeclaration*>(decl);
    if(!qtFunction || (m_onlyShow == ShowSignals && !qtFunction->isSignal())
                   || (m_onlyShow == ShowSlots && !qtFunction->isSlot()))
      return false;
  }
  
  if(dynamic && decl->context()->type() == DUContext::Class) {
    ClassMemberDeclaration* classMember = dynamic_cast<ClassMemberDeclaration*>(decl);
    if(classMember)
      return filterDeclaration(classMember, declarationContext, typeIsConst);
  }

  // https://bugs.kde.org/show_bug.cgi?id=206376
  // hide void functions in expressions but don't hide signals / slots with void return type
  if (m_onlyShow != ShowSignals && m_onlyShow != ShowSlots
      && m_parentContext && decl->isFunctionDeclaration()
      && getIntegralReturnType(decl->abstractType()) == IntegralType::TypeVoid)
  {
    const ExpressionEvaluationResult& result =
            static_cast<CodeCompletionContext*>(m_parentContext.data())->m_expressionResult;
    // for now only hide in non-lvalue expressions so we don't get problems in sig/slot connections e.g.
    if (result.type.isValid() && !result.isLValue())
      return false;
  }

  return true;
}

bool  CodeCompletionContext::filterDeclaration(ClassMemberDeclaration* decl, DUContext* declarationContext, bool typeIsConst) {
  if(m_doAccessFiltering && decl) {
    if (typeIsConst && decl->type<FunctionType>() && !(decl->abstractType()->modifiers() & AbstractType::ConstModifier))
      return false;
    if(!Cpp::isAccessible(m_localClass ? m_localClass.data() : m_duContext.data(), decl, m_duContext->topContext(), declarationContext))
      return false;
  }
  // filter properties from code completion, they mostly have to be accessed via their getter/setters
  if (QPropertyDeclaration* property = dynamic_cast<QPropertyDeclaration*>(decl)) {
    return false;
  }
  return filterDeclaration((Declaration*)decl, declarationContext, false);
}

void CodeCompletionContext::replaceCurrentAccess(QString old, QString _new)
{
  //We must not change the document from within the background, so we use a queued connection to an object created in the foregroud
  QMetaObject::invokeMethod(&accessReplacer, "exec", Qt::QueuedConnection, Q_ARG(KUrl, m_duContext->url().toUrl()), Q_ARG(QString, old), Q_ARG(QString, _new));
}

int CodeCompletionContext::matchPosition() const {
  return m_knownArgumentExpressions.count();
}

void CodeCompletionContext::eventuallyAddGroup(QString name, int priority, QList< KSharedPtr< KDevelop::CompletionTreeItem > > items) {
  if(items.isEmpty())
    return;
  KDevelop::CompletionCustomGroupNode* node = new KDevelop::CompletionCustomGroupNode(name, priority);
  node->appendChildren(items);
  m_storedUngroupedItems << CompletionTreeElementPointer(node);
}

QList< KSharedPtr< KDevelop::CompletionTreeItem > > CodeCompletionContext::keywordCompletionItems() {
  QList<CompletionTreeItemPointer> ret;
  #ifdef TEST_COMPLETION
  return ret;
  #endif
  #define ADD_TYPED_TOKEN_S(X, type) ret << CompletionTreeItemPointer( new TypeConversionCompletionItem(X, type, 0, KSharedPtr<Cpp::CodeCompletionContext>(this)) )
  #define ADD_TYPED_TOKEN(X, type) ADD_TYPED_TOKEN_S(#X, type)
  
  #define ADD_TOKEN(X) ADD_TYPED_TOKEN(X, KDevelop::IndexedType())
  #define ADD_TOKEN_S(X) ADD_TYPED_TOKEN_S(X, KDevelop::IndexedType())

  bool restrictedItems = (m_onlyShow == ShowSignals) ||
                         (m_onlyShow == ShowSlots) || 
                         (m_onlyShow == ShowTypes) ||
                         (m_onlyShow == ShowImplementationHelpers);
  
  if(!restrictedItems || m_onlyShow == ShowTypes) {
    ADD_TOKEN(bool);
    ADD_TOKEN(char);
    ADD_TOKEN(const);
    ADD_TOKEN(double);
    ADD_TOKEN(enum);
    ADD_TOKEN(float);
    ADD_TOKEN(int);
    ADD_TOKEN(long);
    ADD_TOKEN(mutable);
    ADD_TOKEN(register);
    ADD_TOKEN(short);
    ADD_TOKEN(signed);
    ADD_TOKEN(struct);
    ADD_TOKEN(template);
    ADD_TOKEN(typename);
    ADD_TOKEN(union);
    ADD_TOKEN(unsigned);
    ADD_TOKEN(void);
    ADD_TOKEN(volatile);
    ADD_TOKEN(wchar_t);
  }
  
  if(restrictedItems && (m_duContext->type() == DUContext::Other || m_duContext->type() == DUContext::Function))
    return ret;
  
  if(m_duContext->type() == DUContext::Class) {
    ADD_TOKEN_S("Q_OBJECT");
    ADD_TOKEN(private);
    ADD_TOKEN(protected);
    ADD_TOKEN(public);
    ADD_TOKEN_S("signals");
    ADD_TOKEN_S("slots");
    ADD_TOKEN(virtual);
    ADD_TOKEN(friend);
    ADD_TOKEN(explicit);
  }
  
  if(m_duContext->type() == DUContext::Other) {
    ADD_TOKEN(break);
    ADD_TOKEN(case);
    ADD_TOKEN(and);
    ADD_TOKEN(and_eq);
    ADD_TOKEN(asm);
    ADD_TOKEN(bitand);
    ADD_TOKEN(bitor);
    ADD_TOKEN(catch);
    ADD_TOKEN(const_cast);
    ADD_TOKEN(default);
    ADD_TOKEN(delete);
    ADD_TOKEN(do);
    ADD_TOKEN(dynamic_cast);
    ADD_TOKEN(else);
    ADD_TOKEN_S("emit");
    ADD_TOKEN(for);
    ADD_TOKEN(goto);
    ADD_TOKEN(if);
    ADD_TOKEN(incr);
    ADD_TOKEN(new);
    ADD_TOKEN(not);
    ADD_TOKEN(not_eq);
    ADD_TOKEN(or);
    ADD_TOKEN(or_eq);
    ADD_TOKEN(reinterpret_cast);
    ADD_TOKEN(return);
    ADD_TOKEN(static_cast);
    ADD_TOKEN(switch);
    ADD_TOKEN(try);
    ADD_TOKEN(typeid);
    ADD_TOKEN(while);
    ADD_TOKEN(xor);
    ADD_TOKEN(xor_eq);
    ADD_TOKEN(continue);
  }else{
    ADD_TOKEN(inline);
  }
  
  if(m_duContext->type() == DUContext::Global) {
    ADD_TOKEN(export);
    ADD_TOKEN(extern);
    ADD_TOKEN(namespace);
  }
  
  ADD_TOKEN(auto);
  ADD_TOKEN(class);
  ADD_TOKEN(operator);
  ADD_TOKEN(sizeof);
  ADD_TOKEN(static);
  ADD_TOKEN(throw);
  ADD_TOKEN(typedef);
  ADD_TOKEN(using);

  ConstantIntegralType::Ptr trueType(new ConstantIntegralType(IntegralType::TypeBoolean));
  trueType->setValue<bool>(true);
  
  ADD_TYPED_TOKEN(true, trueType->indexed());

  ConstantIntegralType::Ptr falseType(new ConstantIntegralType(IntegralType::TypeBoolean));
  falseType->setValue<bool>(false);

  ADD_TYPED_TOKEN(false, falseType->indexed());
  
  return ret;
}

QString CodeCompletionContext::followingText() const {
  return m_followingText;
}

void CodeCompletionContext::setFollowingText(QString str) {
  m_followingText = str;
}


}

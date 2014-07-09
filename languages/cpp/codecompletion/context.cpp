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

#include <iterator>

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
#include <language/duchain/problem.h>
#include <util/pushvalue.h>

#include "../cppduchain/cppduchain.h"
#include "../cppduchain/typeutils.h"
#include "../cppduchain/overloadresolution.h"
#include "../cppduchain/overloadresolutionhelper.h"
#include "../cppduchain/viablefunctions.h"
#include "../cppduchain/environmentmanager.h"
#include "../cppduchain/cpptypes.h"
#include "../cppduchain/templatedeclaration.h"
#include "../cpplanguagesupport.h"
#include "../cpputils.h"
#include "../cppduchain/environmentmanager.h"
#include "../cppduchain/cppduchain.h"

#include "cppdebughelper.h"
#include "missingincludeitem.h"
#include "implementationhelperitem.h"
#include <qtfunctiondeclaration.h>
#include <templateparameterdeclaration.h>
#include <expressionparser.h>
#include <language/duchain/classdeclaration.h>
#include "model.h"
#include "helpers.h"

// #define ifDebug(x) x

#define LOCKDUCHAIN     DUChainReadLocker lock(DUChain::lock())
#include <cpputils.h>
#include <interfaces/ilanguage.h>
#include <interfaces/foregroundlock.h>
#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>

using namespace KDevelop;

namespace {

///If this is enabled, no chain of useless argument-hints for binary operators is created.
const bool NO_MULTIPLE_BINARY_OPERATORS = true;
///Whether only items that are allowed to be accessed should be shown
const bool DO_ACCESS_FILTERING = true;
///Lines of text to keep for processing each context
const int CONTEXT_LINES = 20;
///Maximum number of parent contexts
const int MAX_DEPTH = 10;

/** 
 * ACCESS_STRINGS are used to determine the special properties of the context.
 * Any alphanum_ word not appearing in ACCESS_STRINGS will be considered an expression.
 * Expressions that fail evaluation invalidate their context and have no completions.
 * Don't list a keyword in ACCESS_STRINGS if:
 * * it should invalidate the context, ie ("break", "continue").
 * * it should invalidate parent-only contexts, ie "if" will correctly invalidate a function ctxt
 * Do list a keyword in ACCESS_STRINGS if:
 * * it can be used to limit valid completions (see m_onlyShow, SHOW_TYPES_ACCESS_STRINGS)
 * * if it should be ignored (ie, "else", "throw")
 * * if it should be passed to a parent context for special processing (ie, "return", "(")
 * UNARY_OPERATORS are not ACCESS_STRINGS, but are stripped away after counting ptr depth
 **/
const QSet<QString> BINARY_ARITHMETIC_OPERATORS = QString("+ - * / % ^ & | < >").split(' ').toSet();
const QSet<QString> ARITHMETIC_COMPARISON_OPERATORS = QString("!= <= >= < >").split(' ').toSet();
//technically ".", "->", ".*", "->*", "::" are also binary operators, but they're handled differently
const QSet<QString> BINARY_OPERATORS =
      QString("+= -= *= /= %= ^= &= |= ~= << >> >>= <<= == && || [ =").split(' ').toSet() +
      BINARY_ARITHMETIC_OPERATORS + ARITHMETIC_COMPARISON_OPERATORS;
//These will be skipped over to find parent contexts
const QSet<QString> UNARY_OPERATORS = QString("++ -- ! ~ + - & *").split(' ').toSet();
//When these operators create a parent context it's safe to use their expression type to match against
//This is only used for the builtin operators, and not the overloaded operators (assuming overloaded operators are found)
//Add any operators where you expect the lhs type and the rhs type to be the same, at least usually
//TODO: boolean logic operators could maybe have a fixed match type of boolean
const QSet<QString> MATCH_TYPE_OPERATORS = QString("!= <= >= = == + - * / % > < -= += *= /= %=").split(' ').toSet();
const QSet<QString> KEYWORD_ACCESS_STRINGS = QString("const_cast< static_cast< dynamic_cast< reinterpret_cast< const typedef public public: protected protected: private private: virtual return else throw emit Q_EMIT case delete delete[] new friend class namespace").split(' ').toSet();
//When these appear as access strings, only show types
const QSet<QString> SHOW_TYPES_ACCESS_STRINGS = QString("const_cast< static_cast< dynamic_cast< reinterpret_cast< const typedef public protected private virtual new friend class").split(' ').toSet();
//A parent context is created for these access strings
//TODO: delete, case and possibly also xxx_cast< should open a parent context and get specialized handling
const QSet<QString> PARENT_ACCESS_STRINGS = BINARY_OPERATORS + QString("< , ( : return case").split(' ').toSet();
//TODO: support ".*" and "->*" as MEMBER_ACCESS_STRINGS
const QSet<QString> MEMBER_ACCESS_STRINGS = QString(". -> ::").split(' ').toSet();
const QSet<QString> ACCESS_STRINGS = KEYWORD_ACCESS_STRINGS + PARENT_ACCESS_STRINGS + MEMBER_ACCESS_STRINGS;

///Pass these to getEndingFromSet in order to specify longest valid match for above sets
const int ACCESS_STR_MATCH = 17; //reinterpret_cast<
const int MEMBER_ACCESS_STR_MATCH = 2; //::
const int BINARY_OPERATOR_MATCH = 3; //>>=
const int UNARY_OPERATOR_MATCH = 2; //++

//Whether identifiers starting with "__" or "_Uppercase" and that are not declared in the current file should be excluded from the code completion
const bool excludeReservedIdentifiers = true;

/**
 * A helper "worker" object which lives in the main thread.
 */
struct MainThreadHelper : public QObject
{
  Q_OBJECT

  public slots:
    void replaceCurrentAccess(const KUrl& url, const QString& oldAccess, const QString& newAccess);
};

void MainThreadHelper::replaceCurrentAccess(const KUrl& url, const QString& oldAccess, const QString& newAccess)
{
  IDocument* document = ICore::self()->documentController()->documentForUrl(url);
  if(document) {
    KTextEditor::View* activeView = document->activeTextView();
    if(activeView) {
      KTextEditor::Document* textDocument = activeView->document();
      if(textDocument) {
        KTextEditor::Cursor cursor = activeView->cursorPosition();

        static KUrl lastUrl;
        static KTextEditor::Cursor lastPos;
        if(lastUrl == url && lastPos == cursor) {
          kDebug() << "Not doing the same access replacement twice at" << lastUrl << lastPos;
          return;
        }
        lastUrl = url;
        lastPos = cursor;

        KTextEditor::Range oldRange = KTextEditor::Range(cursor - KTextEditor::Cursor(0, oldAccess.length()), cursor);
        if(oldRange.start().column() >= 0 && textDocument->text(oldRange) == oldAccess) {
          textDocument->replaceText(oldRange, newAccess);
        }
      }
    }
  }
}

static MainThreadHelper s_mainThreadHelper;

}

namespace Cpp {

//Search for a copy-constructor within class
//*DUChain must be locked*
bool hasCopyConstructor(CppClassType::Ptr classType, TopDUContext* topContext)
{
  if(!classType)
    return false;
  Declaration* decl = classType->declaration(topContext);
  if(!decl)
    return false;
  DUContext* ctx = decl->internalContext();
  if(!ctx)
    return false;

  AbstractType::Ptr constClassType = classType->indexed().abstractType();
  constClassType->setModifiers(AbstractType::ConstModifier);
  ReferenceType::Ptr argumentType(new ReferenceType);
  argumentType->setBaseType(constClassType);

  QList<Declaration*> constructors = ctx->findLocalDeclarations(decl->identifier());
  foreach(Declaration* constructor, constructors) {
    FunctionType::Ptr funType = constructor->type<FunctionType>();
    if(funType && !funType->returnType() && funType->arguments().size() == 1) {
      if(funType->arguments()[0]->equals(argumentType.constData()))
        return true;
    }
  }

  return false;
}

///@todo move these together with those from expressionvisitor into an own file, or make them unnecessary
QList<Declaration*> declIdsToDeclPtrs( const QList<DeclarationId>& decls, uint count, TopDUContext* top ) {

  QList<Declaration*> ret;
  for(uint a = 0; a < count; ++a) {
    Declaration* d = decls[a].getDeclaration(top);
    if(d)
      ret << d;
  }

  return ret;
}

bool isLegalIdentifier( const QChar &theChar ) {
  return theChar.isLetterOrNumber() || theChar == '_';
}

///Gets the longest str from @param list which matches the ending of @param str
QString getEndingFromSet( const QString &str, const QSet<QString> &set, int maxMatchLen) {
  QString end;
  for ( int i = qMin(str.length(), maxMatchLen); i > 0; --i ) {
    end = str.right( i );
    if ( i + i < str.length() &&
         isLegalIdentifier( end[0] ) &&
         isLegalIdentifier( str[str.length()-i-1] ) )
      continue; //don't match ie, "varnamedelete[]"

    if ( set.contains( end ) )
      return end;
  }

  return QString();  
}

QString getEndFunctionOperator( const QString &str ) {
  QString ret = getEndingFromSet( str, BINARY_OPERATORS, BINARY_OPERATOR_MATCH );
  return ret == "[" ? "[]" : str;
}

//Gets rid of uneeded whitespace following a legal identifier
//"int i = " into "int i=" or "delete [ ] " into "delete[]"
void compressEndingWhitespace( QString &str ) {
  for (int i = str.length() - 1; i >= 0; --i) {
    if ( isLegalIdentifier( str[i] ) )
      return;
    if ( str[i].isSpace() )
      str.remove(i, 1);
  }
}

QString whitespaceFree(const QString &orig)
{
  QString ret = orig;
  for (int i = 0; i < ret.length(); ++i) {
    if ( ret[i].isSpace() )
      ret.remove(i, 1);
  }
  return ret;
}

bool isSignal( const QString &str )
{
  return str == "SIGNAL" || str == "Q_SIGNAL";
}
bool isSlot( const QString &str )
{
  return str == "SLOT" || str == "Q_SLOT";
}

QString lastNLines( const QString& str, int n ) {
  int curNewLine = str.lastIndexOf( '\n' );
  int nthLine = curNewLine;

  for ( int i = 0; i < n; ++i )
  {
    if ( curNewLine == -1 )
      break;
    else
      nthLine = curNewLine;

    curNewLine = str.lastIndexOf( '\n', curNewLine - 1 );
  }

  //return the position after the newline, or whole str if no newline
  return str.mid( nthLine + 1 );
}

bool skipToOpening( const QString& text, int &index)
{
  QChar closing = text[ index ];
  QChar opening;
  if ( closing == ')' )
    opening = '(';
  else if ( closing == '>' )
    opening = '<';
  else if ( closing == ']' )
    opening = '[';

  int count = 0;
  int start = index;
  while ( index >= 0 ) {
    QChar ch = text[ index ];
    --index;

    if ( ch == opening )
      ++count;
    else if ( ch == closing )
      --count;

    if ( count == 0 )
      return true;
  }

  index = start;
  return false;
}

/**
 * This function should search backwards in \p _text from \p index and return
 * the index where the expression begins (if there is one)
 * An expression is any legal identifier + member accesses + complete brackets
 * Examples (expression begins at "|"):
 * n = |(x + y)
 * n = |x(y, z)
 * n = x(|y
 * n = |x()->y[].x<>::
 * n = |x("whatever", ++y, x - u)
 * Notes: Doesn't know about keywords
**/
int expressionBefore( const QString& _text, int index )
{
  QString text = KDevelop::clearStrings( _text );
  bool lastWasIdentifier = false;

  --index;

  while ( index >= 0 )
  {
    while ( index >= 0 && text[ index ].isSpace() )
      --index;

    if ( index < 0 )
      break;

    QChar ch = text[ index ];
    QString memberAccess = getEndingFromSet( text.left ( index + 1 ),
                                             MEMBER_ACCESS_STRINGS,
                                             MEMBER_ACCESS_STR_MATCH );
    if ( !memberAccess.isEmpty() )
    {
      index -= memberAccess.length();
      lastWasIdentifier = false;
    }
    else if ( !lastWasIdentifier && isLegalIdentifier( ch ) )
    {
      while ( index >= 0 && isLegalIdentifier( text[ index ] ) )
        --index;
      lastWasIdentifier = true;
    }
    else if ( !lastWasIdentifier && ( ch == ')' || ch == '>' || ch == ']' ) )
    {
      if ( skipToOpening ( text, index ) )
        lastWasIdentifier = false;
      else
        break;
    }
    else
      break;
  }

  ++index;

  while ( index < text.length() && text[ index ].isSpace() )
    ++index;

  return index;
}

QString getUnaryOperator(const QString &context)
{
  QString unOp = getEndingFromSet( context, UNARY_OPERATORS, UNARY_OPERATOR_MATCH );
  QString binOp = getEndingFromSet( context, BINARY_OPERATORS, BINARY_OPERATOR_MATCH );
  if (!binOp.isEmpty()) {
    if (binOp == unOp) {
      int exprStart = expressionBefore(context, context.length() - binOp.length());
      QString exp = context.mid(exprStart, context.length() - exprStart - binOp.length()).trimmed();
      if ( !exp.isEmpty() && !KEYWORD_ACCESS_STRINGS.contains(exp) )
        return QString();
    }
    else if (binOp.contains(unOp)) //ie "&&"
      return QString();
  }
  return unOp;
}

//Returns the class or struct declaration for the given type
Declaration* containerDeclForType(const AbstractType::Ptr& givenType, TopDUContext* top, bool &typeIsPointer)
{
  if (PointerType::Ptr ptrType = givenType.cast<PointerType>())
  {
    if (!typeIsPointer)
    {
      typeIsPointer = true;
      return containerDeclForType(ptrType->baseType(), top, typeIsPointer);
    }
    else
      return 0; //The given type is a pointer to a pointer, and so cannot be accessed with ->
  }

  if (TypeAliasType::Ptr typeAliasType = givenType.cast<TypeAliasType>())
    return containerDeclForType(typeAliasType->type(), top, typeIsPointer);

  if (const IdentifiedType* identifiedType = dynamic_cast<const IdentifiedType*>(givenType.data()))
  {
    if (Declaration *ret = identifiedType->declaration(top))
    {
      if (dynamic_cast<ClassDeclaration*>(ret->logicalDeclaration(top)))
        return ret;
    }
  }

  return 0; //Type could not be identified or was not a class declaration
}

CodeCompletionContext::
CodeCompletionContext( KDevelop::DUContextPointer context, const QString& text,
                       const QString& followingText, const KDevelop::CursorInRevision& position,
                       int depth, const QStringList& knownArgumentExpressions, int line )
  : KDevelop::CodeCompletionContext( context, text, position, depth ),
    m_accessType( NoMemberAccess ),
    m_knownArgumentExpressions( knownArgumentExpressions ),
    m_isConstructorCompletion( false ),
    m_pointerConversionsBeforeMatching( 0 ),
    m_onlyShow( ShowAll ),
    m_expressionIsTypePrefix( false ),
    m_doAccessFiltering( DO_ACCESS_FILTERING )
{
  if ( doIncludeCompletion() )
    return;
  //We'll have to get a few expressionResults and do other DUChain processing during construction
  //so lock the DUChain here
  LOCKDUCHAIN;

  if( !m_duContext || depth > MAX_DEPTH || !isValidPosition() ) {
    m_valid = false;
    return;
  }

  m_followingText = followingText.trimmed();

  if( depth == 0 )
    preprocessText( line );
  m_text = lastNLines( m_text, CONTEXT_LINES );
  compressEndingWhitespace( m_text );

  if( doConstructorCompletion() )
    return;

  skipUnaryOperators( m_text, m_pointerConversionsBeforeMatching );

  QString accessStr = getEndingFromSet( m_text, ACCESS_STRINGS, ACCESS_STR_MATCH );
  m_accessType = findAccessType( accessStr );
  if ( m_depth > 0 || !PARENT_ACCESS_STRINGS.contains( accessStr ) )
    m_text.chop( accessStr.length() );

  QString expressionPrefix;
  findExpressionAndPrefix( m_expression, expressionPrefix, m_expressionIsTypePrefix );
  skipUnaryOperators( expressionPrefix, m_pointerConversionsBeforeMatching );

  m_localClass = findLocalClass();
  m_parentContext = getParentContext( expressionPrefix );

  if ( doSignalSlotCompletion() )
    return;

  m_onlyShow = findOnlyShow( accessStr );
  m_expressionResult = evaluateExpression();

  m_valid = testContextValidity(expressionPrefix, accessStr);
  if (!m_valid)
    return;

  if ( m_accessType == TemplateAccess ||
       m_accessType == FunctionCallAccess ||
       m_accessType == BinaryOpFunctionCallAccess )
  {
    m_knownArgumentTypes = getKnownArgumentTypes();

    if ( m_accessType == BinaryOpFunctionCallAccess )
      m_operator = getEndFunctionOperator( accessStr );
 
    if( !m_expression.isEmpty() && !m_expressionResult.isValid() )
      m_functionName = m_expression; //set m_functionName for Missing Include Completion
  }

  switch( m_accessType ) {
    case ArrowMemberAccess:
      processArrowMemberAccess();
      //Falls through to processAllMemberAccesses, but only needs the missing include part TODO: refactor
    case MemberChoose:
    case StaticMemberChoose:
    case MemberAccess:
      processAllMemberAccesses();
      break;
    case BinaryOpFunctionCallAccess:
    case FunctionCallAccess:
      processFunctionCallAccess();
      break;
    default:
      //Nothing to do for now
    break;
  }
}

void CodeCompletionContext::processAllMemberAccesses() {
  AbstractType::Ptr type = m_expressionResult.type.abstractType();
  if(!type)
    return;

  if(type.cast<PointerType>())
    replaceCurrentAccess( ".", "->" );
}

void CodeCompletionContext::processArrowMemberAccess() {
  //Dereference a pointer
  AbstractType::Ptr containerType = m_expressionResult.type.abstractType();
  PointerType::Ptr pnt = TypeUtils::realType( containerType, m_duContext->topContext() ).cast<PointerType>();
  if( pnt ) {
    ///@todo what about const in pointer?
    m_expressionResult.type = pnt->baseType()->indexed();
    m_expressionResult.isInstance = true;
    return; // expression is a pointer
  }

  //Look for "->" operator
  AbstractType::Ptr realContainer = TypeUtils::realType( containerType, m_duContext->topContext() );
  IdentifiedType* idType = dynamic_cast<IdentifiedType*>( realContainer.data() );
  if ( !idType ) {
    m_valid = false;
    return;
  }

  Declaration* idDecl = idType->declaration(m_duContext->topContext());
  if( !idDecl || !idDecl->internalContext() ) {
    m_valid = false;
    return;
  }

  QList<Declaration*> operatorDeclarations =
      Cpp::findLocalDeclarations( idDecl->internalContext(),
                                  Identifier( "operator->" ),
                                  m_duContext->topContext() );
  if( operatorDeclarations.isEmpty() ) {
    if( idDecl->internalContext()->type() == DUContext::Class )
      replaceCurrentAccess( "->", "." );
    m_valid = false;
    return;
  }

  // TODO use Cpp::isAccessible on operator functions for more correctness?
  foreach( Declaration* decl, operatorDeclarations )
    m_expressionResult.allDeclarationsList().append( decl->id() );

  bool declarationIsConst = ( containerType->modifiers() & AbstractType::ConstModifier ) ||
                            ( idDecl->abstractType()->modifiers() & AbstractType::ConstModifier );
  FunctionType::Ptr function;
  foreach ( Declaration* decl, operatorDeclarations ) {
    FunctionType::Ptr f2 = decl->abstractType().cast<FunctionType>();
    const bool operatorIsConst = f2->modifiers() & AbstractType::ConstModifier;
    if ( operatorIsConst == declarationIsConst ) {
      // Best match
      function = f2;
      break;
    } else if ( operatorIsConst && !function ) {
      // Const result where non-const is ok, accept and keep looking
      function = f2;
    }
  }

  if ( !function ) {
    m_valid = false;
    return; //const declaration has no non-const "operator->"
  }

  m_expressionResult.type = function->returnType()->indexed();
  m_expressionResult.isInstance = true;
}

bool CodeCompletionContext::testContextValidity(const QString &expressionPrefix, const QString &accessStr) const {
  if( !m_expression.isEmpty() && !m_expressionResult.isValid() ) {
    //StaticMemberChoose may be an access to a namespace, like "MyNamespace::".
    //"MyNamespace" cannot be evaluated, still we can give some completions
    //FunctionCallAccess & TemplateAccess can still get missing include completion
    if( m_accessType != FunctionCallAccess &&
        m_accessType != TemplateAccess &&
        m_accessType != StaticMemberChoose )
      return false;
  }
  //Special case for "class" access str, which should only have completions when it is "friend class ..."
  if (accessStr == "class" && !expressionPrefix.endsWith("friend"))
    return false;

  switch ( m_accessType )
  {
    case NoMemberAccess:
      return m_expression.isEmpty() || isImplementationHelperValid();
    case BinaryOpFunctionCallAccess:
      return m_expressionResult.isInstance;
    case MemberAccess:
    case ArrowMemberAccess:
      if (!m_expressionResult.isInstance)
        return false;
      //Fall-through
    case MemberChoose:
    case StaticMemberChoose:
      return !m_expression.isEmpty();
    default:
      return true;
  }
}

DUContextPointer CodeCompletionContext::findLocalClass() const {
  Declaration* classDecl = Cpp::localClassFromCodeContext( m_duContext.data() );
  return classDecl ? DUContextPointer( classDecl->internalContext() ) : DUContextPointer();
}

KDevelop::CodeCompletionContext::Ptr
CodeCompletionContext::getParentContext( const QString &expressionPrefix ) const {
  //this is essentially a poor-mans tokenizer, and we want to find out
  //whether the last token is part of PARENT_ACCESS_STRINGS
  //but we must take into account that longer versions exist in ACCESS_STRINGS,
  //esp. for e.g. "parent:", here ":" would be a PARENT_ACCESS_STRINGS but
  //it is actually not. So we first search in the long version and then
  //double-check that it's actually a proper access string
  QString access = getEndingFromSet( expressionPrefix, ACCESS_STRINGS, ACCESS_STR_MATCH );
  if ( access.isEmpty() || !PARENT_ACCESS_STRINGS.contains(access) )
    return KDevelop::CodeCompletionContext::Ptr();

  QStringList previousArguments;
  QString parentContextText;

  if ( access == "," ) {
    //Get arguments before current position
    int parentContextEnd = expressionPrefix.length();
    skipFunctionArguments( expressionPrefix, previousArguments, parentContextEnd );
    parentContextText = expressionPrefix.left( parentContextEnd );
  }
  else
    parentContextText = expressionPrefix;

  if( m_depth == 0 || parentContextText != m_text )
    return KDevelop::CodeCompletionContext::Ptr(
        new CodeCompletionContext( m_duContext, parentContextText, QString(),
                                   m_position, m_depth + 1, previousArguments ) );

  return KDevelop::CodeCompletionContext::Ptr();
}

void CodeCompletionContext::skipUnaryOperators(QString &str, int &pointerConversions) const {
  ///Eventually take preceding "*" and/or "&" operators and use them for pointer depth conversion of completion items
  if ( str.endsWith("new") )
    pointerConversions = 1;

  QString unOp = getUnaryOperator( str );
  while ( !unOp.isEmpty() ) {
    unOp = getUnaryOperator( str );

    if ( unOp == "&" )
      ++pointerConversions;
    else if ( unOp == "*" )
      --pointerConversions;

    str.chop(unOp.length());
  }
}

bool CodeCompletionContext::doSignalSlotCompletion() {
  if( m_depth > 0 || !parentContext() ||
      parentContext()->accessType() != FunctionCallAccess)
    return false;

  //TODO: support "char* sig = SIGNAL(" properly
  if( isSignal( parentContext()->m_expression ) || isSlot( parentContext()->m_expression ) ) {
    m_onlyShow = isSlot(parentContext()->m_expression) ? ShowSlots : ShowSignals;
    //If we are in "SIGNAL(" or "SLOT(" context, skip it
    setParentContext(KDevelop::CodeCompletionContext::Ptr(parentContext()->parentContext()));
  }

  if( !parentContext() || !m_expression.isEmpty() ||
      parentContext()->accessType() != FunctionCallAccess )
    return false;

  //Check if we're in a connect/disconnect function, and at what param
  foreach( const Cpp::OverloadResolutionFunction &function, parentContext()->functions() )
  {
    DeclarationPointer decl = function.function.declaration();
    if( !decl ||
        ( decl->qualifiedIdentifier().toString() != "QObject::connect" &&
          decl->qualifiedIdentifier().toString() != "QObject::disconnect") )
      continue; //Not a connect/disconnect function

    FunctionType::Ptr funType = decl->type<FunctionType>();
    if( !funType || funType->arguments().size() <= function.matchedArguments ||
        funType->arguments().size() < 3 )
      continue; //Not a recognized overload

    //this is a connect/disconnect, find if at SIGNAL or SLOT param
    if( function.matchedArguments == 1 && parentContext()->m_knownArgumentTypes.size() >= 1 ) {
      //currently at signal param
      m_accessType = SignalAccess;
    }
    else if( funType->arguments()[function.matchedArguments] &&
             funType->arguments()[function.matchedArguments]->toString() == "const char*" )
    {
      //currently at slot param
      m_accessType = SlotAccess;

      //get the corresponding signal's identifier and signature
      if( parentContext()->m_knownArgumentExpressions.size() > 1 ) {
        QString connectedSignal = parentContext()->m_knownArgumentExpressions[1];

        int skipSignal = 0;
        if( connectedSignal.startsWith( "SIGNAL(") )
          skipSignal = 7;
        if( connectedSignal.startsWith( "Q_SIGNAL(") )
          skipSignal = 9;

        if( skipSignal && connectedSignal.endsWith( ")" ) &&
            connectedSignal.length() > skipSignal + 1 )
        {
          connectedSignal = connectedSignal.mid( skipSignal );
          connectedSignal = connectedSignal.left( connectedSignal.length() - 1 );
          //Now connectedSignal is something like myFunction(...), and we want the "...".
          QPair<Identifier, QByteArray> signature = Cpp::qtFunctionSignature( connectedSignal.toUtf8() );
          m_connectedSignalIdentifier = signature.first;
          m_connectedSignalNormalizedSignature = signature.second;
        }
      }
    }

    if( m_accessType == SignalAccess || m_accessType == SlotAccess ) {
      if( function.matchedArguments == 2 ) {
        //The function that does not take the target-argument is being used
        if( Declaration* klass = Cpp::localClassFromCodeContext( m_duContext.data() ) )
          m_expressionResult.type = klass->indexedType();
      }
      else if( parentContext()->m_knownArgumentTypes.size() >=
               function.matchedArguments && function.matchedArguments != 0 )
      {
        m_expressionResult = parentContext()->m_knownArgumentTypes[function.matchedArguments-1];
        m_expressionResult.type = TypeUtils::targetType(TypeUtils::matchingClassPointer(funType->arguments()[function.matchedArguments-1], m_expressionResult.type.abstractType(), m_duContext->topContext()), m_duContext->topContext())->indexed();
      }

      return true;
    }
  }

  return false;
}

ExpressionEvaluationResult CodeCompletionContext::evaluateExpression() const {
  if( m_expression.isEmpty() )
    return ExpressionEvaluationResult();

  ExpressionParser expressionParser;

  if( !m_expressionIsTypePrefix && m_accessType != NoMemberAccess )
    return expressionParser.evaluateExpression( m_expression.toUtf8(), m_duContext );

  ExpressionEvaluationResult res = expressionParser.evaluateType( m_expression.toUtf8(), m_duContext );
  res.isInstance = true;
  return res;
}

bool CodeCompletionContext::doConstructorCompletion() {
  QString text = m_text.trimmed();

  QStringList hadItems;

  ifDebug( kDebug() << "text:" << text; )

  //Jump over all initializers
  while(!text.isEmpty() && text.endsWith(',')) {
    text = text.left(text.length()-1).trimmed();
    //Skip initializer expression
    int start_expr = expressionBefore( text, text.length() );
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
  
  int identifierStart = expressionBefore( text, argumentsStart-1 );
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
  m_accessType = MemberAccess;
  m_doAccessFiltering = false;
  
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

CodeCompletionContext::AccessType CodeCompletionContext::findAccessType( const QString &accessStr ) const {
  if( accessStr == "." )
    return MemberAccess;

  if( accessStr == "->" )
    return ArrowMemberAccess;

  //TODO: add support for MemberChoose
  if( accessStr == "::" )
    return StaticMemberChoose;

  if ( accessStr == "namespace" )
    return NamespaceAccess;

  if ( m_depth > 0 )
  {
    if( accessStr == "(" )
      return FunctionCallAccess;

    if (accessStr == "<" ) {
      //We need to check here whether this really is a template access, or whether
      //it is a "less than" operator, which is a BinaryOpFunctionCallAccess
      int start_expr = expressionBefore( m_text, m_text.length()-1 );

      QString expr = m_text.mid(start_expr, m_text.length() - start_expr - 1).trimmed();

      ExpressionParser expressionParser;
      Cpp::ExpressionEvaluationResult result =
          expressionParser.evaluateExpression(expr.toUtf8(), m_duContext);
      if( result.isValid() &&
          ( !result.isInstance || result.type.type<FunctionType>() ) &&
          !result.type.type<DelayedType>() )
        return TemplateAccess;
    }

    if ( accessStr == "return" )
      return ReturnAccess;

    if ( accessStr == "case" )
      return CaseAccess;

    if( BINARY_OPERATORS.contains( accessStr ) )
      return BinaryOpFunctionCallAccess;
  }

  return NoMemberAccess;
}

void CodeCompletionContext::
findExpressionAndPrefix(QString& expression, QString& expressionPrefix, bool &isTypePrefix) const {
  int start_expr;
  start_expr = expressionBefore( m_text, m_text.length() );
  expression = m_text.mid( start_expr ).trimmed();

  if ( KEYWORD_ACCESS_STRINGS.contains( expression ) ) {
    expression = QString();
    start_expr = -1;
  }

  expressionPrefix = m_text.left(start_expr).trimmed();
  compressEndingWhitespace( expressionPrefix );

  if ( expressionPrefix.isEmpty() )
    return;

  ///handle "Type instance(" or "Type instance =". The "Type" part will be in the prefix
  if( expressionPrefix.endsWith('>') || expressionPrefix.endsWith('*') ||
      isLegalIdentifier( expressionPrefix[expressionPrefix.length()-1] ) ) {

    int  ptrs = 0;
    while ( expressionPrefix.endsWith( QString( "*" ).repeated( ptrs + 1 ) ) )
      ++ptrs;
    int newExpressionStart = expressionBefore(expressionPrefix, expressionPrefix.length() - ptrs);
    QString newExpression = expressionPrefix.mid( newExpressionStart ).trimmed();

    //Make sure it's not picking up something like "if (a < a > b)"
    ExpressionParser expressionParser;
    ExpressionEvaluationResult res = expressionParser.evaluateType( newExpression.toUtf8(), m_duContext );

    //must use toString() comparison because sometimes isInstance is wrong (ie "var*", "new", "") TODO: fix
    if ( res.isValid() && !res.isInstance && whitespaceFree( res.toString() ) == whitespaceFree( newExpression ) ) {
      expressionPrefix = expressionPrefix.left( newExpressionStart );
      compressEndingWhitespace( expressionPrefix );
      expression = newExpression;
      isTypePrefix = true;
      return;
    }
  }

  //Add reference and dereference operators to expression
  QString op; 
  while ( true ) {
    op = getUnaryOperator(expressionPrefix);
    if (op == "*" || op == "&") {
      expression.prepend(op);
      expressionPrefix.chop(op.length());
    } else
      break;
  }
}

QList< ExpressionEvaluationResult > CodeCompletionContext::getKnownArgumentTypes() const {
  ExpressionParser expressionParser;
  QList< ExpressionEvaluationResult > expressionResults;
  for( QStringList::const_iterator it = m_knownArgumentExpressions.constBegin();
       it != m_knownArgumentExpressions.constEnd(); ++it ) {
    expressionResults << expressionParser.evaluateExpression( (*it).toUtf8(), m_duContext );
  }

  return expressionResults;
}

CodeCompletionContext::OnlyShow CodeCompletionContext::findOnlyShow(const QString &accessStr) const {
  //TODO: ShowSignals/Slots doesn't work at all outside of connect/disconnect,
  //but should be used for ie "const char * x = SIGNAL("
  //TODO: Should only show types for a SHOW_TYPES_ACCESS_STRINGS in expressionPrefix
  //(at least for StaticMemberChoose)

  //Either there's no expression, which means Global completion,
  //or there is an expression, which means implementationhelperitems only
  if ( m_accessType == NoMemberAccess && !m_expression.isEmpty() &&
       isImplementationHelperValid() )
    return ShowImplementationHelpers;

  if( SHOW_TYPES_ACCESS_STRINGS.contains( accessStr ) )
    return ShowTypes;

  if ( parentContext() && parentContext()->accessType() == TemplateAccess )
    return ShowTypes;

  if ( parentContext() && parentContext()->accessType() == CaseAccess )
    return ShowIntegralConstants;

  //Only ShowTypes in these DUContexts unless initializing a declaration
  //ie, m_expressionIsTypePrefix == true
  if (m_duContext->type() == DUContext::Class ||
      m_duContext->type() == DUContext::Namespace ||
      m_duContext->type() == DUContext::Global )
  {
    CodeCompletionContext* ctxt = parentContext();
    while (ctxt && !ctxt->m_expressionIsTypePrefix)
      ctxt = ctxt->parentContext();

    if ( !ctxt && !m_expressionIsTypePrefix )
      return ShowTypes;
  }

  return ShowAll;
}

QList< Cpp::ExpressionEvaluationResult > CodeCompletionContext::knownArgumentTypes() const {
  return m_knownArgumentTypes;
}

bool CodeCompletionContext::isConstructorInitialization() {
  return m_isConstructorCompletion;
}

void CodeCompletionContext::processFunctionCallAccess() {
  ///Generate a list of all found functions/operators, together with each a list of optional prefixed parameters
  ///All the variable argument-count management in the following code is done to treat global operator-functions equivalently to local ones. Those take an additional first argument.

  OverloadResolutionHelper helper( m_duContext, TopDUContextPointer(m_duContext->topContext()) );

  if( m_accessType == BinaryOpFunctionCallAccess ) {

    helper.setOperator(OverloadResolver::Parameter(m_expressionResult.type.abstractType(), m_expressionResult.isLValue()));

    m_functionName = "operator"+m_operator;
    
  } else {
    ///Simply take all the declarations that were found by the expression-parser

    helper.setFunctions(declIdsToDeclPtrs(m_expressionResult.allDeclarations, m_expressionResult.allDeclarationsSize(), m_duContext->topContext()));

    if(m_expressionResult.allDeclarationsSize()) {
      Declaration* decl = m_expressionResult.allDeclarations[0].getDeclaration(m_duContext->topContext());
      if(decl)
        m_functionName = decl->identifier().toString();
    }
  }

  if( m_accessType == BinaryOpFunctionCallAccess || m_expression == m_functionName )
    helper.setFunctionNameForADL( QualifiedIdentifier(m_functionName) );
  
  OverloadResolver::ParameterList knownParameters;
  foreach( const ExpressionEvaluationResult &result, m_knownArgumentTypes )
    knownParameters.parameters << OverloadResolver::Parameter( result.type.abstractType(), result.isLValue() );

  helper.setKnownParameters(knownParameters);

  m_matchingFunctionOverloads = helper.resolveToList(true);

  if(m_accessType == BinaryOpFunctionCallAccess) {
    //Filter away all global binary operators that do not have the first argument matched
    QList< Function > oldFunctions = m_matchingFunctionOverloads;
    m_matchingFunctionOverloads.clear();
    foreach(const Function& f, oldFunctions) {
      if(f.matchedArguments == 1 && !f.function.isViable())
        continue;
      else
        m_matchingFunctionOverloads << f;
    }
  }
}

bool CodeCompletionContext::doIncludeCompletion()
{
  QString line = lastNLines(m_text, 1).trimmed();
  if(!line.startsWith("#"))
    return false;

  m_accessType = IncludeListAccess;
  if(line.count('"') == 2 || line.endsWith('>'))
    return true; //We are behind a complete include-directive

  int endOfInclude = CppUtils::findEndOfInclude(line);
  if(endOfInclude == -1)
    return true;

  //Strip away #include
  line = line.mid(endOfInclude).trimmed();
  
  kDebug(9007) << "trimmed include line: " << line;

  if(!line.startsWith('<') && !line.startsWith('"'))
    return true; //We are not behind the beginning of a path-specification

  const bool local = line.startsWith('"');
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

#ifndef TEST_COMPLETION
  m_includeItems = CppUtils::allFilesInIncludePath(m_duContext->url().str(), local, prefixPath);
#else
  Q_UNUSED(local);
#endif

  return true;
}

const CodeCompletionContext::FunctionList& CodeCompletionContext::functions() const {
  return m_matchingFunctionOverloads;
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

QSet<DUContext*> CodeCompletionContext::memberAccessContainers() const {
  QSet<DUContext*> ret;

  if( m_accessType == StaticMemberChoose && m_duContext ) {
    //Locate all namespace-instances we will be completing from
    QList< Declaration* > decls = m_duContext->findDeclarations(QualifiedIdentifier(m_expression)); ///@todo respect position

    // qlist does not provide convenient stable iterators
    std::list<Declaration*> worklist(decls.begin(), decls.end());
    for (std::list<Declaration*>::iterator it = worklist.begin(); it != worklist.end(); ++it) {
      Declaration * decl = *it;
      if((decl->kind() == Declaration::Namespace || dynamic_cast<ClassDeclaration*>(decl))  && decl->internalContext())
        ret.insert(decl->internalContext());
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
    const IdentifiedType* idType = dynamic_cast<const IdentifiedType*>( expressionTarget.data() );
      Declaration* idDecl = 0;
    if( idType && (idDecl = idType->declaration(m_duContext->topContext())) ) {
      DUContext* ctx = idDecl->logicalInternalContext(m_duContext->topContext());
      if( ctx ){
        if(ctx->type() != DUContext::Template) //Forward-declared template classes have a template-context assigned. Those should not be searched.
          ret.insert(ctx);
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
//     kDebug() << "member-access container:" << context->url().str() << context->range() << context->scopeIdentifier(true).toString();
//   }

  return ret;
}

int CodeCompletionContext::pointerConversions() const
{
  return m_pointerConversionsBeforeMatching;
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


bool CodeCompletionContext::isImplementationHelperValid() const
{
  if (m_onlyShow == ShowVariables || m_isConstructorCompletion)
    return false;
  if (m_accessType != NoMemberAccess && m_accessType != StaticMemberChoose)
    return false;

  LOCKDUCHAIN;
  if (!m_duContext)
    return false;

  return ( !parentContext() && ( m_duContext->type() == DUContext::Namespace ||
                                 m_duContext->type() == DUContext::Global) );
}

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
  QSet<IndexedString> disableMacros;
  disableMacros.insert(IndexedString("SIGNAL"));
  disableMacros.insert(IndexedString("SLOT"));
  disableMacros.insert(IndexedString("emit"));
  disableMacros.insert(IndexedString("Q_EMIT"));
  disableMacros.insert(IndexedString("Q_SIGNAL"));
  disableMacros.insert(IndexedString("Q_SLOT"));

  // Use the proxy-context if possible, because that one contains most of the macros if existent
  TopDUContext* useTopContext = proxyContextForUrl(m_duContext->url().toUrl());
  if(!useTopContext)
    useTopContext = m_duContext->topContext();

  m_text = preprocess( m_text, dynamic_cast<Cpp::EnvironmentFile*>(useTopContext->parsingEnvironmentFile().data()), line, disableMacros );

  m_text = clearComments( m_text );
}

CodeCompletionContext::AccessType CodeCompletionContext::accessType() const {
  return m_accessType;
}

CodeCompletionContext* CodeCompletionContext::parentContext() const {
  return static_cast<CodeCompletionContext*>(m_parentContext.data());
}

void getOverridable(DUContext* base, DUContext* current, QMap< QPair<IndexedType, IndexedString>, KDevelop::CompletionTreeItemPointer >& overridable, CodeCompletionContext::Ptr completionContext, int depth = 0) {
  if(!current)
    return;
  
  foreach(Declaration* decl, current->localDeclarations()) {
    ClassFunctionDeclaration* classFun = dynamic_cast<ClassFunctionDeclaration*>(decl);
    // one can only override the direct parent's ctor
    if(classFun && (classFun->isVirtual() || (depth == 0 && classFun->isConstructor())) && !classFun->isExplicitlyDeleted()) {
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

QList< QExplicitlySharedDataPointer< KDevelop::CompletionTreeElement > > CodeCompletionContext::ungroupedElements() {
  return m_storedUngroupedItems;
}

QList<CompletionTreeItemPointer> CodeCompletionContext::memberAccessCompletionItems( const bool& shouldAbort )
{
  QList<CompletionTreeItemPointer> items;
  LOCKDUCHAIN; if (!m_duContext) return items;

  if( !memberAccessContainer().isValid() && m_accessType != StaticMemberChoose )
    return items;

  bool typeIsConst = false;
  AbstractType::Ptr expressionTarget = TypeUtils::targetType(m_expressionResult.type.abstractType(), m_duContext->topContext());
  if (expressionTarget && (expressionTarget->modifiers() & AbstractType::ConstModifier))
    typeIsConst = true;

  QSet<DUContext*> containers = memberAccessContainers();
  ifDebug( kDebug() << "got" << containers.size() << "member-access containers"; )
  if (containers.isEmpty())
  {
    ifDebug( kDebug() << "missing-include completion for" << m_expression << m_expressionResult.toString(); )
    lock.unlock();
    eventuallyAddGroup(i18n("Not Included"), 700, missingIncludeCompletionItems(m_expression, QString(), m_expressionResult, m_duContext, 0, true ));
  }

  //Used to show only one namespace-declaration per namespace
  QSet<QualifiedIdentifier> hadNamespaceDeclarations;

  foreach(DUContext* ctx, containers) {
    if (shouldAbort)
      return items;
    ifDebug( kDebug() << "container:" << ctx->scopeIdentifier(true).toString(); )

    QList<DeclarationDepthPair> decls = ctx->allDeclarations(ctx->range().end, m_duContext->topContext(), false );
    decls += namespaceItems(ctx, ctx->range().end, false, containers);

    foreach( const DeclarationDepthPair& decl, Cpp::hideOverloadedDeclarations(decls, typeIsConst ) )
    {
      //If we have StaticMemberChoose, which means A::Bla, show only static members, except if we're within a class that derives from the container
      ClassMemberDeclaration* classMember = dynamic_cast<ClassMemberDeclaration*>(decl.first);

      if(classMember && !filterDeclaration(classMember, ctx))
        continue;
      else if(!filterDeclaration(decl.first, ctx))
        continue;

      if (accessType() == MemberAccess || accessType() == ArrowMemberAccess) {
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

      if(accessType() != Cpp::CodeCompletionContext::StaticMemberChoose) {
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

  return items;
}

AbstractType::Ptr functionReturnType(DUContext* startContext)
{
  while(startContext && !startContext->owner())
    startContext = startContext->parentContext();
  if(startContext && startContext->owner())
  {
    FunctionType::Ptr funType = startContext->owner()->type<FunctionType>();
    if (funType && funType->returnType())
      return funType->returnType();
  }
  return AbstractType::Ptr();
}

QList<CompletionTreeItemPointer> CodeCompletionContext::returnAccessCompletionItems()
{
  QList<CompletionTreeItemPointer> items;
  LOCKDUCHAIN; if (!m_duContext) return items;

  AbstractType::Ptr returnType = functionReturnType(m_duContext.data());
  if (returnType)
    items << CompletionTreeItemPointer( new TypeConversionCompletionItem( "return " + returnType->toString(), returnType->indexed(), depth(), Cpp::CodeCompletionContext::Ptr(this) ) );
  return items;
}

QList<CompletionTreeItemPointer> CodeCompletionContext::caseAccessCompletionItems()
{
  QList<CompletionTreeItemPointer> items;

  {
#ifndef TEST_COMPLETION
    // Test thread has DUChain locked
    // This is essential here - a ForegroundLock must be acquired _before_ locking DUChain
    ENSURE_CHAIN_NOT_LOCKED;
#endif

    ForegroundLock foregroundLock;
    LOCKDUCHAIN;

    if( m_duContext && m_duContext->importedParentContexts().size() == 1 )
    {
      DUContext* switchContext = m_duContext->importedParentContexts().first().context( m_duContext->topContext() );
      ExpressionParser expressionParser;
      m_expression = switchContext->createRangeMoving()->text();
      m_expressionResult = expressionParser.evaluateExpression( m_expression.toUtf8(), DUContextPointer( switchContext ) );
    }
  }

  IndexedType switchExprType = m_expressionResult.type;

  LOCKDUCHAIN; if (!m_duContext) return items;

  if (switchExprType.abstractType())
    items << CompletionTreeItemPointer( new TypeConversionCompletionItem( "case " + switchExprType.abstractType()->toString(), switchExprType, depth(), Cpp::CodeCompletionContext::Ptr(this) ) );
  return items;
}

QList<CompletionTreeItemPointer> CodeCompletionContext::templateAccessCompletionItems()
{
  QList<CompletionTreeItemPointer> items;
  LOCKDUCHAIN; if (!m_duContext) return items;

  AbstractType::Ptr type = m_expressionResult.type.abstractType();
  IdentifiedType* identified = dynamic_cast<IdentifiedType*>(type.data());
  Declaration* decl = 0;
  if(identified)
    decl = identified->declaration( m_duContext->topContext());
  if(!decl && !m_expressionResult.allDeclarations.isEmpty())
    decl = m_expressionResult.allDeclarations[0].getDeclaration(m_duContext->topContext());
  if(decl) {
    NormalDeclarationCompletionItem* item = new NormalDeclarationCompletionItem( KDevelop::DeclarationPointer(decl),  KDevelop::CodeCompletionContext::Ptr(this), 0, 0 );
    item->m_isTemplateCompletion = true;
    items << CompletionTreeItemPointer( item );
  }else{
    lock.unlock();
    items += missingIncludeCompletionItems(m_expression, QString(), m_expressionResult, m_duContext, depth(), true );
  }
  return items;
}

QList<CompletionTreeItemPointer> CodeCompletionContext::commonFunctionAccessCompletionItems( bool fullCompletion )
{
  QList<CompletionTreeItemPointer> items;

  uint max = MoreArgumentHintsCompletionItem::resetMaxArgumentHints(!fullCompletion);

  if(functions().isEmpty() && m_accessType != BinaryOpFunctionCallAccess) {
    items += missingIncludeCompletionItems(m_expression, QString(), m_expressionResult, m_duContext, depth(), true );
    return items;
  }

  LOCKDUCHAIN; if (!m_duContext) return items;

  uint num = 0;
  foreach( const Cpp::CodeCompletionContext::Function &function, functions() ) {
    if (num == max) {
      //When there are too many overloaded functions, do not show them all
      CompletionTreeItemPointer item( new MoreArgumentHintsCompletionItem( KDevelop::CodeCompletionContext::Ptr(this), i18ncp("Here, overload is used as a programming term.  This string is used to display how many overloaded versions there are of the function whose name is the second argument.", "1 more overload of %2 (show more)", "%1 more overloads of %2 (show more)", functions().count() - num, functionName()), num ) );
      items.push_front(item);
      break;
    }

    items << CompletionTreeItemPointer( new NormalDeclarationCompletionItem( function.function.declaration(), KDevelop::CodeCompletionContext::Ptr(this), 0, num ) );
    ++num;
  }

  return items;
}

QList< CompletionTreeItemPointer > CodeCompletionContext::binaryFunctionAccessCompletionItems( bool fullCompletion )
{
  QList<CompletionTreeItemPointer> items;

  items += commonFunctionAccessCompletionItems(fullCompletion);

  LOCKDUCHAIN; if (!m_duContext) return items;

  //Argument-hints for builtin operators
  AbstractType::Ptr type = m_expressionResult.type.abstractType();
  if(!m_expressionResult.isValid() || !m_expressionResult.isInstance || !type)
    return items;

  IntegralType::Ptr integral = type.cast<IntegralType>();

  if(!integral && (ARITHMETIC_COMPARISON_OPERATORS.contains(m_operator) || BINARY_ARITHMETIC_OPERATORS.contains(m_operator))) {
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
    items << CompletionTreeItemPointer( new TypeConversionCompletionItem( showName, t->indexed(), depth(), Cpp::CodeCompletionContext::Ptr(this) ) );
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
    items << CompletionTreeItemPointer( new TypeConversionCompletionItem( showName, useType, depth(), Cpp::CodeCompletionContext::Ptr(this) ) );
  }

  return items;
}

QList<CompletionTreeItemPointer> CodeCompletionContext::functionAccessCompletionItems(bool fullCompletion)
{
  QList<CompletionTreeItemPointer> items;

  items += commonFunctionAccessCompletionItems(fullCompletion);

  LOCKDUCHAIN; if (!m_duContext) return items;

  if(!m_expressionResult.isValid() ||
     !m_expressionResult.type.abstractType() ||
     (m_expressionResult.isInstance && !m_expressionIsTypePrefix) ||
     m_expressionResult.type.type<FunctionType>())
    return items;

  //Eventually add a builtin copy-constructor if a type is being constructed
  if(!hasCopyConstructor(m_expressionResult.type.type<CppClassType>(), m_duContext->topContext()) &&
      m_knownArgumentExpressions.isEmpty())
  {
      QString showName = m_expressionResult.type.abstractType()->toString() + "(";
      items << CompletionTreeItemPointer( new TypeConversionCompletionItem( showName, m_expressionResult.type, depth(), Cpp::CodeCompletionContext::Ptr(this) ) );
  }

  return items;
}

QList<CompletionTreeItemPointer> CodeCompletionContext::includeListAccessCompletionItems(const bool& shouldAbort)
{
  QList<CompletionTreeItemPointer> items;

  QList<KDevelop::IncludeItem> allIncludeItems = includeItems();
  foreach(const KDevelop::IncludeItem& includeItem, allIncludeItems) {
    if (shouldAbort)
      return items;

    items << CompletionTreeItemPointer( new IncludeFileCompletionItem(includeItem) );
  }

  return items;
}

QList<CompletionTreeItemPointer> CodeCompletionContext::signalSlotAccessCompletionItems()
{
  QList<CompletionTreeItemPointer> items;
  LOCKDUCHAIN; if (!m_duContext) return items;

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
    IdentifiedType* identified = dynamic_cast<IdentifiedType*>(type.data());
    if(identified) {
      Declaration* decl = identified->declaration(m_duContext->topContext());
      if(decl && decl->internalContext() /*&& Cpp::findLocalDeclarations(decl->internalContext(), Identifier("QObject"), m_duContext->topContext()).count()*/) { //hacky test whether it's a QObject
        ///@todo Always allow this when the class is within one of the open projects. Problem: The project lookup is not threadsafe
        if(connectedSignal.isValid() && m_localClass.data() == decl->internalContext()) { ///Create implementation-helper to add a slot
          signalSlots << CompletionTreeItemPointer(new ImplementationHelperItem(ImplementationHelperItem::CreateSignalSlot, DeclarationPointer(connectedSignal.data()), CodeCompletionContext::Ptr(this)));
        }

        foreach(const DeclarationDepthPair &candidate, decl->internalContext()->allDeclarations(CursorInRevision::invalid(), m_duContext->topContext(), false) ) {
          if(QtFunctionDeclaration* classFun = dynamic_cast<QtFunctionDeclaration*>(candidate.first)) {
            if((classFun->isSignal() && m_onlyShow != ShowSlots) || (accessType() == SlotAccess && classFun->isSlot() && filterDeclaration(classFun))) {
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
  return items;
}

QList<IndexedType> CodeCompletionContext::matchTypes()
{
  if (!m_cachedMatchTypes.isEmpty()) {
    return m_cachedMatchTypes;
  }

  QSet<KDevelop::IndexedType> ret;
  switch(m_accessType)
  {
  case BinaryOpFunctionCallAccess:
  case FunctionCallAccess:
  {
    //MatchTypes for custom operator functions
    foreach(const Function &func, m_matchingFunctionOverloads)
    {
      if (!func.function.isValid() || !func.function.isViable() || !func.function.declaration())
        continue;
      FunctionType::Ptr funcType = func.function.declaration()->type<FunctionType>();
      if(funcType && funcType->indexedArgumentsSize() > (uint)func.matchedArguments)
        ret << funcType->indexedArguments()[func.matchedArguments];
    }
    /* generally matching the other side's type is only useful for MATCH_TYPE_OPERATORS ... Consider:
     * if (foo && foo == "str" || <complete here>)
     * The expressionResult preceeding the "||" operator is ["str"] and not [foo == "str"] as it should be
     * therefore the matchType will be "const char *" and not "boolean" as it should be
     * If this is fixed, we could use more operators here. See expressionBefore().
     */
    if ( !m_matchingFunctionOverloads.size() && MATCH_TYPE_OPERATORS.contains(m_operator) )
      ret << m_expressionResult.type;

    /* A hack: constructor initialization currently is FunctionCallAccess, although obviously the
     * expression is not a function. Still, we want the match type here, so we check the parents
     * of this parent and if there we find constructor completion, we can use m_expressionResult
     */
    if ( m_expressionResult.isInstance && !m_expressionResult.type.abstractType().cast<FunctionType>() )
      ret << m_expressionResult.type;
    break;
  }
  case ReturnAccess:
    if (AbstractType::Ptr returnType = functionReturnType(m_duContext.data()))
      ret << returnType->indexed();
    break;
  case CaseAccess:
    if (m_expressionResult.isValid() && m_expressionResult.type)
      ret << m_expressionResult.type;
  default:
    break;
  }

  m_cachedMatchTypes = ret.toList();
  return m_cachedMatchTypes;
}

QList<DeclAccessPair> CodeCompletionContext::containedDeclarationsForLookahead(Declaration* container, TopDUContext* top,
                                                                               bool isPointer) const
{
  static const IndexedIdentifier arrowOpIdentifier(Identifier("operator->"));
  QList<DeclAccessPair> ret;
  if (!container || !container->internalContext())
    return ret;

  Declaration *arrowOperator = 0;
  QVector<Declaration*> declarations = container->internalContext()->localDeclarations(top);
  foreach(Declaration *decl, declarations)
  {
    if (decl->isTypeAlias() || decl->isForwardDeclaration() || decl->type<EnumerationType>())
      continue; //Skip declarations that are not accessed via ./->

    if (!isPointer && decl->indexedIdentifier() == arrowOpIdentifier)
      arrowOperator = decl;

    if (!filterDeclaration(dynamic_cast<ClassMemberDeclaration*>(decl))) {
      continue;
    }

    if (Cpp::effectiveType(decl)) {
      ret << DeclAccessPair(decl, isPointer);
    }
  }
  //If we found an "->", try to treat it as a smart pointer
  if (arrowOperator) {
    ret += containedDeclarationsForLookahead( containerDeclForType(Cpp::effectiveType(arrowOperator), top, isPointer),
                                              top, true );
  }
  return ret;
}

QList<DeclAccessPair> CodeCompletionContext::getLookaheadMatches(Declaration* forDecl, const QList<IndexedType>& matchTypes) const
{
  QList<DeclAccessPair> ret;
  if (forDecl->isFunctionDeclaration() || forDecl->kind() != Declaration::Instance || !forDecl->abstractType())
    return ret; //We can only use instances, for now no sub decls of functions either TODO: be nice to get no-arg functions at least
  TopDUContext* top = m_duContext->topContext();
  bool typeIsPointer = false;
  Declaration* container = containerDeclForType(Cpp::effectiveType(forDecl), top, typeIsPointer);
  if (!container) {
    return ret;
  }

  QHash<Declaration*, QList<DeclAccessPair> >::const_iterator cacheIt = m_lookaheadMatchesCache.constFind(container);
  if (cacheIt != m_lookaheadMatchesCache.constEnd()) {
    return cacheIt.value();
  }

  /// FIXME: use QVector + std::remove_if
  ret = containedDeclarationsForLookahead(container, top, typeIsPointer);

  QList<DeclAccessPair>::iterator it = ret.begin();
  Cpp::TypeConversion conv(top);
  while (it != ret.end()) {
    bool match = false;
    const IndexedType& declEffectiveType = Cpp::effectiveType(it->first)->indexed();
    Q_ASSERT(declEffectiveType);

    foreach (const IndexedType& matchType, matchTypes) {
      //Don't lookahead if the current type is a (precise) match
      //Cheaper than checking if it converts, and probably good enough
      if (matchType == forDecl->indexedType())
        continue;
      if (conv.implicitConversion(declEffectiveType, matchType)) {
        match = true;
        break;
      }
    }

    if (!match) {
      it = ret.erase(it);
    } else {
      ++it;
    }
  }

  // Could use hideOverloadedDeclarations theoretically here, but it would do very
  // little since we don't have the real declaration depth

  m_lookaheadMatchesCache.insert(container, ret);

  return ret;
}

QList<DeclarationDepthPair> CodeCompletionContext::namespaceItems(DUContext* duContext, const CursorInRevision& position,
                                                                  bool global, const QSet<DUContext*>& skipContexts) const
{
  QList<DeclarationDepthPair> decls;
  QList<Declaration*> foundDecls;
  //Collect the contents of unnamed namespaces
  if (global) {
    foundDecls = duContext->findDeclarations(QualifiedIdentifier(unnamedNamespaceIdentifier().identifier()), position);
  } else {
    foundDecls = duContext->findLocalDeclarations(unnamedNamespaceIdentifier().identifier(), position);
  }
  foreach(Declaration* ns, foundDecls)
    if(ns->kind() == Declaration::Namespace && ns->internalContext())
      decls += ns->internalContext()->allDeclarations(position, duContext->topContext(), false);

  //Collect the Declarations from all "using namespace" imported contexts
  if (global) {
    foundDecls = duContext->findDeclarations( globalImportIdentifier(), position,
                                               0, DUContext::NoFiltering );
  } else {
    foundDecls = duContext->findLocalDeclarations( globalImportIdentifier(), position,
                                                   0, AbstractType::Ptr(), DUContext::NoFiltering );
  }

  QSet<QualifiedIdentifier> ids;
  foreach(Declaration* importDecl, foundDecls) {
    NamespaceAliasDeclaration* aliasDecl = dynamic_cast<NamespaceAliasDeclaration*>(importDecl);
    if(aliasDecl) {
      ids.insert(aliasDecl->importIdentifier());
    }else{
      kDebug() << "Import is not based on NamespaceAliasDeclaration";
    }
  }

  QualifiedIdentifier ownNamespaceScope = Cpp::namespaceScopeComponentFromContext(duContext->scopeIdentifier(true), duContext, duContext->topContext());
  if(!ownNamespaceScope.isEmpty())
    for(int a = 1; a <= ownNamespaceScope.count(); ++a)
      ids += ownNamespaceScope.left(a);

  foreach(const QualifiedIdentifier &id, ids) {
    QList<Declaration*> importedContextDecls = duContext->findDeclarations( id );
    foreach(Declaration* contextDecl, importedContextDecls) {
      if(contextDecl->kind() != Declaration::Namespace || !contextDecl->internalContext())
        continue;
      DUContext* context = contextDecl->internalContext();
      if (skipContexts.contains(context)) {
        continue;
      }

      if(context->range().contains(duContext->range()) && context->url() == duContext->url())
        continue; //If the context surrounds the current one, the declarations are visible through allDeclarations(..).
      foreach(Declaration* decl, context->localDeclarations()) {
        if(filterDeclaration(decl))
          decls << qMakePair(decl, 1200);
      }
    }
  }
  return decls;
}

QList< CompletionTreeItemPointer > CodeCompletionContext::standardAccessCompletionItems() {
  QList<CompletionTreeItemPointer> items;
  LOCKDUCHAIN; if (!m_duContext) return items;
  //Normal case: Show all visible declarations
  QSet<QualifiedIdentifier> hadNamespaceDeclarations;

  bool typeIsConst = false;
  if (Declaration* func = Cpp::localFunctionFromCodeContext(m_duContext.data())) {
    if (func->abstractType() && (func->abstractType()->modifiers() & AbstractType::ConstModifier))
      typeIsConst = true;
  }
  QList<DeclarationDepthPair> decls = m_duContext->allDeclarations(m_duContext->type() == DUContext::Class ? m_duContext->range().end : m_position, m_duContext->topContext());
  decls += namespaceItems(m_duContext.data(), m_position, true);

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

      if(filterDeclaration(decl.first, 0, true)) {
        decls << decl;
      }
    }
    
  decls = Cpp::hideOverloadedDeclarations(decls, typeIsConst);

  foreach( const DeclarationDepthPair& decl, decls ) {
    NormalDeclarationCompletionItem* item = new NormalDeclarationCompletionItem(DeclarationPointer(decl.first), KDevelop::CodeCompletionContext::Ptr(this), decl.second );

    if( m_onlyShow == ShowIntegralConstants && !isIntegralConstant(decl.first, false) )
      item->m_fixedMatchQuality = 0;

    items << CompletionTreeItemPointer(item);
  }

  return items;
}

void CodeCompletionContext::addOverridableItems()
{
  if(m_duContext->type() != DUContext::Class)
    return;

  //Show override helper items
  QMap< QPair<IndexedType, IndexedString>, KDevelop::CompletionTreeItemPointer > overridable;
  foreach(const DUContext::Import &import, m_duContext->importedParentContexts())
  {
    DUContext* ctx = import.context(m_duContext->topContext());
    if(ctx)
      getOverridable(m_duContext.data(), ctx, overridable, Ptr(this));
  }

  if(!overridable.isEmpty())
    eventuallyAddGroup(i18n("Virtual Override"), 0, overridable.values());
}

void CodeCompletionContext::addImplementationHelpers()
{
  QList<CompletionTreeItemPointer> helpers = getImplementationHelpers();
  if(!helpers.isEmpty()) {
    eventuallyAddGroup(i18nc("@action", "Implement Function"), 0, helpers);
  }
}

void CodeCompletionContext::addCPPBuiltin()
{
  ///Eventually add a "this" item
  LOCKDUCHAIN; if (!m_duContext) return;
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
    QExplicitlySharedDataPointer<TypeConversionCompletionItem> item( new TypeConversionCompletionItem("this", thisPointer->indexed(), 0, Cpp::CodeCompletionContext::Ptr(this)) );
    item->setPrefix(thisPointer->toString());
    QList<CompletionTreeItemPointer> lst;
    lst += CompletionTreeItemPointer(item.data());
    eventuallyAddGroup(i18n("C++ Builtin"), 800, lst);
  }
  eventuallyAddGroup(i18n("C++ Builtin"), 800, keywordCompletionItems());
}

bool CodeCompletionContext::shouldAddParentItems(bool fullCompletion)
{
  if (!m_parentContext)
    return false;

  if ( !fullCompletion && (!Cpp::useArgumentHintInAutomaticCompletion() || depth() != 0) )
    return false;

  if ( NO_MULTIPLE_BINARY_OPERATORS && m_accessType == BinaryOpFunctionCallAccess &&
                                       parentContext()->m_accessType == BinaryOpFunctionCallAccess )
    return false;

  return true;
}

QList<CompletionTreeItemPointer> CodeCompletionContext::completionItems(bool& shouldAbort, bool fullCompletion) {
    QList<CompletionTreeItemPointer> items;
    if(!m_valid)
      return items;

    // Call parent context before adding our items because if parent is CaseAccess, this call
    // will make it compute its expression type (which we need in standardAccessCompletionItems())
    if(shouldAddParentItems(fullCompletion))
      items = parentContext()->completionItems( shouldAbort, fullCompletion );

    switch(m_accessType) {
      case MemberAccess:
      case ArrowMemberAccess:
      case StaticMemberChoose:
      case MemberChoose:
        items += memberAccessCompletionItems(shouldAbort);
        break;
      case ReturnAccess:
        items += returnAccessCompletionItems();
        break;
      case CaseAccess:
        items += caseAccessCompletionItems();
        break;
      case TemplateAccess:
        items += templateAccessCompletionItems();
        break;
      case FunctionCallAccess:
        items += functionAccessCompletionItems(fullCompletion);
        break;
      case BinaryOpFunctionCallAccess:
        items += binaryFunctionAccessCompletionItems(fullCompletion);
        break;
      case IncludeListAccess:
        items += includeListAccessCompletionItems(shouldAbort);
        break;
      case SignalAccess:
      case SlotAccess:
        items += signalSlotAccessCompletionItems();
        //Since there is 2 connect() functions, the third argument may be a slot as well as a QObject*, so also
        //give normal completion items
        if(parentContext() && parentContext()->m_knownArgumentExpressions.size() != 2)
          break;
      default:
        if(depth() == 0 && (m_onlyShow == ShowAll || m_onlyShow == ShowTypes || m_onlyShow == ShowIntegralConstants))
        {
          items += standardAccessCompletionItems();
#ifndef TEST_COMPLETION
          eventuallyAddGroup(i18n("Not Included"), 700, missingIncludeCompletionItems(m_followingText + ':', {}, {}, m_duContext));
#endif
          addCPPBuiltin();
        }
        break;
    }

    LOCKDUCHAIN; if (!m_duContext) return items;
    if (m_accessType == MemberAccess ||
        m_accessType == ArrowMemberAccess ||
        m_accessType == MemberChoose ||
        m_accessType == NoMemberAccess)
      addLookaheadMatches(items);

    if (parentContext()) {
      foreach(const IndexedType &matchType, parentContext()->matchTypes()) {
        addSpecialItemsForArgumentType(matchType.abstractType());
      }
    }

    if(depth() == 0)
    {
      if (!parentContext())
        addOverridableItems();
      if (isImplementationHelperValid())
        addImplementationHelpers();
    }

    return items;
}

void CodeCompletionContext::addLookaheadMatches(const QList<CompletionTreeItemPointer> items)
{
  QList<IndexedType> matchTypes;
  if (m_parentContext)
    matchTypes = parentContext()->matchTypes();

  if (!matchTypes.size())
    return;

  QList<CompletionTreeItemPointer> lookaheadMatches;
  foreach( const CompletionTreeItemPointer &item, items ) {
    Declaration* decl = item->declaration().data();
    if (!decl)
      continue;

    QList<DeclAccessPair> lookaheadDecls = getLookaheadMatches(decl, matchTypes);
    foreach(const DeclAccessPair &lookaheadDecl, lookaheadDecls)
    {
      NormalDeclarationCompletionItem* lookaheadItem =
          new NormalDeclarationCompletionItem(DeclarationPointer(lookaheadDecl.first), KDevelop::CodeCompletionContext::Ptr(this));
      lookaheadItem->prefixText = decl->identifier().toString() + (lookaheadDecl.second ? "->" : ".");
      //Perhaps it'd be nice to have these stand out more without polluting the "Best Matches"
      //NormalDeclarationCompletionItem should be refactored in order to make subclassing simpler
      lookaheadItem->m_fixedMatchQuality = 0;
      lookaheadMatches << CompletionTreeItemPointer(lookaheadItem);
    }
  }
  m_lookaheadMatchesCache.clear();

  eventuallyAddGroup(i18n("Lookahead Matches"), 800, lookaheadMatches);
}

QList<CompletionTreeItemPointer> CodeCompletionContext::getImplementationHelpers() {
  QList<CompletionTreeItemPointer> ret;
  TopDUContext* searchInContext = m_duContext->topContext();

  if(searchInContext)
    ret += getImplementationHelpersInternal(m_duContext->scopeIdentifier(true), searchInContext);

  if(!CppUtils::isHeader( searchInContext->url().toUrl() )) {
    KUrl headerUrl = CppUtils::sourceOrHeaderCandidate( searchInContext->url().str(), false );
    searchInContext = ICore::self()->languageController()->language("C++")->languageSupport()->standardContext(headerUrl);
    if(searchInContext)
      ret += getImplementationHelpersInternal(m_duContext->scopeIdentifier(true), searchInContext);
  }

  return ret;
}

QList<CompletionTreeItemPointer> CodeCompletionContext::getImplementationHelpersInternal(const QualifiedIdentifier& minimumScope, DUContext* context)
{
  QList<CompletionTreeItemPointer> ret;

  foreach(Declaration* decl, context->localDeclarations()) {
    if (decl->range().isEmpty() || decl->isDefinition() || FunctionDefinition::definition(decl)) {
      continue;
    }
    if (!decl->qualifiedIdentifier().toString().startsWith(minimumScope.toString())) {
      continue;
    }
    AbstractFunctionDeclaration* funDecl = dynamic_cast<AbstractFunctionDeclaration*>(decl);
    if (!funDecl) {
      continue;
    }
    ClassFunctionDeclaration* classFun = dynamic_cast<ClassFunctionDeclaration*>(decl);
    if (classFun && (classFun->isAbstract() || classFun->isSignal())) {
      continue;
    }
    ret << KDevelop::CompletionTreeItemPointer(
      new ImplementationHelperItem( ImplementationHelperItem::CreateDefinition,
                                    DeclarationPointer(decl),
                                    QExplicitlySharedDataPointer<CodeCompletionContext>(this)));
  }

  foreach(DUContext* child, context->childContexts()) {
    if(child->type() == DUContext::Namespace
        || child->type() == DUContext::Class
        || child->type() == DUContext::Helper)
    {
      ret += getImplementationHelpersInternal(minimumScope, child);
    }
  }

  return ret;
}

void CodeCompletionContext::addSpecialItemsForArgumentType(AbstractType::Ptr type) {
  QList< CompletionTreeItemPointer > items;
  if(EnumerationType::Ptr enumeration = TypeUtils::realType(type, m_duContext->topContext()).cast<EnumerationType>()) {
    Declaration* enumDecl = enumeration->declaration(m_duContext->topContext());
    if(enumDecl && enumDecl->internalContext()) {
      DUContext* enumInternal = enumDecl->internalContext();
      foreach(Declaration* enumerator, enumInternal->localDeclarations()) {
        NormalDeclarationCompletionItem *item =
            new NormalDeclarationCompletionItem( DeclarationPointer(enumerator), KDevelop::CodeCompletionContext::Ptr(this), 0 );
        item->prependScopePrefix = true;
        item->m_fixedMatchQuality = 0;
        items << CompletionTreeItemPointer(item);
      }
    }
  }
  eventuallyAddGroup("Enum values", 0, items);
}

bool CodeCompletionContext::visibleFromWithin(Declaration* decl, DUContext* currentContext) const {
  if(!decl || !currentContext)
    return false;
  if(currentContext->imports(decl->context()))
    return true;
  
  return visibleFromWithin(decl, currentContext->parentContext());
}

bool CodeCompletionContext::isIntegralConstant(Declaration* decl, bool acceptHelperItems) const {

    // Usability issue: if we're matching for integral constants,
    // types and functions are also allowed (see filterDeclaration()),
    // but shall not pollute "best matches" as one rarely would need them.
    // So introduce "acceptHelperItems" to distinguish between filtering items and demoting them to zero match quality.
    // (see standardAccessCompletionItems())

    switch (decl->kind()) {
      case Declaration::Namespace:
      case Declaration::NamespaceAlias:
      case Declaration::Type:
        // Type-names and namespace-names in general may be used for completing constants either as:
        // "IntegralType(42)"
        // "EnumName::someEnumerator" (that's valid C++11)
        // "ClassName::someStaticConstantField"
        // "NamespaceName::see_everything_above"
        return acceptHelperItems;

      case Declaration::Instance: {
        FunctionType::Ptr funType;
        IntegralType::Ptr integralType;

        // If a declaration is a known integer compile-time constant, it's valid.
        // NOTE: are there any chances of missing a compile-time constant due to our parser's shortcomings?
        if (ConstantIntegralType::Ptr constantIntegralType = decl->type<ConstantIntegralType>())
          integralType = constantIntegralType.cast<IntegralType>();

        // If a declaration is a constexpr function returning integral type, it's valid.
        // TODO: change this when constexpr becomes parsed:
        // - add check for constexpr-ness after (funType = ...)
        // - remove (acceptHelperItems) since that will be a true match - eligible for "best matches"
        else if (acceptHelperItems && (funType = decl->type<FunctionType>()))
          integralType = funType->returnType().cast<IntegralType>();

        // Finally, check if retrieved type is integer.
        return (integralType && TypeUtils::isIntegerType(integralType));
      }

      case Declaration::Alias:
      case Declaration::Import:
      default:
        return false;
    }
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

bool  CodeCompletionContext::filterDeclaration(Declaration* decl, DUContext* declarationContext, bool dynamic) const {
  if(!decl)
    return true;

  if (decl->isExplicitlyDeleted()) {
    return false;
  }

  if(dynamic_cast<TemplateParameterDeclaration*>(decl) && !visibleFromWithin(decl, m_duContext.data()))
    return false;
  
  static const IndexedIdentifier friendIdentifier(Identifier("friend"));
  
  if(decl->indexedIdentifier().isEmpty()) //Filter out nameless declarations
    return false;

  if(decl->indexedIdentifier() == friendIdentifier || decl->indexedIdentifier() == Cpp::unnamedNamespaceIdentifier()
     || decl->indexedIdentifier() == globalIndexedImportIdentifier())
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

  if (m_accessType == NamespaceAccess)
    return decl->kind() == Declaration::Namespace || decl->kind() == Declaration::NamespaceAlias;

  if(m_onlyShow == ShowIntegralConstants && !isIntegralConstant(decl, true))
    return false;

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
      return filterDeclaration(classMember, declarationContext);
  }

  // https://bugs.kde.org/show_bug.cgi?id=206376
  // hide void functions in expressions but don't hide signals / slots with void return type
  if (m_onlyShow != ShowSignals && m_onlyShow != ShowSlots
      && m_parentContext && decl->isFunctionDeclaration()
      && getIntegralReturnType(decl->abstractType()) == IntegralType::TypeVoid)
  {
    const ExpressionEvaluationResult& result =
            static_cast<const CodeCompletionContext*>(m_parentContext.data())->m_expressionResult;
    // for now only hide in non-lvalue expressions so we don't get problems in sig/slot connections e.g.
    if (result.type.isValid() && !result.isLValue())
      return false;
  }

  return true;
}

bool CodeCompletionContext::filterDeclaration(ClassMemberDeclaration* decl, DUContext* declarationContext) const {
  if(m_doAccessFiltering && decl) {
    if(!Cpp::isAccessible(m_localClass ? m_localClass.data() : m_duContext.data(), decl, m_duContext->topContext(), declarationContext))
      return false;
  }
  return filterDeclaration((Declaration*)decl, declarationContext, false);
}

void CodeCompletionContext::replaceCurrentAccess(const QString& old, const QString& _new)
{
  //We must not change the document from within the background, so we use a queued connection to an object created in the foregroud
  QMetaObject::invokeMethod(&s_mainThreadHelper, "replaceCurrentAccess", Qt::QueuedConnection,
                            Q_ARG(KUrl, m_duContext->url().toUrl()), Q_ARG(QString, old), Q_ARG(QString, _new));
}

int CodeCompletionContext::matchPosition() const {
  return m_knownArgumentExpressions.count();
}

void CodeCompletionContext::eventuallyAddGroup(QString name, int priority, QList< QExplicitlySharedDataPointer< KDevelop::CompletionTreeItem > > items) {
  if(items.isEmpty())
    return;
  KDevelop::CompletionCustomGroupNode* node = new KDevelop::CompletionCustomGroupNode(name, priority);
  node->appendChildren(items);
  m_storedUngroupedItems << CompletionTreeElementPointer(node);
}

QList< QExplicitlySharedDataPointer< KDevelop::CompletionTreeItem > > CodeCompletionContext::keywordCompletionItems() {
  QList<CompletionTreeItemPointer> ret;
  #ifdef TEST_COMPLETION
  return ret;
  #endif
  #define ADD_TYPED_TOKEN_S(X, type) ret << CompletionTreeItemPointer( new TypeConversionCompletionItem(X, type, 0, QExplicitlySharedDataPointer<Cpp::CodeCompletionContext>(this)) )
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
    ADD_TOKEN(char16_t);
    ADD_TOKEN(char32_t);
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
    ADD_TOKEN(nullptr);
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
  m_followingText = str.trimmed();
}


}

#include "context.moc"

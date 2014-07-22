/*
 * KDevelop C++ Code Completion Support
 *
 * Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "item.h"
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <ktexteditor/range.h>
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <cpptypes.h>
#include <QModelIndex>
#include "helpers.h"
#include "model.h"
#include <language/duchain/declaration.h>
#include <language/duchain/classfunctiondeclaration.h>
#include <language/duchain/namespacealiasdeclaration.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/classdeclaration.h>
#include "../cppduchain/qtfunctiondeclaration.h"
#include <typeutils.h>
#include <cppduchain.h>
#include <templatedeclaration.h>
#include <language/codecompletion/codecompletionhelper.h>
#include "context.h"
#include <ktexteditor/codecompletioninterface.h>
#include <ktexteditor/document.h>
#include <ktexteditor/movingrange.h>
#include <ktexteditor/movinginterface.h>
#include <memory>
using namespace KDevelop;

namespace {
QString linePrefix( KTextEditor::Document* document, const KTextEditor::Range& word )
{
    KTextEditor::Range linePrefixRange( word.start().line(), 0, word.start().line(), word.start().column() );
    return document->text( linePrefixRange );
}

QString lineSuffixAndWord( KTextEditor::Document* document, const KTextEditor::Range& word )
{
    int endLineLength = document->lineLength( word.end().line() );
    KTextEditor::Range lineSuffixRange( word.start().line(), word.start().column(), word.end().line(), endLineLength );
    return document->text( lineSuffixRange );
}
bool declarationNeedsTemplateParameters(const Declaration* decl) {
  const Cpp::TemplateDeclaration* asTemplate = dynamic_cast<const Cpp::TemplateDeclaration*>(decl);
  //TODO: It'd be nice to have comma separated blanks for CppTemplateParameterTypes in specializations, with the cursor placed in the first one
  if(asTemplate && !asTemplate->specializedWith().isValid()) {
    DUContext* templateContext = asTemplate->templateContext(decl->topContext());
    if(templateContext) {
      foreach(Declaration* decl, templateContext->localDeclarations()) {
        if(decl->type<CppTemplateParameterType>())
          return true;
      }
    }
  }
  return false;
}
} // anonymous namespace

namespace Cpp {

AbstractType::Ptr applyPointerConversions(AbstractType::Ptr type, int pointerConversions, TopDUContext *top)
{
  if(pointerConversions == 0 || type)
    return type;

  if(pointerConversions > 0) {
    for(int a = 0; a < pointerConversions && type; ++a) {
      type = TypeUtils::increasePointerDepth(type);
    }
  }else{
    for(int a = pointerConversions; a < 0 && type; ++a) {
      type = TypeUtils::decreasePointerDepth(type, top, true);
    }
  }

  return type;
}

int getMatchQuality(CodeCompletionContext *context, const Declaration* decl, TopDUContext* top)
{
  if (!context->parentContext() || decl->kind() == Declaration::Type
      || decl->kind() == Declaration::Namespace || decl->kind() == Declaration::NamespaceAlias)
  {
    return 0;
  }

  const QList<IndexedType>& matchTypes = context->parentContext()->matchTypes();
  if (matchTypes.isEmpty())
    return 0;

  int pointerConversions = context->pointerConversions();
  if (pointerConversions > 1)
    return 0; //Can't do "&&foo"

  const IndexedType& effectiveDeclType = applyPointerConversions(effectiveType(decl), pointerConversions, top)->indexed();

  bool fromLValue = (bool)decl->type<ReferenceType>() ||
                    (!dynamic_cast<const AbstractFunctionDeclaration*>(decl) &&
                      decl->kind() == Declaration::Instance);
  if(pointerConversions > 0 && !fromLValue)
    return 0;

  Cpp::TypeConversion conv(top);

  int bestQuality = 0;
  foreach(const IndexedType& type, matchTypes) {
    int quality = conv.implicitConversion( effectiveDeclType, type, fromLValue )  * 10 / Cpp::MaximumConversionResult;
    if(quality > bestQuality)
      bestQuality = quality;
  }

  return bestQuality;
}

void executeSignalSlotCompletionItem( KTextEditor::View* view, const KTextEditor::Range& enteredWord,
                                      bool isSignal, const QString& name, const QString& signature )
{
  QString newText;
  KTextEditor::Range word( enteredWord );

  // Find out whether user has already entered SIGNAL/SLOT
  // If yes, we remove that word and re-insert it.
  // This is needed to compensate for a possible mistake when one types SIGNAL instead of SLOT or vice versa.
  // NOTE: we do not scan for Q_SIGNAL or Q_SLOT, as these words (with "Q_" in beginning)
  // also match ones without "Q_"; thus we keep the user's preference of typing "Q_" or not.
  {
    QString prefixText = linePrefix( view->document(), word );
    // We match SIGNAL or SLOT, followed by spaces, then (maybe) an opening bracket and spaces, then EOL.
    // Thus we ensure that cursor is positioned like "SIGNAL( <here>", not "SIGNAL( something <here>".
    // That is, we match only those SIGNAL/SLOT statements, for which we shall suggest a complete method signature.
    QRegExp signalSlotRegExp( "(SIGNAL|SLOT)\\s*(\\(\\s*)$" );
    int signalSlotAt = signalSlotRegExp.lastIndexIn( prefixText );
    if( signalSlotAt >= 0 ) {
      word.setStart(KTextEditor::Cursor(word.start().line(), signalSlotAt));
    }
  }

  // Compute the replacement text
  {
    if( isSignal ) {
      newText += "SIGNAL(";
    } else {
      newText += "SLOT(";
    }
    newText += name + '(' + signature + "))";
  }

  // Try to remove the leftovers at the suffix, like when one types
  // "SIGNAL( <here> someOtherSignal(args))"
  // or "<here> SIGNAL(someOtherSignal(args))".
  // Implement this by deleting everything to the next statement delimiter or end of last line
  // (if a delimiter is absent), because a SIGNAL/SLOT macro can apparently be used only as an initializer/parameter.
  // Also, take brackets (nesting) into account. Start with zero level at the beginning of the matched word
  // (why beginning: if we've got a SIGNAL/SLOT word and a bracket in prefix, then count for it too),
  // skip all characters when "nesting > 0" (they are arguments) and stop when "nesting < 0"
  // (i. e., when we encounter connect()'s closing bracket).
  {
    QString suffixText = lineSuffixAndWord( view->document(), word );
    int nesting = 0, position = 0;
    for( ; position < suffixText.size(); ++position ) {
      QChar c = suffixText.at( position );
      bool isDelimiter = false;

      switch( c.unicode() ) {
      case '(' : ++nesting; break;
      case ')' : --nesting; break;
      case ',' :
      case ';' : isDelimiter = true; break;
      default  : break;
      }

      if( ( nesting <  0 ) ||
          ( ( nesting == 0 ) && isDelimiter ) ) {
        break;
      }
    } // scan loop

    // User-friendliness: do not touch any trailing space.
    for( ;
         position > 0 && QChar( suffixText.at( position - 1 ) ).isSpace();
         --position )
      { /* no-op */ }

    // Now "position" points to the first character that should _not_ be removed.
    // Calculate its line and column.
    // This is a great place for off-by-one bugs...
    // NOTE: a word can span multiple lines. As the suffix text is by definition at the last line of word,
    // the found char is at the last line of the range.
    // Now translate position to column, but using distance from position to suffix text end (rather than to its beginning).
    int lastLineLength = view->document()->lineLength( word.end().line() );
    int distanceToTextEnd = suffixText.length() - position;
    int characterColumn = lastLineLength - distanceToTextEnd;
    word.setEnd(KTextEditor::Cursor(word.end().line(), characterColumn ));
  }

  view->document()->replaceText( word, newText );
  return;
}

void NormalDeclarationCompletionItem::execute(KTextEditor::View* view, const KTextEditor::Range& _word) {
  if( completionContext() && completionContext()->depth() != 0 )
    return; //Do not replace any text when it is an argument-hint

  if(m_isQtSignalSlotCompletion) {
    KDevelop::DUChainReadLocker lock( KDevelop::DUChain::lock() );
    Cpp::QtFunctionDeclaration* classFun = dynamic_cast<Cpp::QtFunctionDeclaration*>( m_declaration.data() );
    if( !classFun ) {
      kWarning() << "Signal/slot completion declaration is not a QtFunctionDeclaration";
      return;
    }
    executeSignalSlotCompletionItem( view, _word, classFun->isSignal(),
                                     classFun->identifier().toString(), classFun->normalizedSignature().str() );
    return;
  }

  KTextEditor::Range word(_word);
  QString newText;

  if(!useAlternativeText) {
    KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
    if(m_declaration) {
      newText = m_declaration->identifier().toString();
      if(prependScopePrefix) {
        QualifiedIdentifier parentScope = Cpp::stripPrefixes(m_completionContext->duContext(), m_declaration->context()->scopeIdentifier(true));
        if(!parentScope.isEmpty())
            newText.prepend (parentScope.toString() + "::");
      }
    } else {
      kDebug() << "Declaration disappeared";
      if(!alternativeText.isEmpty())
        newText = alternativeText;
      else
        return;
    }
  }else{
    newText = alternativeText;
  }
  newText.prepend(prefixText);
  
  // Text that will be removed in a separate editing step (so the user can undo it)
  QScopedPointer<KTextEditor::MovingRange> removeInSecondStep;
  
  KTextEditor::Cursor cursor = view->cursorPosition();
  KTextEditor::Document* document = view->document();
  if(cursor != word.end())
  {
    KTextEditor::MovingInterface* moving = dynamic_cast<KTextEditor::MovingInterface*>(document);
    Q_ASSERT(moving);
    removeInSecondStep.reset(moving->newMovingRange(KTextEditor::Range(cursor, word.end()), KTextEditor::MovingRange::DoNotExpand));
    word.setEnd(cursor);
  }
  
  KTextEditor::Range nextToken = KTextEditor::Range(_word.end(), KTextEditor::Cursor(_word.end().line(), _word.end().column() + 2));
  bool followingColon = document->text(nextToken) == "::";
  bool followingBrace = document->text(nextToken).contains('{');
  document->replaceText(word, newText);
  
  KTextEditor::Cursor end = word.start();
  end.setColumn(end.column() + newText.length());
  
  bool jumpForbidden = false;
  
  KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
  
  if(m_declaration.data())
  {
    if(declarationNeedsTemplateParameters(m_declaration.data())) {
      jumpForbidden = true;
      lock.unlock();
      document->insertText( end, "<>" );
      end.setColumn(end.column() + 2);
      view->setCursorPosition( end - KTextEditor::Cursor(0, 1) );
      lock.lock();
    }
    
    if(m_declaration.data()->kind() == Declaration::Namespace) {
      CodeCompletionContext* ctx = static_cast<CodeCompletionContext*>(m_completionContext.data());
      if (ctx->accessType() != CodeCompletionContext::NamespaceAccess && !followingColon) {
        // completing a namespace identifier usage
        lock.unlock();
        document->insertText(end, "::");
        end.setColumn(end.column() + 2);
        lock.lock();
      } else if (ctx->accessType() == CodeCompletionContext::NamespaceAccess && !followingBrace) {
        // we complete a namespace declaration, thus add braces
        lock.unlock();
        document->insertText(end, " {}");
        end.setColumn(end.column() + 3);
        lock.lock();
      }
    }
      
    if( !useAlternativeText && m_declaration && (dynamic_cast<AbstractFunctionDeclaration*>(m_declaration.data()) || completionContext()->isConstructorInitialization()) ) {
      //Do some intelligent stuff for functions with the parens:
      lock.unlock();
      KTextEditor::Cursor insertionPosition = end;
      if (removeInSecondStep) {
        KTextEditor::Range removeRange = removeInSecondStep->toRange();
        insertionPosition += removeRange.end() - removeRange.start();
      }
      insertFunctionParenText(view, insertionPosition, m_declaration, jumpForbidden);
    }
    
    if (removeInSecondStep) {
      //if we would remove text after the inserted text, skip that if it is a property
      //of the executed item and additionally insert a . or ->
      Identifier id(document->text(removeInSecondStep->toRange()));
      QString insertAccessor = keepRemainingWord(id);
      if (!insertAccessor.isEmpty()) {
        document->insertText(removeInSecondStep->toRange().start(), insertAccessor);
        removeInSecondStep.reset(nullptr);
      }
    }
  }
  
  if(removeInSecondStep)
  {
    KTextEditor::Range removeRange = removeInSecondStep->toRange();
    if(!removeRange.isEmpty() && removeRange.end() > end && removeRange.end().line() == end.line() && removeRange.end().column() <= document->lineLength(removeRange.end().line()))
    {
      // We stop the editing sequence, which was initiated by kate, so the user can manually undo the removal
      // -- I removed this feature, it's unclear how/if it works with the new transactions and not worth porting
      document->removeText(removeRange);
    }
  }
}

QString NormalDeclarationCompletionItem::keepRemainingWord(const Identifier& id)
{
  DUChainReadLocker lock;

  if (!m_declaration) {
    return QString();
  }

  TypePtr<StructureType> type;
  QString insertAccessor;
  if (m_declaration->type<StructureType>()) {
    type = m_declaration->type<StructureType>();
    if (m_declaration->internalContext() && m_declaration->internalContext()->type() == DUContext::Class) {
      insertAccessor = "::";
    } else {
      insertAccessor = ".";
    }
  } else if (m_declaration->type<PointerType>()) {
    type = StructureType::Ptr::dynamicCast(m_declaration->type<PointerType>()->baseType());
    insertAccessor = "->";
  }
  if (!type && (m_declaration->type<ReferenceType>())) {
    type = StructureType::Ptr::dynamicCast(m_declaration->type<ReferenceType>()->baseType());
    insertAccessor = ".";
  }
  if (type) {
    return keepRemainingWord(type, id, insertAccessor);
  }
  return QString();
}

QString NormalDeclarationCompletionItem::keepRemainingWord(const StructureType::Ptr &type, const Identifier &id, const QString &insertAccessor)
{
  ENSURE_CHAIN_READ_LOCKED;
  Q_ASSERT(m_declaration);

  if (Declaration* structDecl = type->declaration(m_declaration->topContext())) {
    if (!structDecl->internalContext()) {
      return QString();
    }
    if (structDecl->internalContext()->findDeclarations(id).count()) {
      return insertAccessor;
    }
    // might be a smart pointer or similar, hence check retval of operator-> if available
    const QList<Declaration*> opDecs = structDecl->internalContext()->findDeclarations(Identifier( "operator->" ));
    if (!opDecs.isEmpty()) {
      Declaration* smartPtrFn = opDecs.first();
      if (FunctionType::Ptr funType = smartPtrFn->type<FunctionType>()) {
        if (PointerType::Ptr ptrRetType = PointerType::Ptr::dynamicCast(funType->returnType())) {
          if (StructureType::Ptr retStructType = StructureType::Ptr::dynamicCast(ptrRetType->baseType())) {
            return keepRemainingWord(retStructType, id, "->");
          }
        }
      }
    }
  }
  return QString();
}

const bool indentByDepth = false;

//The name to be viewed in the name column
inline QString nameForDeclaration(Declaration* dec) {
  QString ret = dec->identifier().toString();

  if (ret.isEmpty())
    return "<unknown>";
  else
    return ret;
}

KTextEditor::CodeCompletionModel::CompletionProperties NormalDeclarationCompletionItem::completionProperties() const {
  Declaration* dec = m_declaration.data();
  if(!dec)
    return (KTextEditor::CodeCompletionModel::CompletionProperties)0;

  CodeCompletionModel::CompletionProperties p = DUChainUtils::completionProperties(dec);

  AbstractType::Ptr type(dec->abstractType());
  if (type) {
    if (type->modifiers() & AbstractType::ConstModifier)
      p |= CodeCompletionModel::Const;
    if (type->modifiers() & AbstractType::VolatileModifier) {
      ;//TODO
    }

    switch (dec->abstractType()->whichType()) {
      case AbstractType::TypeEnumerator:
          //Get the properties from the parent, because that may contain information like "private"
          if(dec->context()->owner()) {
            p = DUChainUtils::completionProperties(dec->context()->owner());
            p &= ~CodeCompletionModel::Enum;
            p |=  CodeCompletionModel::Variable;
          }
        break;
      case AbstractType::TypeStructure:
        if (CppClassType::Ptr classType =  dec->type<CppClassType>())
          p |= CodeCompletionModel::Class;
        break;
      default:
        break;
    }
  }

  if(useAlternativeText) {
    //Remove other scope flags, and add the local scope flag
    ///@todo Create an own "Hint" scope within KTextEditor::CodeCompletionModel, and use that
    p &= ~CodeCompletionModel::GlobalScope;
    p &= ~CodeCompletionModel::NamespaceScope;
    p |= CodeCompletionModel::LocalScope;
  }
  return p;
}

bool NormalDeclarationCompletionItem::completingTemplateParameters() const
{
  return m_isTemplateCompletion || declarationNeedsTemplateParameters(m_declaration.data());
}

QString NormalDeclarationCompletionItem::shortenedTypeString(KDevelop::DeclarationPointer decl, int desiredTypeLength) const
{
  if(m_cachedTypeStringDecl == decl && m_cachedTypeStringLength == static_cast<uint>(desiredTypeLength))
    return m_cachedTypeString;
  
  QString ret;
  
  if(completionContext() && completionContext()->duContext())
    ret = Cpp::shortenedTypeString(decl.data(), completionContext()->duContext(), desiredTypeLength);
  else
    ret = KDevelop::NormalDeclarationCompletionItem::shortenedTypeString(decl, desiredTypeLength);
  
  m_cachedTypeString = ret;
  m_cachedTypeStringDecl = decl;
  m_cachedTypeStringLength = desiredTypeLength;
  
  return ret;
}

KDevelop::QualifiedIdentifier NormalDeclarationCompletionItem::stripPrefix() const {
  if(completionContext() && completionContext()->duContext()) {
    const TopDUContext* top = completionContext()->duContext()->topContext();
    
    if(completionContext()->memberAccessContainer().allDeclarations.size())
      if( Declaration * const decl = completionContext()->memberAccessContainer().allDeclarations[0].getDeclaration(top) ) {
        AbstractType::Ptr t = decl->abstractType();
        IdentifiedType* idType = dynamic_cast<IdentifiedType*>(t.data());
        if(idType)
          return idType->qualifiedIdentifier();
      }
    
    return completionContext()->duContext()->scopeIdentifier(true);
  }
  
  return QualifiedIdentifier();
}

QVariant NormalDeclarationCompletionItem::data(const QModelIndex& index, int role, const KDevelop::CodeCompletionModel* model) const {

  DUChainReadLocker lock(DUChain::lock(), 500);
  if(!lock.locked()) {
    kDebug(9007) << "Failed to lock the du-chain in time";
    return QVariant();
  }
  
  if(!completionContext()) {
    kDebug(9007) << "Missing completion-context";
    return QVariant();
  }

  if(!m_declaration) {
    if(role == Qt::DisplayRole && index.column() == CodeCompletionModel::Name)
      return alternativeText;
    return QVariant();
  }else if(useAlternativeText) {
    if(role == Qt::DisplayRole) {
      if(index.column() == CodeCompletionModel::Name)
        return alternativeText;
      else
        return QVariant();
    }
  }

  Declaration* dec = const_cast<Declaration*>( m_declaration.data() );

  switch (role) {
    case CodeCompletionModel::MatchQuality:
      if(m_fixedMatchQuality == -1) {
        m_fixedMatchQuality = getMatchQuality(completionContext().data(), dec, model->currentTopContext().data());
      }
      return m_fixedMatchQuality;

    case CodeCompletionModel::ItemSelected:
       return QVariant(Cpp::NavigationWidget::shortDescription(dec));
    case Qt::DisplayRole:
      switch (index.column()) {
        case CodeCompletionModel::Prefix:
        {
          if(m_isQtSignalSlotCompletion) {
            Cpp::QtFunctionDeclaration* funDecl = dynamic_cast<Cpp::QtFunctionDeclaration*>(dec);
            if(funDecl) {
              if(funDecl->isSignal())
                return QVariant("SIGNAL");
              if(funDecl->isSlot())
                return QVariant("SLOT");
            }
          }
          int depth = m_inheritanceDepth;
          if( depth >= 1000 )
            depth-=1000;
          QString indentation;
          if(indentByDepth)
            indentation = QString(depth, ' ');

          if(m_declaration->kind() == Declaration::Namespace)
            return QString(indentation + "namespace");
          
          if( NamespaceAliasDeclaration* alias = dynamic_cast<NamespaceAliasDeclaration*>(dec) ) {
            if( alias->identifier().isEmpty() ) {
              return QString(indentation + "using namespace");/* " + alias->importIdentifier().toString();*/
            } else {
              return QString(indentation + "namespace");/* " + alias->identifier().toString() + " = " + alias->importIdentifier().toString();*/
            }
          }

          if( dec->isTypeAlias() )
            indentation += "typedef ";

          if( dec->kind() == Declaration::Type && !dec->type<FunctionType>() && !dec->isTypeAlias() ) {
              if (CppClassType::Ptr classType =  dec->type<CppClassType>()){
                ClassDeclaration* classDecl  = dynamic_cast<ClassDeclaration*>(dec);
                if(classDecl) {
                  switch (classDecl->classType()) {
                    case ClassDeclarationData::Class:
                      return QString(indentation + "class");
                      break;
                    case ClassDeclarationData::Struct:
                      return QString(indentation + "struct");
                      break;
                    case ClassDeclarationData::Union:
                      return QString(indentation + "union");
                      break;
                    default:
                      ;
                  }
                }else if(dec->isForwardDeclaration()) {
                  return QString(indentation + "class"); ///@todo Would be useful to have the class/struct/union info also for forward-declarations
                }
              }
              if(dec->type<EnumerationType>()) {
                return "enum";
              }
            return QVariant();
          }
          if (m_declaration->abstractType()) {
            if(EnumeratorType::Ptr enumerator = m_declaration->type<EnumeratorType>()) {
              if(m_declaration->context()->owner() && m_declaration->context()->owner()->abstractType()) {
                if(!m_declaration->context()->owner()->identifier().isEmpty())
                  return shortenedTypeString(DeclarationPointer(m_declaration->context()->owner()), desiredTypeLength);
                else
                  return "enum";
              }
            }
            if (FunctionType::Ptr functionType = m_declaration->type<FunctionType>()) {
              ClassFunctionDeclaration* funDecl = dynamic_cast<ClassFunctionDeclaration*>(m_declaration.data());

              if (functionType->returnType()) {
                QString ret = shortenedTypeString(m_declaration, desiredTypeLength);
                if(shortenArgumentHintReturnValues && argumentHintDepth() && ret.length() > maximumArgumentHintReturnValueLength)
                  return QString("...");
                else
                  return ret;
              }else if(argumentHintDepth()) {
                return QString();//Don't show useless prefixes in the argument-hints
              }else if(funDecl && funDecl->isConstructor() )
                return "<constructor>";
              else if(funDecl && funDecl->isDestructor() )
                return "<destructor>";
              else
                return "<incomplete type>";

            } else {
              return shortenedTypeString(m_declaration, desiredTypeLength);
            }
          } else {
            return "<incomplete type>";
          }
          return QVariant();
        }
        break;
        case CodeCompletionModel::Scope: {
          //The scopes are not needed
          return QVariant();
/*          QualifiedIdentifier id = dec->qualifiedIdentifier();
          if (id.isEmpty())
            return QVariant();
          id.pop();
          if (id.isEmpty())
            return QVariant();
          return id.toString() + "::";*/
        }

        case CodeCompletionModel::Arguments:
        {
          QString ret;
          
          if(completingTemplateParameters())
            createTemplateArgumentList(*this, ret, 0);
          
          if (dec->type<FunctionType>()) {
            needCachedArgumentList();
            
            return m_cachedArgumentList->text;
          }
          
          return ret;
        }
        break;
        case CodeCompletionModel::Name:
        {
          if(alternativeText.isEmpty())
            alternativeText = declarationName();
          return QString(prefixText + alternativeText);
        }
        case CodeCompletionModel::Postfix:
        {
            if (FunctionType::Ptr functionType = m_declaration->type<FunctionType>()) {
              // Retrieve const/volatile string
              return functionType->AbstractType::toString();
            }
            return QVariant();
        }
      }
      break;
    case CodeCompletionModel::HighlightingMethod:
    if( index.column() == CodeCompletionModel::Arguments ) {
//       if( completionContext()->memberAccessOperation() == Cpp::CodeCompletionContext::FunctionCallAccess ) {
        return QVariant(CodeCompletionModel::CustomHighlighting);
/*      }else{
        return QVariant();
      }
      break;*/
    } /*else if(index.column() == CodeCompletionModel::Name) {
      return QVariant(CodeCompletionModel::CustomHighlighting);
    }*/

    break;

    case CodeCompletionModel::CustomHighlight:
    if( index.column() == CodeCompletionModel::Arguments /*&& completionContext()->memberAccessOperation() == Cpp::CodeCompletionContext::FunctionCallAccess*/ ) {
      needCachedArgumentList();
      
      return QVariant(m_cachedArgumentList->highlighting);
    }
//     if( index.column() == CodeCompletionModel::Name ) {
//       //Bold
//       QTextCharFormat boldFormat;
//       boldFormat.setFontWeight((QFont::Normal + QFont::DemiBold)/2);
// 
//       QList<QVariant> ret;
//       ret << 0;
//       ret << nameForDeclaration(dec).length();
//       ret << QVariant(boldFormat);
// 
//       return QVariant(ret);
//     }
    break;
    case Qt::DecorationRole:
     {
      CodeCompletionModel::CompletionProperties p = completionProperties();

      //If it's a signal, remove t he protected flag when computing the decoration. Signals are always protected, and this will give a nicer icon.
      if(p & CodeCompletionModel::Signal)
        p = CodeCompletionModel::Signal;
      //If it's a slot, remove all flags except the slot flag, because that will give a nicer icon. Access-rights are checked anyway.
      if(p & CodeCompletionModel::Slot)
        p = CodeCompletionModel::Slot;
      
      
      if( index.column() == CodeCompletionModel::Icon ) {
        lock.unlock();
        return DUChainUtils::iconForProperties(p);
      }
      break;
    }
    default:
      return KDevelop::NormalDeclarationCompletionItem::data(index, role, model);
  }
  return QVariant();
}

void NormalDeclarationCompletionItem::needCachedArgumentList() const
{
  if(!m_cachedArgumentList)
  {
    m_cachedArgumentList = QExplicitlySharedDataPointer<CachedArgumentList>(new CachedArgumentList);
    
    if(!m_declaration)
      return;
    
    if(m_isTemplateCompletion || declarationNeedsTemplateParameters(m_declaration.data()))
      createTemplateArgumentList(*this, m_cachedArgumentList->text, &m_cachedArgumentList->highlighting);


    if (m_declaration->type<FunctionType>())
      createArgumentList(*this, m_cachedArgumentList->text, &m_cachedArgumentList->highlighting);
  }
}

QWidget* NormalDeclarationCompletionItem::createExpandingWidget(const KDevelop::CodeCompletionModel* model) const
{
  return new Cpp::NavigationWidget(m_declaration, model->currentTopContext());
}

bool NormalDeclarationCompletionItem::createsExpandingWidget() const
{
  return true;
}

QExplicitlySharedDataPointer<CodeCompletionContext> NormalDeclarationCompletionItem::completionContext() const {
  return CodeCompletionContext::Ptr(static_cast<CodeCompletionContext*>(m_completionContext.data()));
}

void IncludeFileCompletionItem::execute(KTextEditor::View* view, const KTextEditor::Range& _word)
{
  KTextEditor::Document* document = view->document();

  KTextEditor::Range word(_word);

  QString newText = includeItem.isDirectory ? includeItem.name + '/' : includeItem.name;

  if(!includeItem.isDirectory) {
    //Add suffix and newline
    QString lineText = document->line( word.end().line() ).trimmed();
    if(lineText.startsWith("#include")) {
      lineText = lineText.mid(8).trimmed();
      if(lineText.startsWith('"'))
        newText += '\"';
      else if(lineText.startsWith('<'))
       newText += '>';
    }

    word.setEnd(KTextEditor::Cursor(word.end().line(), document->lineLength(word.end().line()) ));
  }

  document->replaceText(word, newText);
}

void TypeConversionCompletionItem::setPrefix(QString s) {
  m_prefix = s;
}

QList<KDevelop::IndexedType> TypeConversionCompletionItem::type() const {
  return QList<KDevelop::IndexedType>() << m_type;
}

void TypeConversionCompletionItem::execute(KTextEditor::View* view, const KTextEditor::Range& word) {
  if(argumentHintDepth() == 0)
    view->document()->replaceText( word, m_text );
}

QVariant TypeConversionCompletionItem::data(const QModelIndex& index, int role, const KDevelop::CodeCompletionModel* /*model*/) const
{
  switch (role) {
    case Qt::DisplayRole:
      switch (index.column()) {
        case CodeCompletionModel::Prefix:
          return QVariant(m_prefix);
        case CodeCompletionModel::Name: {
          return m_text;
        }
      }
      break;
    case CodeCompletionModel::ItemSelected:
      return QVariant();
  }

  return QVariant();
}

int TypeConversionCompletionItem::argumentHintDepth() const {
  return m_argumentHintDepth;
}

TypeConversionCompletionItem::TypeConversionCompletionItem(QString text, KDevelop::IndexedType type, int argumentHintDepth, QExplicitlySharedDataPointer<Cpp::CodeCompletionContext> _completionContext) : m_text(text), m_type(type), m_argumentHintDepth(argumentHintDepth), completionContext(_completionContext) {
}

MoreArgumentHintsCompletionItem::MoreArgumentHintsCompletionItem(KDevelop::CodeCompletionContext::Ptr context, QString text, uint oldNumber) : NormalDeclarationCompletionItem(DeclarationPointer(), context) {
  alternativeText = text;
  m_oldNumber = oldNumber;
}

namespace {
  const uint defaultMaxArgumentHints = 8;
  const uint maxArgumentHintsExtensionSteps = 20;
  uint currentMaxArgumentHints = defaultMaxArgumentHints;
};

uint MoreArgumentHintsCompletionItem::resetMaxArgumentHints(bool isAutomaticCompletion)
{
  uint ret = currentMaxArgumentHints;
  currentMaxArgumentHints = defaultMaxArgumentHints;
  
  if(isAutomaticCompletion)
    return 1;
  
  return ret;
}

void MoreArgumentHintsCompletionItem::execute(KTextEditor::View* view, const KTextEditor::Range& word)
{
  currentMaxArgumentHints = m_oldNumber + maxArgumentHintsExtensionSteps;
  
  // Restart code-completion
  KTextEditor::CodeCompletionInterface* iface = dynamic_cast<KTextEditor::CodeCompletionInterface*>(view);
  Q_ASSERT(iface);
  iface->abortCompletion();
    ///@todo 1. This is a non-public interface, and 2. Completion should be started in "automatic invocation" mode
  QMetaObject::invokeMethod(view, "userInvokedCompletion", Qt::QueuedConnection);
}

}

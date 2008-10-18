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

#include "completionitem.h"
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <ktexteditor/range.h>
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <cpptypes.h>
#include <QModelIndex>
#include "completionhelpers.h"
#include "cppcodecompletionmodel.h"
#include <language/duchain/declaration.h>
#include <language/duchain/classfunctiondeclaration.h>
#include <language/duchain/namespacealiasdeclaration.h>
#include "cppduchain/navigation/navigationwidget.h"
#include <language/duchain/duchainutils.h>

using namespace KDevelop;

///@todo Implement a proper duchain based shortening-scheme, and use it throughout the completion
//If this is true, the return-values of argument-hints will be just written as "..." if they are too long
const bool shortenArgumentHintReturnValues = true;
const int maximumArgumentHintReturnValueLength = 30;

void NormalDeclarationCompletionItem::execute(KTextEditor::Document* document, const KTextEditor::Range& word) {
  bool spaceBeforeParen = false; ///@todo Take this from some astyle config or something
  bool spaceBetweenParens = true;
  bool spaceBetweenEmptyParens = false;

  if( completionContext && completionContext->depth() != 0 )
    return; //Do not replace any text when it is an argument-hint

  QString newText;

  if(!useAlternativeText) {
    KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
    if(m_declaration) {
      newText = m_declaration->identifier().toString();
    } else {
      kDebug() << "Declaration disappeared";
      return;
    }
  }else{
    newText = alternativeText;
  }

  document->replaceText(word, newText);

  if( !useAlternativeText && m_declaration && dynamic_cast<AbstractFunctionDeclaration*>(m_declaration.data()) ) { //Do some intelligent stuff for functions with the parens:
    KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
    bool haveArguments = false;
    if( m_declaration && m_declaration->type<FunctionType>() && m_declaration->type<FunctionType>()->indexedArgumentsSize() )
      haveArguments = true;
    //Need to have a paren behind
    QString suffix = document->text( KTextEditor::Range( word.end(), word.end() + KTextEditor::Cursor(1, 0) ) );
    if( suffix.trimmed().startsWith("(") ) {
      //Move the cursor behind the opening paren
      if( document->activeView() )
        document->activeView()->setCursorPosition( word.end() + KTextEditor::Cursor( 0, suffix.indexOf('(')+1 ) );
    }else{
      //We need to insert an opening paren
      QString openingParen;
      if( spaceBeforeParen )
        openingParen = " (";
      else
        openingParen = "(";

      if( spaceBetweenParens && (haveArguments || spaceBetweenEmptyParens) )
        openingParen += " ";

      QString closingParen;
      if( spaceBetweenParens && (haveArguments) ) {
        closingParen = " )";
      } else
        closingParen = ")";

      KTextEditor::Cursor jumpPos = word.end() + KTextEditor::Cursor( 0, openingParen.length() );

      //If no arguments, move the cursor behind the closing paren
      if( !haveArguments )
        jumpPos += KTextEditor::Cursor( 0, closingParen.length() );

      lock.unlock();
      document->insertText( word.end(), openingParen + closingParen );
      if( document->activeView() )
        document->activeView()->setCursorPosition( jumpPos );
    }
  }
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
    if (type->modifiers() & AbstractType::VolatileModifier)
      ;//TODO

    switch (dec->abstractType()->whichType()) {
      case AbstractType::TypeIntegral:
        if (dec->type<EnumerationType>()) {
          // Remove variable bit set in DUChainUtils
          p &= ~CodeCompletionModel::Variable;
          p |= CodeCompletionModel::Enum;
        }
        if (dec->type<EnumeratorType>()) {
          //Get the properties from the parent, because that may contain information like "private"
          if(dec->context()->owner()) {
            p = DUChainUtils::completionProperties(dec->context()->owner());
          }
          // Remove variable bit set in DUChainUtils
          p &= 0xffffffff - CodeCompletionModel::Variable;
          p |= CodeCompletionModel::Enum;
        }
        break;
      case AbstractType::TypeStructure:
        if (CppClassType::Ptr classType =  dec->type<CppClassType>())
          switch (classType->classType()) {
            case CppClassType::Class:
              p |= CodeCompletionModel::Class;
              break;
            case CppClassType::Struct:
              // Remove class bit set in DUChainUtils
              p &= ~CodeCompletionModel::Class;
              p |= CodeCompletionModel::Struct;
              break;
            case CppClassType::Union:
              // Remove class bit set in DUChainUtils
              p &= ~CodeCompletionModel::Class;
              p |= CodeCompletionModel::Union;
              break;
          }
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


QVariant NormalDeclarationCompletionItem::data(const QModelIndex& index, int role, const CodeCompletionModel* model) const {

  DUChainReadLocker lock(DUChain::lock(), 500);
  if(!lock.locked()) {
    kDebug(9007) << "Failed to lock the du-chain in time";
    return QVariant();
  }

  static CompletionTreeItemPointer currentMatchContext;


  //Stuff that does not require a declaration:
  switch (role) {
    case CodeCompletionModel::SetMatchContext:
      currentMatchContext = CompletionTreeItemPointer(const_cast<NormalDeclarationCompletionItem*>(this));
      return QVariant(1);
  };

  if(!m_declaration) {
    if(role == Qt::DisplayRole && index.column() == CodeCompletionModel::Name)
      return alternativeText;
    return QVariant();
  }else if(index.column() == CodeCompletionModel::Name && useAlternativeText)
    return alternativeText;

  Declaration* dec = const_cast<Declaration*>( m_declaration.data() );

  switch (role) {
    case CodeCompletionModel::BestMatchesCount:
      return QVariant(5);
    break;
    case CodeCompletionModel::MatchQuality:
    {
      if( currentMatchContext && currentMatchContext->asItem()) {
        const NormalDeclarationCompletionItem& contextItem(*currentMatchContext->asItem<NormalDeclarationCompletionItem>());
        if( contextItem.asItem() && contextItem.m_declaration && contextItem.completionContext && contextItem.completionContext->memberAccessOperation() == Cpp::CodeCompletionContext::FunctionCallAccess && contextItem.listOffset < contextItem.completionContext->functions().count() )
        {
          Cpp::CodeCompletionContext::Function f( contextItem.completionContext->functions()[contextItem.listOffset] );

          if( f.function.isValid() && f.function.isViable() && f.function.declaration() && f.function.declaration()->type<FunctionType>() && f.function.declaration()->type<FunctionType>()->indexedArgumentsSize() > f.matchedArguments ) {
            Cpp::TypeConversion conv(model->currentTopContext().data());

            ///@todo fill the lvalue-ness correctly
            int quality = ( conv.implicitConversion( effectiveType(dec)->indexed(), f.function.declaration()->type<FunctionType>()->indexedArguments()[f.matchedArguments], true )  * 10 ) / Cpp::MaximumConversionResult;
            return QVariant(quality);
          }else{
            //kDebug(9007) << "MatchQuality requested with invalid match-context";
          }
        } else {
          //kDebug(9007) << "MatchQuality requested with invalid match-context";
        }
      }
    }
    return QVariant();
    case CodeCompletionModel::ItemSelected:
       return QVariant(Cpp::NavigationWidget::shortDescription(dec));
    case CodeCompletionModel::IsExpandable:
      return QVariant(true);
    case CodeCompletionModel::ExpandingWidget: {
      Cpp::NavigationWidget* nav = new Cpp::NavigationWidget(DeclarationPointer(dec), model->currentTopContext());
      model->addNavigationWidget(this, nav);

      QVariant v;
      v.setValue<QWidget*>((QWidget*)nav);
      return v;
    }
    case Qt::DisplayRole:
      switch (index.column()) {
        case CodeCompletionModel::Prefix:
        {
          int depth = m_inheritanceDepth;
          if( depth >= 1000 )
            depth-=1000;
          QString indentation;
          if(indentByDepth)
            indentation = QString(depth, ' ');

          if( NamespaceAliasDeclaration* alias = dynamic_cast<NamespaceAliasDeclaration*>(dec) ) {
            if( alias->identifier().isEmpty() ) {
              return indentation + "using namespace";/* " + alias->importIdentifier().toString();*/
            } else {
              return indentation + "namespace";/* " + alias->identifier().toString() + " = " + alias->importIdentifier().toString();*/
            }
          }

          if( dec->isTypeAlias() )
            indentation += "typedef ";

          if( dec->kind() == Declaration::Type && !dec->type<FunctionType>() && !dec->isTypeAlias() ) {
            if (CppClassType::Ptr classType =  dec->type<CppClassType>())
              switch (classType->classType()) {
                case CppClassType::Class:
                  return indentation + "class";
                  break;
                case CppClassType::Struct:
                  return indentation + "struct";
                  break;
                case CppClassType::Union:
                  return indentation + "union";
                  break;
              }
              if(dec->type<EnumerationType>()) {
                return "enum";
              }
            return QVariant();
          }
          if (dec->abstractType()) {
            if(EnumeratorType::Ptr enumerator = dec->type<EnumeratorType>()) {
              if(dec->context()->owner() && dec->context()->owner()->abstractType()) {
                if(!dec->context()->owner()->identifier().isEmpty())
                  return dec->context()->owner()->abstractType()->toString();
                else
                  return "enum";
              }
            }
            if (FunctionType::Ptr functionType = dec->type<FunctionType>()) {
              ClassFunctionDeclaration* funDecl = dynamic_cast<ClassFunctionDeclaration*>(dec);

              if (functionType->returnType()) {
                QString ret = indentation + functionType->returnType()->toString();
                if(shortenArgumentHintReturnValues && argumentHintDepth() && ret.length() > maximumArgumentHintReturnValueLength)
                  return QString("...");
                else
                  return ret;
              }else if(funDecl && funDecl->isConstructor() )
                return indentation + "<constructor>";
              else if(funDecl && funDecl->isDestructor() )
                return indentation + "<destructor>";
              else
                return indentation + "<incomplete type>";

            } else {
              QString ret = indentation;
              return  ret + dec->abstractType()->toString();
            }
          } else {
            return indentation + "<incomplete type>";
          }
        }

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

        case CodeCompletionModel::Name:
          return nameForDeclaration(dec);

        case CodeCompletionModel::Arguments:
          if (FunctionType::Ptr functionType = dec->type<FunctionType>()) {
            QString ret;
            createArgumentList(*this, ret, 0);
            return ret;
          }
        break;
        case CodeCompletionModel::Postfix:
          if (FunctionType::Ptr functionType = dec->type<FunctionType>()) {
            // Retrieve const/volatile string
            return functionType->AbstractType::toString();
          }
          break;
      }
      break;
    case CodeCompletionModel::HighlightingMethod:
    if( index.column() == CodeCompletionModel::Arguments ) {
      if( completionContext->memberAccessOperation() == Cpp::CodeCompletionContext::FunctionCallAccess ) {
        return QVariant(CodeCompletionModel::CustomHighlighting);
      }else{
        return QVariant();
      }
      break;
    } else if(index.column() == CodeCompletionModel::Name) {
      return QVariant(CodeCompletionModel::CustomHighlighting);
    }

    break;

    case CodeCompletionModel::CustomHighlight:
    if( index.column() == CodeCompletionModel::Arguments && completionContext->memberAccessOperation() == Cpp::CodeCompletionContext::FunctionCallAccess ) {
      QString ret;
      QList<QVariant> highlight;
      createArgumentList(*this, ret, &highlight);
      return QVariant(highlight);
    }
    if( index.column() == CodeCompletionModel::Name ) {
      //Bold
      QTextCharFormat boldFormat;
      boldFormat.setFontWeight(QFont::Bold);

      QList<QVariant> ret;
      ret << 0;
      ret << nameForDeclaration(dec).length();
      ret << QVariant(boldFormat);

      return QVariant(ret);
    }
    break;
    case CodeCompletionModel::CompletionRole:
      return (int)completionProperties();
    case Qt::DecorationRole:
     {
      CodeCompletionModel::CompletionProperties p = completionProperties();

      if( index.column() == CodeCompletionModel::Icon ) {
        lock.unlock();
        return DUChainUtils::iconForProperties(p);
      }
      break;
    }

    case CodeCompletionModel::ScopeIndex:
      return static_cast<int>(reinterpret_cast<long>(dec->context()));
  }

  return QVariant();
}

int NormalDeclarationCompletionItem::inheritanceDepth() const
{
  return m_inheritanceDepth;
}

int IncludeFileCompletionItem::inheritanceDepth() const
{
  return includeItem.pathNumber;
}

int NormalDeclarationCompletionItem::argumentHintDepth() const
{
  if( completionContext && completionContext->memberAccessOperation() == Cpp::CodeCompletionContext::FunctionCallAccess )
      return completionContext->depth();
    else
      return 0;
}

int IncludeFileCompletionItem::argumentHintDepth() const
{
  return 0;
}

QVariant IncludeFileCompletionItem::data(const QModelIndex& index, int role, const CodeCompletionModel* model) const
{
  DUChainReadLocker lock(DUChain::lock(), 500);
  if(!lock.locked()) {
    kDebug(9007) << "Failed to lock the du-chain in time";
    return QVariant();
  }

  const Cpp::IncludeItem& item( includeItem );

  switch (role) {
    case CodeCompletionModel::IsExpandable:
      return QVariant(true);
    case CodeCompletionModel::ExpandingWidget: {
      Cpp::NavigationWidget* nav = new Cpp::NavigationWidget(item, model->currentTopContext());
      model->addNavigationWidget(this, nav);

       QVariant v;
       v.setValue<QWidget*>((QWidget*)nav);
       return v;
    }
    case Qt::DisplayRole:
      switch (index.column()) {
        case CodeCompletionModel::Prefix:
          if(item.isDirectory)
            return QVariant("directory");
          else
            return QVariant("file");
        case CodeCompletionModel::Name: {
          return item.isDirectory ? item.name + "/" : item.name;
        }
      }
      break;
    case CodeCompletionModel::ItemSelected:
    {
      return QVariant( Cpp::NavigationWidget::shortDescription(item) );
    }
  }

  return QVariant();
}

void IncludeFileCompletionItem::execute(KTextEditor::Document* document, const KTextEditor::Range& _word) {

  KTextEditor::Range word(_word);

  QString newText = includeItem.isDirectory ? includeItem.name + "/" : includeItem.name;

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

    word.end().setColumn( document->lineLength(word.end().line()) );
  }

  document->replaceText(word, newText);
  ///@todo Make this more intelligent, also add a closing '"' or '>'
}

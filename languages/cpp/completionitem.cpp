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
#include <duchain.h>
#include <duchainlock.h>
#include <ktexteditor/codecompletionmodel.h>
#include <ktexteditor/range.h>
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <cpptypes.h>
#include <QModelIndex>
#include "completionhelpers.h"
#include "cppcodecompletionmodel.h"
#include <duchain/declaration.h>
#include <duchain/classfunctiondeclaration.h>
#include <duchain/namespacealiasdeclaration.h>
#include "navigationwidget.h"
#include "duchainutils.h"


using namespace KTextEditor;
using namespace KDevelop;

///Intermediate nodes
class CompletionTreeNode;
///Leaf items
class CompletionTreeItem;

CompletionTreeElement::CompletionTreeElement() : m_parent(0), m_rowInParent(0) {
}

CompletionTreeElement::~CompletionTreeElement() {
}

CompletionTreeElement* CompletionTreeElement::parent() const {
  return m_parent;
}

void CompletionTreeElement::setParent(CompletionTreeElement* parent) {
    Q_ASSERT(m_parent == 0);

    m_parent = parent;
    CompletionTreeNode* node = parent->asNode();
    if( node ) {
      m_rowInParent = node->children.count();
    }
}

int CompletionTreeElement::columnInParent() const {
  return 0;
}

CompletionTreeNode::CompletionTreeNode() : CompletionTreeElement() {
}
CompletionTreeNode::~CompletionTreeNode() {
}
  
CompletionTreeNode* CompletionTreeElement::asNode() {
  return dynamic_cast<CompletionTreeNode*>(this);
}

CompletionTreeItem* CompletionTreeElement::asItem() {
  return dynamic_cast<CompletionTreeItem*>(this);
}

const CompletionTreeNode* CompletionTreeElement::asNode() const {
  return dynamic_cast<const CompletionTreeNode*>(this);
}

const CompletionTreeItem* CompletionTreeElement::asItem() const {
  return dynamic_cast<const CompletionTreeItem*>(this);
}

int CompletionTreeElement::rowInParent() const {
  return m_rowInParent;
/*  if( !m_parent )
    return 0;
  Q_ASSERT(m_parent->asNode());
  
  return m_parent->asNode()->children.indexOf( KSharedPtr<CompletionTreeElement>(const_cast<CompletionTreeElement*>(this)) );*/
}

void CompletionTreeItem::execute(KTextEditor::Document* document, const KTextEditor::Range& word) {
  kDebug(9700) << "doing nothing";
}

void NormalDeclarationCompletionItem::execute(KTextEditor::Document* document, const KTextEditor::Range& word) {
  bool spaceBeforeParen = false; ///@todo Take this from some astyle config or something
  bool spaceBetweenParens = true;
  bool spaceBetweenEmptyParens = false;
  
  if( completionContext && completionContext->depth() != 0 )
    return; //Do not replace any text when it is an argument-hint

  QString newText;

  {
    KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
    if(declaration) {
      newText = declaration->identifier().toString();
    } else {
      kDebug() << "Declaration disappeared";
      return;
    }
  }

  document->replaceText(word, newText);

  if( declaration && dynamic_cast<AbstractFunctionDeclaration*>(declaration.data()) ) { //Do some intelligent stuff for functions with the parens:
    KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
    bool haveArguments = false;
    if( declaration && declaration->type<CppFunctionType>() && declaration->type<CppFunctionType>()->arguments().count() )
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

QVariant CompletionTreeItem::data(const QModelIndex& index, int role, const CppCodeCompletionModel* model) const {
  if(role = Qt::DisplayRole)
    return QString("not implemented");
  return QVariant();
}

//The name to be viewed in the name column
QString nameForDeclaration(Declaration* dec) {
  if (dec->identifier().toString().isEmpty())
    return "<unknown>";
  else
    return dec->identifier().toString();
}

QVariant NormalDeclarationCompletionItem::data(const QModelIndex& index, int role, const CppCodeCompletionModel* model) const {

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

  if(!declaration) {
    if(role == Qt::DisplayRole && index.column() == CodeCompletionModel::Name)
      return alternativeText;
    return QVariant();
  }
  
  Declaration* dec = const_cast<Declaration*>( declaration.data() );

  switch (role) {
    case CodeCompletionModel::BestMatchesCount:
      return QVariant(5);
    break;
    case CodeCompletionModel::MatchQuality:
    {
      if( currentMatchContext && currentMatchContext->asItem()) {
        const NormalDeclarationCompletionItem& contextItem(*currentMatchContext->asItem<NormalDeclarationCompletionItem>());
        if( contextItem.asItem() && contextItem.declaration && contextItem.completionContext && contextItem.completionContext->memberAccessOperation() == Cpp::CodeCompletionContext::FunctionCallAccess && contextItem.listOffset < contextItem.completionContext->functions().count() )
        {
          Cpp::CodeCompletionContext::Function f( contextItem.completionContext->functions()[contextItem.listOffset] );

          if( f.function.isValid() && f.function.isViable() && f.function.declaration() && f.function.declaration()->type<CppFunctionType>() && f.function.declaration()->type<CppFunctionType>()->arguments().count() > f.matchedArguments ) {
            Cpp::TypeConversion conv(model->currentTopContext().data());

            ///@todo fill the lvalue-ness correctly
            int quality = ( conv.implicitConversion( effectiveType(dec), f.function.declaration()->type<CppFunctionType>()->arguments()[f.matchedArguments], true )  * 10 ) / Cpp::MaximumConversionResult;
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

          if( dec->kind() == Declaration::Type && !dec->type<CppFunctionType>() && !dec->isTypeAlias() ) {
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
            return QVariant();
          }
          if (dec->abstractType()) {
            if (CppFunctionType::Ptr functionType = dec->type<CppFunctionType>()) {
              ClassFunctionDeclaration* funDecl = dynamic_cast<ClassFunctionDeclaration*>(dec);

              if (functionType->returnType())
                return indentation + functionType->returnType()->toString();
              else if(funDecl && funDecl->isConstructor() )
                return indentation + "<constructor>";
              else if(funDecl && funDecl->isDestructor() )
                return indentation + "<destructor>";
              else
                return indentation + "<incomplete type>";

            } else {
              QString ret = indentation;
              if(dec->type<CppEnumeratorType>())
                ret += "enumerator ";
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
          if (CppFunctionType::Ptr functionType = dec->type<CppFunctionType>()) {
            QString ret;
            createArgumentList(*this, ret, 0);
            return ret;
          }
        break;
        case CodeCompletionModel::Postfix:
          if (CppFunctionType::Ptr functionType = dec->type<CppFunctionType>()) {
            return functionType->cvString();
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
    case Qt::DecorationRole:
    case CodeCompletionModel::CompletionRole: {
      CodeCompletionModel::CompletionProperties p = DUChainUtils::completionProperties(dec);

      if (dec->abstractType()) {
        if (CppCVType* cv = dynamic_cast<CppCVType*>(dec->abstractType().data())) {
          if (cv->isConstant())
            p |= CodeCompletionModel::Const;
          if (cv->isVolatile())
            ;//TODO
          }

        switch (dec->abstractType()->whichType()) {
          case AbstractType::TypeIntegral:
            if (dec->type<CppEnumerationType>()) {
              // Remove variable bit set in DUChainUtils
              p &= ~CodeCompletionModel::Variable;
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

      if( role == CodeCompletionModel::CompletionRole ) {
        return (int)p;

      } else {
        if( index.column() == CodeCompletionModel::Icon ) {
          lock.unlock();
          return DUChainUtils::iconForProperties(p);
        }
        break;

      }
    }

    case CodeCompletionModel::ScopeIndex:
      return static_cast<int>(reinterpret_cast<long>(dec->context()));
  }

  return QVariant();
}

int CompletionTreeItem::inheritanceDepth() const
{
  return 0;
}

int NormalDeclarationCompletionItem::inheritanceDepth() const
{
  return m_inheritanceDepth;
}

int IncludeFileCompletionItem::inheritanceDepth() const
{
  return includeItem.pathNumber;
}

int CompletionTreeItem::argumentHintDepth() const
{
  return 0;
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

QVariant IncludeFileCompletionItem::data(const QModelIndex& index, int role, const CppCodeCompletionModel* model) const
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

void IncludeFileCompletionItem::execute(KTextEditor::Document* document, const KTextEditor::Range& word) {

  QString newText = includeItem.isDirectory ? includeItem.name + "/" : includeItem.name;

  document->replaceText(word, newText);
  ///@todo Make this more intelligent, also add a closing '"' or '>'
}
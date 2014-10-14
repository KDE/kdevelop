/*
 * KDevelop Generic Code Completion Support
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

#include "normaldeclarationcompletionitem.h"
#include "codecompletionmodel.h"
#include "../duchain/duchainlock.h"
#include "../duchain/duchain.h"
#include "../duchain/classfunctiondeclaration.h"
#include "../duchain/types/functiontype.h"
#include "../duchain/types/enumeratortype.h"
#include "../duchain/duchainutils.h"
#include "util/debug.h"

#include <KTextEditor/Document>
#include <KTextEditor/View>


namespace KDevelop {

const int NormalDeclarationCompletionItem::normalBestMatchesCount = 5;
//If this is true, the return-values of argument-hints will be just written as "..." if they are too long
const bool NormalDeclarationCompletionItem::shortenArgumentHintReturnValues = true;
const int NormalDeclarationCompletionItem::maximumArgumentHintReturnValueLength = 30;
const int NormalDeclarationCompletionItem::desiredTypeLength = 20;

NormalDeclarationCompletionItem::NormalDeclarationCompletionItem(KDevelop::DeclarationPointer decl, QExplicitlySharedDataPointer<CodeCompletionContext> context, int inheritanceDepth)
  : m_completionContext(context), m_declaration(decl), m_inheritanceDepth(inheritanceDepth) {
}

KDevelop::DeclarationPointer NormalDeclarationCompletionItem::declaration() const {
  return m_declaration;
}

QExplicitlySharedDataPointer< KDevelop::CodeCompletionContext > NormalDeclarationCompletionItem::completionContext() const {
  return m_completionContext;
}

int NormalDeclarationCompletionItem::inheritanceDepth() const
{
  return m_inheritanceDepth;
}

int NormalDeclarationCompletionItem::argumentHintDepth() const
{
  if( m_completionContext )
      return m_completionContext->depth();
    else
      return 0;
}

QString NormalDeclarationCompletionItem::declarationName() const
{
  QString ret = m_declaration->identifier().toString();
  if (ret.isEmpty())
    return "<unknown>";
  else
    return ret;
}

void NormalDeclarationCompletionItem::execute(KTextEditor::View* view, const KTextEditor::Range& word) {

  if( m_completionContext && m_completionContext->depth() != 0 )
    return; //Do not replace any text when it is an argument-hint

  KTextEditor::Document* document = view->document();
  QString newText;

  {
    KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
    if(m_declaration) {
      newText = declarationName();
    } else {
      qCDebug(LANGUAGE) << "Declaration disappeared";
      return;
    }
  }

  document->replaceText(word, newText);
  KTextEditor::Range newRange = word;
  newRange.setEnd(KTextEditor::Cursor(newRange.end().line(), newRange.start().column() + newText.length()));

  executed(view, newRange);
}

QWidget* NormalDeclarationCompletionItem::createExpandingWidget(const KDevelop::CodeCompletionModel* model) const
{
  Q_UNUSED(model);
  return 0;
}

bool NormalDeclarationCompletionItem::createsExpandingWidget() const
{
  return false;
}

QString NormalDeclarationCompletionItem::shortenedTypeString(KDevelop::DeclarationPointer decl, int desiredTypeLength) const
{
  Q_UNUSED(desiredTypeLength);
  return decl->abstractType()->toString();
}

void NormalDeclarationCompletionItem::executed(KTextEditor::View* view, const KTextEditor::Range& word)
{
  Q_UNUSED(view);
  Q_UNUSED(word);
}

QVariant NormalDeclarationCompletionItem::data(const QModelIndex& index, int role, const KDevelop::CodeCompletionModel* model) const
{
  DUChainReadLocker lock(DUChain::lock(), 500);
  if(!lock.locked()) {
    qCDebug(LANGUAGE) << "Failed to lock the du-chain in time";
    return QVariant();
  }

  if(!m_declaration)
    return QVariant();

  switch (role) {
    case Qt::DisplayRole:
      if (index.column() == CodeCompletionModel::Name) {
        return declarationName();
      } else if(index.column() == CodeCompletionModel::Postfix) {
          if (FunctionType::Ptr functionType = m_declaration->type<FunctionType>()) {
            // Retrieve const/volatile string
            return functionType->AbstractType::toString();
          }
      } else if(index.column() == CodeCompletionModel::Prefix) {
          if(m_declaration->kind() == Declaration::Namespace)
            return QString("namespace");
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
        }
      break;
    case CodeCompletionModel::BestMatchesCount:
      return QVariant(normalBestMatchesCount);
    break;
    case CodeCompletionModel::IsExpandable:
      return QVariant(createsExpandingWidget());
    case CodeCompletionModel::ExpandingWidget: {
      QWidget* nav = createExpandingWidget(model);
      Q_ASSERT(nav);
      model->addNavigationWidget(this, nav);

      QVariant v;
      v.setValue<QWidget*>(nav);
      return v;
    }
    case CodeCompletionModel::ScopeIndex:
      return static_cast<int>(reinterpret_cast<quintptr>(m_declaration->context()));

    case CodeCompletionModel::CompletionRole:
      return (int)completionProperties();
    case Qt::DecorationRole:
     {
      if( index.column() == CodeCompletionModel::Icon ) {
        CodeCompletionModel::CompletionProperties p = completionProperties();
        lock.unlock();
        return DUChainUtils::iconForProperties(p);
      }
      break;
    }
  }
  return QVariant();
}

}


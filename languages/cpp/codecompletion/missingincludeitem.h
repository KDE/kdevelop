/*
   Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef MISSINGINCLUDECOMPLETIONITEM_H
#define MISSINGINCLUDECOMPLETIONITEM_H

#include "item.h"

namespace Cpp {

class ExpressionEvaluationResult;

class MissingIncludeCompletionItem : public KDevelop::CompletionTreeItem
{
public:
  MissingIncludeCompletionItem(const QString& addedInclude, const QString& canonicalFile,
                               const QString& displayTextPrefix,
                               const KDevelop::IndexedDeclaration& decl, int argumentHintDepth);

  virtual QVariant data(const QModelIndex& index, int role, const KDevelop::CodeCompletionModel* model) const;

  virtual void execute(KTextEditor::Document* document, const KTextEditor::Range& word);

  virtual int inheritanceDepth() const;
  virtual int argumentHintDepth() const {
    return m_argumentHintDepth;
  }

  QString lineToInsert() const;

  int m_argumentHintDepth;
  QString m_addedInclude;
  QString m_canonicalPath;
  QString m_displayTextPrefix;
  KDevelop::IndexedDeclaration m_decl;
};

class ForwardDeclarationItem : public NormalDeclarationCompletionItem {
  public:
  ForwardDeclarationItem(KDevelop::DeclarationPointer decl);
  virtual void execute(KTextEditor::Document* document, const KTextEditor::Range& word);
  virtual QVariant data(const QModelIndex& index, int role, const KDevelop::CodeCompletionModel* model) const;
};

///DUChain must be locked
///@param displayTextPrefix may be needed so the created items pass a specific filtering in the completion-list
QList<KDevelop::CompletionTreeItemPointer> missingIncludeCompletionItems(const QString& expression,
                                                                         const QString& displayTextPrefix,
                                                                         const Cpp::ExpressionEvaluationResult& expressionResult,
                                                                         const KDevelop::DUContextPointer& context,
                                                                         int argumentHintDepth = 0,
                                                                         bool needInstance = false);

///DUChain must be locked
QExplicitlySharedDataPointer<MissingIncludeCompletionItem> includeDirectiveFromUrl(const KUrl& fromUrl, const KDevelop::IndexedDeclaration& decl);

}

#endif

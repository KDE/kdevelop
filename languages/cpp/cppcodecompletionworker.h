/*
 * KDevelop C++ Code Completion Support
 *
 * Copyright 2006-2007 Hamish Rodda <rodda@kde.org>
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

#ifndef KDEVCPPCODECOMPLETIONWORKER_H
#define KDEVCPPCODECOMPLETIONWORKER_H

#include <QThread>
#include <QList>

#include "cppcodecompletionmodel.h"

namespace Cpp {
  class CodeCompletionContext;
}

class CompletionTreeNode;
class CompletionTreeItem;

class CompletionTreeElement : public KShared {
public:
  CompletionTreeElement(CompletionTreeElement* _parent);
  
  virtual ~CompletionTreeElement();

  CompletionTreeElement* parent() const;

  int rowInParent() const;
  
  int columnInParent() const;

  ///Each element is either a node, or an item.
  
  CompletionTreeNode* asNode();
  
  CompletionTreeItem* asItem();
  
  const CompletionTreeNode* asNode() const;
  
  const CompletionTreeItem* asItem() const;
  
private:
  CompletionTreeElement* m_parent;
  int m_rowInParent;
};

struct CompletionTreeNode : public CompletionTreeElement {
  CompletionTreeNode(CompletionTreeElement* _parent);
  ~CompletionTreeNode();
  
  KTextEditor::CodeCompletionModel::ExtraItemDataRoles role;
  QVariant roleValue;
  QList<KSharedPtr<CompletionTreeElement> > children;
};

struct CompletionTreeItem : public CompletionTreeElement {
  CompletionTreeItem(CompletionTreeElement* _parent);
  
  CppCodeCompletionModel::CompletionItem item;
};

class CodeCompletionWorker : public QThread
{
  Q_OBJECT

  public:
    CodeCompletionWorker(CppCodeCompletionModel* parent);
    virtual ~CodeCompletionWorker();

    CppCodeCompletionModel* model() const;

    void abortCurrentCompletion();

  Q_SIGNALS:
    void foundDeclarations(QList<KSharedPtr<CompletionTreeElement> >, void* completionContext);

  protected:
    virtual void run();

  private Q_SLOTS:
    void computeCompletions(KDevelop::DUContextPointer context, const KTextEditor::Cursor& position, KTextEditor::View* view);

  private:

    void computeGroups(QList<CppCodeCompletionModel::CompletionItem> items, KSharedPtr<Cpp::CodeCompletionContext> completionContext);
  
    KTextEditor::Cursor m_position;
    bool m_abort;
    QMutex* m_mutex;
};

#endif // KDEVCPPCODECOMPLETIONWORKER_H

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

#ifndef KDEVPLATFORM_KDEV_CODECOMPLETIONITEM_H
#define KDEVPLATFORM_KDEV_CODECOMPLETIONITEM_H

#include <ksharedptr.h>
#include <ktexteditor/codecompletionmodel.h>

#include "../duchain/duchainpointer.h"
#include "codecompletioncontext.h"

namespace KTextEditor {
  class CodeCompletionModel;
  class Range;
  class Cursor;
}

class QModelIndex;

namespace KDevelop
{
class CodeCompletionModel;

struct CompletionTreeNode;
class CompletionTreeItem;
class IndexedType;

class KDEVPLATFORMLANGUAGE_EXPORT CompletionTreeElement : public QSharedData {
public:
  CompletionTreeElement();
  
  virtual ~CompletionTreeElement();

  CompletionTreeElement* parent() const;

  ///Reparenting is not supported. This is only allowed if parent() is still zero.
  void setParent(CompletionTreeElement*);

  int rowInParent() const;
  
  int columnInParent() const;

  ///Each element is either a node, or an item.
  
  CompletionTreeNode* asNode();
  
  CompletionTreeItem* asItem();

  template<class T>
  T* asItem() {
    return dynamic_cast<T*>(this);
  }

  template<class T>
  const T* asItem() const {
    return dynamic_cast<const T*>(this);
  }
  
  const CompletionTreeNode* asNode() const;
  
  const CompletionTreeItem* asItem() const;
  
private:
  CompletionTreeElement* m_parent;
  int m_rowInParent;
};

struct KDEVPLATFORMLANGUAGE_EXPORT CompletionTreeNode : public CompletionTreeElement {
  CompletionTreeNode();
  ~CompletionTreeNode();
  
  KTextEditor::CodeCompletionModel::ExtraItemDataRoles role;
  QVariant roleValue;
  
  ///Will append the child, and initialize it correctly to create a working tree-structure
  void appendChild(QExplicitlySharedDataPointer<CompletionTreeElement>);
  void appendChildren(QList<QExplicitlySharedDataPointer<CompletionTreeElement> >);
  void appendChildren(QList<QExplicitlySharedDataPointer<CompletionTreeItem> >);
  
  ///@warning Do not manipulate this directly, that's bad for consistency. Use appendChild instead.
  QList<QExplicitlySharedDataPointer<CompletionTreeElement> > children;
};

class KDEVPLATFORMLANGUAGE_EXPORT CompletionTreeItem : public CompletionTreeElement
{
public:

  ///Execute the completion item. The default implementation does nothing.
  virtual void execute(KTextEditor::View* view, const KTextEditor::Range& word);

  ///Should return normal completion data, @see KTextEditor::CodeCompletionModel
  ///The default implementation returns "unimplemented", so re-implement it!
  ///The duchain is not locked when this is called
  ///Navigation-widgets should be registered to the model, then it will care about the interaction.
  virtual QVariant data(const QModelIndex& index, int role, const CodeCompletionModel* model) const;
  
  ///Should return the inheritance-depth. The completion-items don't need to return it through the data() function.
  virtual int inheritanceDepth() const;
  ///Should return the argument-hint depth. The completion-items don't need to return it through the data() function.
  virtual int argumentHintDepth() const;

  ///The default-implementation calls DUChainUtils::completionProperties
  virtual KTextEditor::CodeCompletionModel::CompletionProperties completionProperties() const;

  ///If this item represents a Declaration, this should return the declaration.
  ///The default-implementation returns zero.
  virtual DeclarationPointer declaration() const;
  
  ///Should return the types should be used for matching items against this one when it's an argument hint.
  ///The matching against all types should be done, and the best one will be used as final match result.
  virtual QList<IndexedType> typeForArgumentMatching() const;
  
  ///Should return whether this completion-items data changes with input done by the user during code-completion.
  ///Returning true is very expensive.
  virtual bool dataChangedWithInput() const;
};

///A custom-group node, that can be used as-is. Just create it, and call appendChild to add group items.
///The items in the group will be shown in the completion-list with a group-header that contains the given name
struct KDEVPLATFORMLANGUAGE_EXPORT CompletionCustomGroupNode : public CompletionTreeNode {
  ///@param inheritanceDepth @see KTextEditor::CodeCompletionModel::GroupRole
  CompletionCustomGroupNode(QString groupName, int inheritanceDepth = 700);
  
  int inheritanceDepth;
};

typedef QExplicitlySharedDataPointer<CompletionTreeItem> CompletionTreeItemPointer;
typedef QExplicitlySharedDataPointer<CompletionTreeElement> CompletionTreeElementPointer;

}

#endif

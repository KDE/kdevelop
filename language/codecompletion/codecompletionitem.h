/*
 * KDevelop Code Completion Support
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

#ifndef KDEV_CODECOMPLETIONITEM_H
#define KDEV_CODECOMPLETIONITEM_H

#include <ksharedptr.h>
#include <ktexteditor/codecompletionmodel.h>

#include <duchainpointer.h>
#include "codecompletioncontext.h"

namespace KTextEditor {
  class CodeCompletionModel;
  class Document;
  class Range;
  class Cursor;
}

class QModelIndex;

namespace KDevelop
{
class CodeCompletionModel;

class CompletionTreeNode;
class CompletionTreeItem;

class KDEVPLATFORMLANGUAGE_EXPORT CompletionTreeElement : public KShared {
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
  QList<KSharedPtr<CompletionTreeElement> > children;
};

struct KDEVPLATFORMLANGUAGE_EXPORT CompletionTreeItem : public CompletionTreeElement {
  ///Execute the completion item. The default implementation does nothing.
  virtual void execute(KTextEditor::Document* document, const KTextEditor::Range& word);

  ///Should return normal completion data, @see KTextEditor::CodeCompletionModel
  ///The default implementation returns "unimplemented", so re-implement it!
  ///The duchain is not locked when this is called
  ///Navigation-widgets should be registered to the model, then it will care about the interaction.
  virtual QVariant data(const QModelIndex& index, int role, const CodeCompletionModel* model) const;
  
  ///Should return the inheritance-depth. The completion-items don't need to return it through the data() function.
  virtual int inheritanceDepth() const;
  ///Should return the argument-hint depth. The completion-items don't need to return it through the data() function.
  virtual int argumentHintDepth() const;
};

typedef KSharedPtr<CompletionTreeItem> CompletionTreeItemPointer;

}

#endif

/*
 * KDevelop Generic Code Completion Support
 *
 * Copyright 2006-2008 Hamish Rodda <rodda@kde.org>
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

#ifndef KDEVPLATFORM_CODECOMPLETIONITEMGROUPER_H
#define KDEVPLATFORM_CODECOMPLETIONITEMGROUPER_H

#include "codecompletionmodel.h"
#include "codecompletionitem.h"
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>

namespace KDevelop {

///Always the last item of a grouping chain: Only inserts the items
struct CodeCompletionItemLastGrouper {
  CodeCompletionItemLastGrouper(QList<QExplicitlySharedDataPointer<CompletionTreeElement> >& tree, CompletionTreeNode* parent, QList<CompletionTreeItemPointer> items)
  {
    foreach( CompletionTreeItemPointer item, items ) {
      item->setParent(parent);
      tree << QExplicitlySharedDataPointer<CompletionTreeElement>( item.data() );
    }
  }
};

///Helper class that helps us grouping the completion-list. A chain of groupers can be built, by using NextGrouper.
template<class KeyExtractor, class NextGrouper = CodeCompletionItemLastGrouper>
struct CodeCompletionItemGrouper {
  typedef typename KeyExtractor::KeyType KeyType;

  CodeCompletionItemGrouper(QList<QExplicitlySharedDataPointer<CompletionTreeElement> >& tree, CompletionTreeNode* parent, QList<CompletionTreeItemPointer> items)
  {
    typedef QMap<KeyType, QList<CompletionTreeItemPointer> > GroupMap;
    GroupMap groups;

    foreach(const CompletionTreeItemPointer& item, items) {
      KeyType key = KeyExtractor::extract(item);
      typename GroupMap::iterator it = groups.find(key);
      if(it == groups.end())
        it = groups.insert(key, QList<CompletionTreeItemPointer>());

      (*it).append(item);
    }

    for( typename GroupMap::const_iterator it = groups.constBegin(); it != groups.constEnd(); ++it ) {
      QExplicitlySharedDataPointer<CompletionTreeNode> node(new CompletionTreeNode());
      node->setParent(parent);
      node->role = (KTextEditor::CodeCompletionModel::ExtraItemDataRoles)KeyExtractor::Role;
      node->roleValue = QVariant(it.key());

      tree << QExplicitlySharedDataPointer<CompletionTreeElement>( node.data() );
      
      NextGrouper nextGrouper(node->children, node.data(), *it);
    }
  }
};

///Extracts the argument-hint depth from completion-items, to be used in ItemGrouper for grouping by argument-hint depth.
struct ArgumentHintDepthExtractor {
  typedef int KeyType;
  enum { Role = KTextEditor::CodeCompletionModel::ArgumentHintDepth };
  
  static KeyType extract( const CompletionTreeItemPointer& item ) {
    return item->argumentHintDepth();
  }
};

struct InheritanceDepthExtractor {
  typedef int KeyType;
  
  enum { Role = KTextEditor::CodeCompletionModel::InheritanceDepth };
  
  static KeyType extract( const CompletionTreeItemPointer& item ) {
    return item->inheritanceDepth();
  }
};

struct SimplifiedAttributesExtractor {
  typedef int KeyType;
  
  enum { Role = KTextEditor::CodeCompletionModel::CompletionRole };

  static int groupingProperties;
  
  static KeyType extract( const CompletionTreeItemPointer& item ) {
      DUChainReadLocker lock(DUChain::lock());
      return item->completionProperties() & groupingProperties;
  }
};


}

#endif // KDEVPLATFORM_CODECOMPLETIONITEMGROUPER_H

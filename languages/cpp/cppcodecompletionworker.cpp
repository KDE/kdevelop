/*
 * KDevelop C++ Code Completion Support
 *
 * Copyright 2006-2007 Hamish Rodda <rodda@kde.org>
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

#include "cppcodecompletionworker.h"

#include <kdebug.h>

#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <klocale.h>

#include "cppduchain/cppduchain.h"
#include "cppduchain/typeutils.h"

#include "cppduchain/overloadresolutionhelper.h"

#include <declaration.h>
#include <ducontext.h>
#include "cpptypes.h"
#include <duchainlock.h>
#include <duchainbase.h>
#include "dumpchain.h"
#include "codecompletioncontext.h"
#include <duchainutils.h>

using namespace KDevelop;
using namespace TypeUtils;

CppCodeCompletionWorker::CppCodeCompletionWorker(CppCodeCompletionModel* parent)
  : CodeCompletionWorker(parent)
{
}

void CppCodeCompletionWorker::computeCompletions(KDevelop::DUContextPointer context, const KTextEditor::Cursor& position, KTextEditor::View* view, const KTextEditor::Range& contextRange, const QString& contextText)
{
  Cpp::CodeCompletionContext::Ptr completionContext( new Cpp::CodeCompletionContext( context, contextText ) );
  if (CppCodeCompletionModel* m = model())
    m->setCompletionContext(KDevelop::CodeCompletionContext::Ptr::staticCast(completionContext));

  if( completionContext->isValid() ) {
    DUChainReadLocker lock(DUChain::lock());

    if (!context) {
      kDebug(9007) << "Completion context disappeared before completions could be calculated";
      return;
    }

    QList<CompletionTreeItemPointer> items = completionContext->completionItems(SimpleCursor(position), aborting());

    if (aborting())
      return;
    
    computeGroups( items, completionContext );

  } else {
    kDebug(9007) << "setContext: Invalid code-completion context";
  }
}

///Always the last item of a grouping chain: Only inserts the items
struct LastGrouper {
  LastGrouper(QList<KSharedPtr<CompletionTreeElement> >& tree, CompletionTreeNode* parent, QList<CompletionTreeItemPointer> items)
  {
    foreach( CompletionTreeItemPointer item, items ) {
      item->setParent(parent);
      tree << KSharedPtr<CompletionTreeElement>( item.data() );
    }
  }
};

///Helper class that helps us grouping the completion-list. A chain of groupers can be built, by using NextGrouper.
template<class KeyExtractor, class NextGrouper = LastGrouper>
struct ItemGrouper {
  typedef typename KeyExtractor::KeyType KeyType;
  
  ItemGrouper(QList<KSharedPtr<CompletionTreeElement> >& tree, CompletionTreeNode* parent, QList<CompletionTreeItemPointer> items)
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

    for( typename GroupMap::const_iterator it = groups.begin(); it != groups.end(); ++it ) {
      KSharedPtr<CompletionTreeNode> node(new CompletionTreeNode());
      node->setParent(parent);
      node->role = (KTextEditor::CodeCompletionModel::ExtraItemDataRoles)KeyExtractor::Role;
      node->roleValue = QVariant(it.key());

      tree << KSharedPtr<CompletionTreeElement>( node.data() );
      
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
    const NormalDeclarationCompletionItem* decItem = item->asItem<NormalDeclarationCompletionItem>();
    if( decItem && decItem->declaration.data() )
      return DUChainUtils::completionProperties(decItem->declaration.data()) & groupingProperties;
    else
      return 0;
  }
};

///@todo make configurable. These are the attributes that can be respected for grouping.
int SimplifiedAttributesExtractor::groupingProperties = CodeCompletionModel::Public | CodeCompletionModel::Protected | CodeCompletionModel::Private | CodeCompletionModel::Static | CodeCompletionModel::TypeAlias | CodeCompletionModel::Variable | CodeCompletionModel::Class | CodeCompletionModel::GlobalScope | CodeCompletionModel::LocalScope | CodeCompletionModel::GlobalScope | CodeCompletionModel::NamespaceScope;

void CppCodeCompletionWorker::computeGroups(QList<CompletionTreeItemPointer> items, KSharedPtr<Cpp::CodeCompletionContext> completionContext)
{
  kDebug(9007) << "grouping" << items.count() << "completion-items";
  QList<KSharedPtr<CompletionTreeElement> > tree;
  /**
   * 1. Group by argument-hint depth
   * 2. Group by inheritance depth
   * 3. Group by simplified attributes
   * */
  ItemGrouper<ArgumentHintDepthExtractor, ItemGrouper<InheritanceDepthExtractor, ItemGrouper<SimplifiedAttributesExtractor> > > argumentHintDepthGrouper(tree, 0, items);

  emit foundDeclarations( tree, completionContext.data() );
}

CppCodeCompletionModel* CppCodeCompletionWorker::model() const
{
  return const_cast<CppCodeCompletionModel*>(static_cast<const CppCodeCompletionModel*>(parent()));
}

#include "cppcodecompletionworker.moc"

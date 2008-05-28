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

using namespace KTextEditor;
using namespace KDevelop;
using namespace TypeUtils;

//Whether the list of argument-hints should contain all overloaded versions of operators.
//Disabled for now, because there is usually a huge list of overloaded operators.
const int maxOverloadedOperatorArgumentHints = 5;
const int maxOverloadedArgumentHints = 5;


CodeCompletionWorker::CodeCompletionWorker(CppCodeCompletionModel* parent)
  : QThread(parent)
  , m_mutex(new QMutex)
{
}

CodeCompletionWorker::~CodeCompletionWorker()
{
  delete m_mutex;
}

void CodeCompletionWorker::computeCompletions(KDevelop::DUContextPointer context, const KTextEditor::Cursor& position, KTextEditor::View* view)
{
  {
    QMutexLocker lock(m_mutex);
    m_abort = false;
  }

  //Compute the text we should complete on
  KTextEditor::Document* doc = view->document();
  if( !doc ) {
    kDebug(9007) << "No document for completion";
    return;
  }

  KTextEditor::Range range;
  QString text;
  {
    range = KTextEditor::Range(context->range().start.textCursor(), position);
    text = doc->text(range);
  }

  if( text.isEmpty() ) {
    kDebug(9007) << "no text for context";
    return;
  }

  if( position.column() == 0 ) //Seems like when the cursor is a the beginning of a line, kate does not give the \n
    text += '\n';

  Cpp::CodeCompletionContext::Ptr completionContext( new Cpp::CodeCompletionContext( context, text ) );
  if (CppCodeCompletionModel* m = model())
    m->setCompletionContext(completionContext);

  typedef QPair<Declaration*, int> DeclarationDepthPair;

  if( completionContext->isValid() ) {
    DUChainReadLocker lock(DUChain::lock());

    if (!context) {
      kDebug(9007) << "Completion context disappeared before completions could be calculated";
      return;
    }

    if (m_abort)
      return;

    QList<CompletionTreeItemPointer> items;
    
    if( completionContext->memberAccessContainer().isValid() ||completionContext->memberAccessOperation() == Cpp::CodeCompletionContext::StaticMemberChoose )
    {
      QList<DUContext*> containers = completionContext->memberAccessContainers();
      if( !containers.isEmpty() ) {
        foreach(DUContext* ctx, containers) {
          if (m_abort)
            return;

          foreach( const DeclarationDepthPair& decl, Cpp::hideOverloadedDeclarations( ctx->allDeclarations(ctx->range().end, context->topContext(), false ) ) )
            items << CompletionTreeItemPointer( new NormalDeclarationCompletionItem( DeclarationPointer(decl.first), completionContext, decl.second ) );
        }
      } else {
        kDebug(9007) << "setContext: no container-type";
      }
    } else if( completionContext->memberAccessOperation() == Cpp::CodeCompletionContext::IncludeListAccess ) {
      //Include-file completion
      int cnt = 0;
      QList<Cpp::IncludeItem> allIncludeItems = completionContext->includeItems();
      foreach(const Cpp::IncludeItem& includeItem, allIncludeItems) {
        if (m_abort)
          return;

        items << CompletionTreeItemPointer( new IncludeFileCompletionItem(includeItem) );
        ++cnt;
      }
      kDebug(9007) << "Added " << cnt << " include-files to completion-list";
    } else {
      //Show all visible declarations
      QList<DeclarationDepthPair> decls = Cpp::hideOverloadedDeclarations( context->allDeclarations(context->type() == DUContext::Class ? context->range().end : SimpleCursor(position), context->topContext()) );
      foreach( const DeclarationDepthPair& decl, decls ) {
        if (m_abort)
          return;
        items << CompletionTreeItemPointer( new NormalDeclarationCompletionItem(DeclarationPointer(decl.first), completionContext, decl.second ) );
      }

      kDebug(9007) << "setContext: using all declarations visible:" << decls.size();
    }


    ///Find all recursive function-calls that should be shown as call-tips
    Cpp::CodeCompletionContext::Ptr parentContext = completionContext;
    do {
      if (m_abort)
        return;

      parentContext = parentContext->parentContext();
      if( parentContext ) {
        if( parentContext->memberAccessOperation() == Cpp::CodeCompletionContext::FunctionCallAccess ) {
          //When there is too many overloaded functions, do not show them. They can just be too many.
          if( ((parentContext->functions().count() == 0 || parentContext->functions().count() > maxOverloadedOperatorArgumentHints) && parentContext->additionalContextType() == Cpp::CodeCompletionContext::BinaryOperatorFunctionCall)
                || parentContext->functions().count() > maxOverloadedArgumentHints) {
            items << CompletionTreeItemPointer( new NormalDeclarationCompletionItem( KDevelop::DeclarationPointer(), parentContext, 0, 0 ) );
            if(parentContext->functions().count())
              items.back()->asItem<NormalDeclarationCompletionItem>()->alternativeText = i18n("%1 overloads of", parentContext->functions().count()) + " " + parentContext->functionName();
            else
              items.back()->asItem<NormalDeclarationCompletionItem>()->alternativeText = parentContext->functionName();
          }else{
            int num = 0;
            foreach( Cpp::CodeCompletionContext::Function function, parentContext->functions() ) {
              items << CompletionTreeItemPointer( new NormalDeclarationCompletionItem( function.function.declaration(), parentContext, 0, num ) );
              ++num;
            }
          }
        } else {
          kDebug(9007) << "parent-context has non function-call access type";
        }
      }
    } while( parentContext );
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
  enum { Role = CodeCompletionModel::ArgumentHintDepth };
  
  static KeyType extract( const CompletionTreeItemPointer& item ) {
    return item->argumentHintDepth();
  }
};

struct InheritanceDepthExtractor {
  typedef int KeyType;
  
  enum { Role = CodeCompletionModel::InheritanceDepth };
  
  static KeyType extract( const CompletionTreeItemPointer& item ) {
    return item->inheritanceDepth();
  }
};

struct SimplifiedAttributesExtractor {
  typedef int KeyType;
  
  enum { Role = CodeCompletionModel::CompletionRole };

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

void CodeCompletionWorker::computeGroups(QList<CompletionTreeItemPointer> items, KSharedPtr<Cpp::CodeCompletionContext> completionContext)
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

CppCodeCompletionModel* CodeCompletionWorker::model() const
{
  return const_cast<CppCodeCompletionModel*>(static_cast<const CppCodeCompletionModel*>(parent()));
}

void CodeCompletionWorker::run()
{
  exec();
}

void CodeCompletionWorker::abortCurrentCompletion()
{
  QMutexLocker lock(m_mutex);
  m_abort = true;
}

#include "cppcodecompletionworker.moc"

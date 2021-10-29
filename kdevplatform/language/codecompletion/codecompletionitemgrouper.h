/*
    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_CODECOMPLETIONITEMGROUPER_H
#define KDEVPLATFORM_CODECOMPLETIONITEMGROUPER_H

#include "codecompletionmodel.h"
#include "codecompletionitem.h"
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>

namespace KDevelop {
///Always the last item of a grouping chain: Only inserts the items
struct CodeCompletionItemLastGrouper
{
    CodeCompletionItemLastGrouper(QList<QExplicitlySharedDataPointer<CompletionTreeElement>>& tree,
                                  CompletionTreeNode* parent, const QList<CompletionTreeItemPointer>& items)
    {
        tree.reserve(tree.size() + items.size());
        for (auto& item : items) {
            item->setParent(parent);
            tree << QExplicitlySharedDataPointer<CompletionTreeElement>(item.data());
        }
    }
};

///Helper class that helps us grouping the completion-list. A chain of groupers can be built, by using NextGrouper.
template <class KeyExtractor, class NextGrouper = CodeCompletionItemLastGrouper>
struct CodeCompletionItemGrouper
{
    using KeyType = typename KeyExtractor::KeyType;

    CodeCompletionItemGrouper(QList<QExplicitlySharedDataPointer<CompletionTreeElement>>& tree,
                              CompletionTreeNode* parent, const QList<CompletionTreeItemPointer>& items)
    {
        using GroupMap = QMap<KeyType, QList<CompletionTreeItemPointer>>;
        GroupMap groups;

        for (auto& item : items) {
            KeyType key = KeyExtractor::extract(item);
            typename GroupMap::iterator it = groups.find(key);
            if (it == groups.end())
                it = groups.insert(key, QList<CompletionTreeItemPointer>());

            (*it).append(item);
        }

        tree.reserve(tree.size() + groups.size());
        for (typename GroupMap::const_iterator it = groups.constBegin(); it != groups.constEnd(); ++it) {
            QExplicitlySharedDataPointer<CompletionTreeNode> node(new CompletionTreeNode());
            node->setParent(parent);
            node->role = ( KTextEditor::CodeCompletionModel::ExtraItemDataRoles )KeyExtractor::Role;
            node->roleValue = QVariant(it.key());

            tree << QExplicitlySharedDataPointer<CompletionTreeElement>(node.data());

            NextGrouper nextGrouper(node->children, node.data(), *it);
        }
    }
};

///Extracts the argument-hint depth from completion-items, to be used in ItemGrouper for grouping by argument-hint depth.
struct ArgumentHintDepthExtractor
{
    using KeyType = int;
    enum { Role = KTextEditor::CodeCompletionModel::ArgumentHintDepth };

    static KeyType extract(const CompletionTreeItemPointer& item)
    {
        return item->argumentHintDepth();
    }
};

struct InheritanceDepthExtractor
{
    using KeyType = int;

    enum { Role = KTextEditor::CodeCompletionModel::InheritanceDepth };

    static KeyType extract(const CompletionTreeItemPointer& item)
    {
        return item->inheritanceDepth();
    }
};

struct SimplifiedAttributesExtractor
{
    using KeyType = int;

    enum { Role = KTextEditor::CodeCompletionModel::CompletionRole };

    static const int groupingProperties;

    static KeyType extract(const CompletionTreeItemPointer& item)
    {
        DUChainReadLocker lock(DUChain::lock());
        return item->completionProperties() & groupingProperties;
    }
};
}

#endif // KDEVPLATFORM_CODECOMPLETIONITEMGROUPER_H

/*
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_ABSTRACTINCLUDEFILECOMPLETIONITEM_H
#define KDEVPLATFORM_ABSTRACTINCLUDEFILECOMPLETIONITEM_H

#include "codecompletionitem.h"
#include <language/languageexport.h>
#include "../util/includeitem.h"
#include "../duchain/duchain.h"
#include "../duchain/duchainlock.h"
#include "codecompletionmodel.h"

namespace KDevelop {
//A completion item used for completing include-files
template <typename NavigationWidget>
class AbstractIncludeFileCompletionItem
    : public CompletionTreeItem
{
public:
    AbstractIncludeFileCompletionItem(const IncludeItem& include) : includeItem(include)
    {
    }

    QVariant data(const QModelIndex& index, int role, const KDevelop::CodeCompletionModel* model) const override
    {
        DUChainReadLocker lock(DUChain::lock(), 500);
        if (!lock.locked()) {
            qDebug() << "Failed to lock the du-chain in time";
            return QVariant();
        }

        const IncludeItem& item(includeItem);

        switch (role) {
        case CodeCompletionModel::IsExpandable:
            return QVariant(true);
        case CodeCompletionModel::ExpandingWidget: {
            auto* nav = new NavigationWidget(item, model->currentTopContext());

            QVariant v;
            v.setValue<QWidget*>(( QWidget* )nav);
            return v;
        }
        case Qt::DisplayRole:
            switch (index.column()) {
            case CodeCompletionModel::Prefix:
                if (item.isDirectory)
                    return QStringLiteral("directory");
                else
                    return QStringLiteral("file");
            case CodeCompletionModel::Name: {
                return item.isDirectory ? (item.name + QLatin1Char('/')) : item.name;
            }
            }
            break;
        case CodeCompletionModel::ItemSelected:
        {
            return QVariant(NavigationWidget::shortDescription(item));
        }
        }

        return QVariant();
    }

    void execute(KTextEditor::View* view, const KTextEditor::Range& word) override = 0;

    int inheritanceDepth() const override
    {
        return includeItem.pathNumber;
    }
    int argumentHintDepth() const override
    {
        return 0;
    }

    IncludeItem includeItem;
};
}

#endif // KDEVPLATFORM_ABSTRACTINCLUDEFILECOMPLETIONITEM_H

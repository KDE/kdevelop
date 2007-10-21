/***************************************************************************
 *   Copyright 2007 Rober Gruber <rgruber@users.sourceforge.net>                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "snippetfilterproxymodel.h"

#include "snippetstore.h"
#include "snippet.h"

SnippetFilterProxyModel::SnippetFilterProxyModel(QObject *parent)
 : QSortFilterProxyModel(parent)
{
    connect(SnippetStore::instance(),
            SIGNAL( dataChanged(const QModelIndex&, const QModelIndex&) ),
            this,
            SLOT( dataChanged(const QModelIndex&, const QModelIndex&) ));
}


SnippetFilterProxyModel::~SnippetFilterProxyModel()
{
}

void SnippetFilterProxyModel::changeFilter(const QString& filter)
{
    filter_ = filter;
    clear();
}

bool SnippetFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex & sourceParent) const
{
    if (filter_.isEmpty()) {
        // No filtering needed...
        return true;
    }

    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    QStandardItem* item = SnippetStore::instance()->itemFromIndex( index );
    if (!item)
        return false;

    Snippet* snippet = dynamic_cast<Snippet*>( item );
    if (snippet) {
        if ( snippet->text().contains( filter_) ||
             snippet->getKeywordList().contains( filter_ ) )
            return true;
        else
            return false;
    }

    // if it's not a snippet; allow it...
    return true;
}

void SnippetFilterProxyModel::dataChanged(const QModelIndex& /*topLeft*/, const QModelIndex& /*bottomRight*/)
{
    // If we don't do this, the model will contain strange QModelIndex elements after a
    // sync of a repository. Stangely this only happens on Linux. When running under Windows
    // everything's ok, evan without this hack.
    // By letting the proxy reevaluate the items, these elements will be removed.

    ///@todo check if this is OK
    clear();
}

#include "snippetfilterproxymodel.moc"

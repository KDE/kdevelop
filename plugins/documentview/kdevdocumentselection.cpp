/*
    SPDX-FileCopyrightText: 2005 Adam Treat <treat@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kdevdocumentselection.h"

KDevDocumentSelection::KDevDocumentSelection( QAbstractItemModel * model )
        : QItemSelectionModel( model )
{}

KDevDocumentSelection::~KDevDocumentSelection()
{}

void KDevDocumentSelection::select( const QModelIndex & index,
                                    QItemSelectionModel::SelectionFlags command )
{
    if ( !index.parent().isValid() )
        QItemSelectionModel::select( index, NoUpdate );
    else
        QItemSelectionModel::select( index, command );
}

void KDevDocumentSelection::select( const QItemSelection & selection,
                                    QItemSelectionModel::SelectionFlags command )
{
    const QList<QModelIndex> selections = selection.indexes();
    for (auto& selection : selections) {
        if (!selection.parent().isValid()) {
            QItemSelectionModel::select(selection, NoUpdate);
            return;
        }
    }

    QItemSelectionModel::select( selection, command );
}

#include "moc_kdevdocumentselection.cpp"

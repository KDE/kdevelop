/* This file is part of KDevelop
Copyright (C) 2005 Adam Treat <treat@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include "kdevdocumentmodel.h"
#include "kdevdocumentselection.h"

#include <kdebug.h>

KDevDocumentSelection::KDevDocumentSelection( KDevDocumentModel * model )
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
    QList<QModelIndex> selections = selection.indexes();
    QList<QModelIndex>::Iterator it = selections.begin();
    for ( ; it != selections.end(); ++it )
        if ( !( *it ).parent().isValid() )
            return QItemSelectionModel::select( selection, NoUpdate );

    QItemSelectionModel::select( selection, command );
}

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on

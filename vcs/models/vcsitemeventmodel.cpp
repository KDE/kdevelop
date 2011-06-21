/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "vcsitemeventmodel.h"

#include <QModelIndex>
#include <QVariant>
#include <QList>

#include <klocale.h>

#include <tests/modeltest.h>
#include <QDebug>

#include "../vcsrevision.h"
#include "../vcsevent.h"

namespace KDevelop
{

VcsItemEventModel::VcsItemEventModel( QObject* parent )
    : QStandardItemModel( parent )
{
    setHorizontalHeaderLabels(QStringList()
        << i18n("Location")
        << i18n("Actions")
        << i18n("Source Location")
        << i18n("Source Revision")
    );
}

VcsItemEventModel::~VcsItemEventModel()
{}

void VcsItemEventModel::addItemEvents( const QList<KDevelop::VcsItemEvent>& list )
{
    foreach(const KDevelop::VcsItemEvent& ev, list) {
        
        KDevelop::VcsItemEvent::Actions act = ev.actions();
        QStringList actionStrings;
        if( act & KDevelop::VcsItemEvent::Added )
            actionStrings << i18n("Added");
        else if( act & KDevelop::VcsItemEvent::Deleted )
            actionStrings << i18n("Deleted");
        else if( act & KDevelop::VcsItemEvent::Modified )
            actionStrings << i18n("Modified");
        else if( act & KDevelop::VcsItemEvent::Copied )
            actionStrings << i18n("Copied");
        
        QList<QStandardItem*> rowItems = QList<QStandardItem*>()
            << new QStandardItem(ev.repositoryLocation())
            << new QStandardItem(actionStrings.join(i18nc("separes an action list", ", ")))
            << new QStandardItem(ev.repositoryCopySourceLocation())
            << new QStandardItem(ev.repositoryCopySourceRevision().revisionValue().toString());
        
        rowItems.first()->setData(qVariantFromValue(ev));
        appendRow(rowItems);
    }
}

KDevelop::VcsItemEvent VcsItemEventModel::itemEventForIndex( const QModelIndex& idx ) const
{
    return itemFromIndex(idx)->data().value<VcsItemEvent>();
}

}

#include "vcsitemeventmodel.moc"

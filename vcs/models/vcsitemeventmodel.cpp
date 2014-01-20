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

#include <QDebug>
#include <QIcon>
#include <QModelIndex>
#include <QVariant>
#include <QList>
#include <QUrl>

#include <klocale.h>
#include <KMimeType>

#include <tests/modeltest.h>

#include "../vcsrevision.h"
#include "../vcsevent.h"

namespace KDevelop
{

VcsItemEventModel::VcsItemEventModel( QObject* parent )
    : QStandardItemModel( parent )
{
    setColumnCount(2);
}

VcsItemEventModel::~VcsItemEventModel()
{}

void VcsItemEventModel::addItemEvents( const QList<KDevelop::VcsItemEvent>& list )
{
    if(rowCount()==0)
        setColumnCount(2);
    
    bool copySource = false;
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
        else if( act & KDevelop::VcsItemEvent::Replaced )
            actionStrings << i18n("Replaced");
        KMimeType::Ptr mime = KMimeType::findByUrl( QUrl(ev.repositoryLocation()), 0, false, true );
        QList<QStandardItem*> rowItems = QList<QStandardItem*>()
            << new QStandardItem(QIcon::fromTheme(mime->iconName()), ev.repositoryLocation())
            << new QStandardItem(actionStrings.join(i18nc("separes an action list", ", ")));
        QString loc = ev.repositoryCopySourceLocation();
        if(!loc.isEmpty()) { //according to the documentation, those are optional. don't force them on the UI
            rowItems << new QStandardItem(ev.repositoryCopySourceLocation());
            VcsRevision rev = ev.repositoryCopySourceRevision();
            if(rev.revisionType()!=VcsRevision::Invalid) {
                rowItems << new QStandardItem(ev.repositoryCopySourceRevision().revisionValue().toString());
            }
            copySource = true;
        }
        
        rowItems.first()->setData(qVariantFromValue(ev));
        appendRow(rowItems);
    }
    if(copySource)
        setColumnCount(4);
}

QVariant VcsItemEventModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role==Qt::DisplayRole) {
        switch(section)
        {
            case 0: return i18n("Location");
            case 1: return i18n("Actions");
            case 2: return i18n("Source Location");
            case 3: return i18n("Source Revision");
        }
    }
    return QStandardItemModel::headerData(section, orientation, role);
}


KDevelop::VcsItemEvent VcsItemEventModel::itemEventForIndex( const QModelIndex& idx ) const
{
    return itemFromIndex(idx)->data().value<VcsItemEvent>();
}

}

#include "vcsitemeventmodel.moc"

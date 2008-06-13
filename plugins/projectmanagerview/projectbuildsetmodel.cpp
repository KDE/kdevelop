/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                        *
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

#include "projectbuildsetmodel.h"

#include <QVariant>

#include <kurl.h>
#include <klocale.h>
#include <kdebug.h>
#include <kconfiggroup.h>

#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>

#include "projectmodel.h"
#include "iproject.h"

QString getRelativeFolder( KDevelop::ProjectBaseItem* item )
{
    if( !item )
        return "";

    if( item->type() == KDevelop::ProjectBaseItem::Folder
          || item->type() == KDevelop::ProjectBaseItem::BuildFolder )
    {

        return item->project()->relativeUrl( item->folder()->url() ).path();
    }else
    {
        return getRelativeFolder( dynamic_cast<KDevelop::ProjectBaseItem*>( item->parent() ) );
    }
}

KDevelop::ProjectBaseItem* findItem( const QString& item, const QString& path, KDevelop::ProjectBaseItem* top )
{
    if( top && top->text() == item && getRelativeFolder( top ) == path )
    {
        return top;
    }else if( top->hasChildren() )
    {
        for( int i = 0; i < top->rowCount(); i++ )
        {
            QStandardItem* sitem = top->child( i );
            KDevelop::ProjectBaseItem* prjitem = dynamic_cast<KDevelop::ProjectBaseItem*>(sitem);
            if( prjitem )
            {
                if( prjitem->file()
                    && prjitem->text() == item
                    && path == getRelativeFolder( prjitem->file() ) )
                {
                    return prjitem;
                }else if( prjitem->folder()
                          && prjitem->text() == item
                          && path == getRelativeFolder( prjitem->folder() ) )
                {
                    return prjitem;
                }else if( prjitem->target()
                          && prjitem->text() == item
                          && path == getRelativeFolder( prjitem->target() ) )
                {
                    return prjitem;
                }else
                {
                    KDevelop::ProjectBaseItem* tmp = findItem( item, path, prjitem );
                    if( tmp )
                        return tmp;
                }
            }
        }
    }
    return 0;
}


BuildItem::BuildItem()
{
}

BuildItem::BuildItem( const QString& itemName, const QString& projectName, const QString& itemPath )
        : m_itemName( itemName ), m_projectName( projectName ), m_itemPath( itemPath )
{
}

BuildItem::BuildItem( KDevelop::ProjectBaseItem* item )
{
    initializeFromItem( item );
}

BuildItem::BuildItem( const BuildItem& rhs )
{
    m_itemName = rhs.itemName();
    m_projectName = rhs.projectName();
    m_itemPath = rhs.itemPath();
}

void BuildItem::initializeFromItem( KDevelop::ProjectBaseItem* item )
{
    if( item )
    {
        m_itemName = item->text();
        m_itemPath = getRelativeFolder( item );
        m_projectName = item->project()->name();
    }
}

KDevelop::ProjectBaseItem* BuildItem::findItem() const
{
    KDevelop::ProjectBaseItem* top = 0;
    KDevelop::IProject* project = KDevelop::ICore::self()->projectController()->findProjectByName( projectName() );
    if( project )
    {
        top = ::findItem( itemName(), itemPath(), project->projectItem() );
    }
    return top;
}

bool operator==( const BuildItem& rhs, const BuildItem& lhs  )
{
    return( rhs.itemName() == lhs.itemName() && rhs.projectName() == lhs.projectName() && rhs.itemPath() == lhs.itemPath() );
}

BuildItem& BuildItem::operator=( const BuildItem& rhs )
{
    if( this == &rhs )
        return *this;
    m_itemName = rhs.itemName();
    m_projectName = rhs.projectName();
    m_itemPath = rhs.itemPath();
    return *this;
}

ProjectBuildSetModel::ProjectBuildSetModel( QObject* parent )
    : QAbstractTableModel( parent )
{
}

QVariant ProjectBuildSetModel::data( const QModelIndex& idx, int role ) const
{
    if( !idx.isValid() || idx.row() < 0 || idx.column() < 0
         || idx.row() >= rowCount() || idx.column() >= columnCount()
         || role != Qt::DisplayRole )
    {
        return QVariant();
    }
    switch( idx.column() )
    {
        case 0:
            return m_items.at( idx.row() ).itemName();
            break;
        case 1:
            return m_items.at( idx.row() ).projectName();
            break;
        case 2:
            return m_items.at( idx.row() ).itemPath();
            break;
    }
    return QVariant();
}

QVariant ProjectBuildSetModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if( section < 0 || section >= columnCount()
        || orientation != Qt::Horizontal || role != Qt::DisplayRole )
        return QVariant();

    switch( section )
    {
        case 0:
            return i18n("Name");
            break;
        case 1:
            return i18n("Project");
            break;
        case 2:
            return i18n("Folder");
            break;
    }
    return QVariant();
}

int ProjectBuildSetModel::rowCount( const QModelIndex& parent ) const
{
    if( parent.isValid() )
        return 0;
    return m_items.count();
}

int ProjectBuildSetModel::columnCount( const QModelIndex& parent ) const
{
    if( parent.isValid() )
        return 0;
    return 3;
}

void ProjectBuildSetModel::addProjectItem( KDevelop::ProjectBaseItem* item )
{
    if( m_items.contains( item ) )
        return;
    beginInsertRows( QModelIndex(), rowCount(), rowCount() );
    m_items.append(BuildItem(item));
    endInsertRows();
}

bool ProjectBuildSetModel::removeRows( int row, int count, const QModelIndex& parent )
{
    if( parent.isValid() || row > rowCount() || row < 0 || (row+count) > rowCount() || count <= 0 )
        return false;

    beginRemoveRows( QModelIndex(), row, row+count-1 );
    for( int i = row; i < row+count; i++ )
    {
        m_items.removeAt( row );
    }
    endRemoveRows();
    return true;
}

KDevelop::ProjectBaseItem* ProjectBuildSetModel::itemForIndex( const QModelIndex& idx )
{
    if( !idx.isValid() || idx.row() < 0 || idx.column() < 0
         || idx.column() >= columnCount() || idx.row() >= rowCount() )
        return 0;
    return m_items.at( idx.row() ).findItem();
}

QList<BuildItem> ProjectBuildSetModel::items()
{
    return m_items ;
}

void ProjectBuildSetModel::saveSettings( KConfigGroup & base ) const
{
    kDebug() << "storing buildset" << rowCount();
    for( int i = 0; i < rowCount(); i++ )
    {
        KConfigGroup grp = base.group(QString("Builditem%1").arg(i));
        grp.writeEntry("Projectname", m_items.at(i).projectName());
        grp.writeEntry("Itemname", m_items.at(i).itemName());
        grp.writeEntry("Itempath", m_items.at(i).itemPath());
    }
    base.writeEntry("Number of Builditems", rowCount());
}

void ProjectBuildSetModel::readSettings( KConfigGroup & base )
{
    // readSettings() should only be called once during loading the plugin
    Q_ASSERT( m_items.isEmpty() );
    int count = base.readEntry("Number of Builditems", 0);
    kDebug() << "reading" << count << "number of entries";
    for( int i = 0; i < count; i++ )
    {
        KConfigGroup grp = base.group(QString("Builditem%1").arg(i));
        QString name = grp.readEntry("Projectname");
        QString item = grp.readEntry("Itemname");
        QString path = grp.readEntry("Itempath");
        beginInsertRows( QModelIndex(), rowCount(), rowCount() );
        m_items.append( BuildItem( item, name, path ) );
        endInsertRows();
    }
    kDebug() << "rowcount after reading settings" << rowCount();
}

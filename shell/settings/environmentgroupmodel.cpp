/* This file is part of KDevelop
Copyright 2007 Andreas Pakulat <apaku@gmx.de>

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
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/

#include "environmentgroupmodel.h"
#include <QtCore/QVariant>
#include <QtCore/QStringList>
#include <QtCore/QModelIndex>

#include <kdebug.h>
#include <KLocalizedString>

namespace KDevelop
{

EnvironmentGroupModel::EnvironmentGroupModel()
    : QAbstractTableModel()
{
}

int EnvironmentGroupModel::rowCount( const QModelIndex& parent ) const
{
    if( parent.isValid() )
        return 0;
    if( !m_currentGroup.isEmpty() )
        return m_varsByIndex.count();
    return 0;
}

int EnvironmentGroupModel::columnCount( const QModelIndex& parent ) const
{
    if( parent.isValid() )
        return 0;
    return 2;
}

Qt::ItemFlags EnvironmentGroupModel::flags( const QModelIndex& idx ) const
{
    if( !idx.isValid() || m_currentGroup.isEmpty() )
    {
        return Qt::NoItemFlags ;
    }
    return ( Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled );
}

QVariant EnvironmentGroupModel::data( const QModelIndex& idx, int role ) const
{
    if( !idx.isValid() || ( role != Qt::DisplayRole && role != Qt::EditRole )
        || m_currentGroup.isEmpty()
        || idx.row() < 0 || idx.row() >= rowCount(QModelIndex())
        || idx.column() < 0 || idx.column() >= columnCount(QModelIndex()) )
    {
        return QVariant();
    }

    if( idx.column() == 0 )
    {
        return m_varsByIndex[idx.row()];
    } else
    {
        QString var = m_varsByIndex[idx.row()];
        return variables( m_currentGroup ).value( var );
    }
}

QVariant EnvironmentGroupModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if( section < 0 || section >= columnCount(QModelIndex()) || m_currentGroup.isEmpty()
        || orientation != Qt::Horizontal || role != Qt::DisplayRole )
    {
        return QVariant();
    }
    if( section == 0 )
    {
        return i18n( "Variable" );
    } else
    {
        return i18n( "Value" );
    }
}

bool EnvironmentGroupModel::setData( const QModelIndex& idx, const QVariant& data, int role )
{
    if( !idx.isValid() || role != Qt::EditRole || m_currentGroup.isEmpty()
        || idx.row() < 0 || idx.row() >= rowCount(QModelIndex())
        || idx.column() < 0 || idx.column() >= columnCount(QModelIndex()) )
    {
        return false;
    }

    if( idx.column() == 0 )
    {
        QString var = m_varsByIndex[idx.row()];
        QString value = variables( m_currentGroup ).value( var );
        variables( m_currentGroup ).remove( var );
        variables( m_currentGroup ).insert( data.toString(), value );
        m_varsByIndex[idx.row()] = data.toString();
    }
    else
    {
        variables( m_currentGroup ).insert( m_varsByIndex[idx.row()], data.toString() );
    }
    
    emit dataChanged(idx, idx);
    return true;
}

QModelIndex EnvironmentGroupModel::addVariable( const QString& var, const QString& value )
{
    const int pos = m_varsByIndex.indexOf(var);
    if (pos != -1) {
        return index(pos, 0, QModelIndex()); // No duplicates
    }

    const int insertPos = rowCount( QModelIndex() );
    beginInsertRows( QModelIndex(), insertPos, insertPos );
    m_varsByIndex << var;
    variables( m_currentGroup ).insert( var, value );
    endInsertRows();
    return index(insertPos, 0, QModelIndex());
}

void EnvironmentGroupModel::removeGroup( const QString& grp )
{
    if( groups().contains( grp ) )
    {
        EnvironmentGroupList::removeGroup( grp );
        setCurrentGroup(defaultGroup());
    }
}

void EnvironmentGroupModel::removeVariables( QModelIndexList indexes )
{
    if (m_currentGroup.isEmpty())
        return;
    
    //Need to have the indexes in order for reverse removal
    qSort<QModelIndexList>(indexes);
    
    for (int i = indexes.size() - 1; i >= 0; --i)
    {
        if (!indexes[i].isValid())
            continue;
        
        beginRemoveRows( QModelIndex(), indexes[i].row(), indexes[i].row() );
        QString var = m_varsByIndex[indexes[i].row()];
        m_varsByIndex.removeAt(indexes[i].row());
        variables( m_currentGroup ).remove( var );
        endRemoveRows();
    }
}

void EnvironmentGroupModel::setCurrentGroup( const QString& group )
{
    if( group.isEmpty() )
        return;
    m_currentGroup = group;
    m_varsByIndex.clear();

    foreach( const QString &var, variables( m_currentGroup ).keys() )
    {
        m_varsByIndex << var;
    }
    reset();
}

void EnvironmentGroupModel::changeDefaultGroup( const QString& grp )
{
    if( !grp.isEmpty() )
        setDefaultGroup( grp );
}

void EnvironmentGroupModel::loadFromConfig( KConfig* cfg )
{
    loadSettings( cfg );
    setCurrentGroup(defaultGroup());
}


void EnvironmentGroupModel::saveToConfig( KConfig* cfg )
{
    saveSettings( cfg );
}

}

#include "environmentgroupmodel.moc"

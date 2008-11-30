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
        return m_variableMap.count();
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
        return m_variableMap[idx.row()];
    } else
    {
        QString var = m_variableMap[idx.row()];
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
        return "Variable";
    } else
    {
        return "Value";
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
        QString var = m_variableMap[idx.row()];
        QString value = variables( m_currentGroup ).value( var );
        variables( m_currentGroup ).remove( var );
        variables( m_currentGroup ).insert( data.toString(), value );
        m_variableMap[idx.row()] = data.toString();
    } else
    {
        variables( m_currentGroup ).insert( m_variableMap[idx.row()], data.toString() );
    }
    return true;
}

void EnvironmentGroupModel::addVariable( const QString& var, const QString& value )
{
    beginInsertRows( QModelIndex(), rowCount( QModelIndex() ), rowCount( QModelIndex() ) );
    m_variableMap.insert( rowCount( QModelIndex() ), var );
    variables( m_currentGroup ).insert( var, value );
    endInsertRows();
}

void EnvironmentGroupModel::removeGroup( const QString& grp )
{
    if( groups().contains( grp ) )
    {
        EnvironmentGroupList::removeGroup( grp );
        setCurrentGroup(defaultGroup());
    }
}

void EnvironmentGroupModel::removeVariable( const QModelIndex& idx )
{
    kDebug() << "checking idx" << idx << rowCount(QModelIndex()) << m_currentGroup << columnCount(QModelIndex());
    if( idx.isValid() && !m_currentGroup.isEmpty()
        && idx.row() >=0 && idx.row() < rowCount(QModelIndex())
        && idx.column() >= 0 && idx.column() < columnCount(QModelIndex()) )
    {
        beginRemoveRows( QModelIndex(), idx.row(), idx.row() );
        QString var = m_variableMap[idx.row()];
        m_variableMap.remove(idx.row());
        variables( m_currentGroup ).remove( var );
        endRemoveRows();
    }
}

void EnvironmentGroupModel::setCurrentGroup( const QString& group )
{
    if( group.isEmpty() )
        return;
    m_currentGroup = group;
    m_variableMap.clear();
    int i = 0;
    foreach( const QString &var, variables( m_currentGroup ).keys() )
    {
        m_variableMap.insert(i++, var);
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

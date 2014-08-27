/* This file is part of KDevelop
Copyright 2013 Ivan Shapovalov <intelfx100@gmail.com>

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

#include "environmentselectionmodel.h"

namespace
{

QStringList entriesFromEnv( const KDevelop::EnvironmentGroupList& env )
{
    // We add an empty (i. e. default profile) entry to the front of the model's list.
    return QStringList( QString() ) + env.groups();
}

}

namespace KDevelop
{

EnvironmentSelectionModel::EnvironmentSelectionModel( QObject* parent ) :
    QStringListModel( parent ),
    m_env( KSharedConfig::openConfig() )
{
    setStringList( entriesFromEnv( m_env ) );
    m_groupsLookupTable = stringList().toSet();
}

QVariant EnvironmentSelectionModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if( section != 0 ||
        orientation != Qt::Horizontal ||
        role != Qt::DisplayRole ) {
        return QVariant();
    }

    return i18nc("@title:column", "Profile");
}

QVariant EnvironmentSelectionModel::data( const QModelIndex& index, int role ) const
{
    QVariant nativeData = QStringListModel::data( index, Qt::DisplayRole );
    QString profileName = nativeData.toString();

    switch( role ) {
    case Qt::DisplayRole:
        if( profileName.isEmpty() ) {
            return i18nc( "@item:inlistbox", "Use default profile (currently: %1)", m_env.defaultGroup() );
        }

        if( !m_groupsLookupTable.contains( profileName ) ) {
            return i18nc( "@item:inlistbox", "%1 (does not exist)", profileName );
        }

        return nativeData;

    case EffectiveNameRole:
        if( profileName.isEmpty() ) {
            return m_env.defaultGroup();
        }

        return nativeData;

    default:
        return QStringListModel::data( index, role );
    }
}

bool EnvironmentSelectionModel::setData( const QModelIndex& /*index*/, const QVariant& /*value*/, int /*role*/ )
{
    return false;
}

EnvironmentGroupList EnvironmentSelectionModel::environment() const
{
    return m_env;
}

void EnvironmentSelectionModel::reload()
{
    m_env = EnvironmentGroupList( KSharedConfig::openConfig() );

    setStringList( entriesFromEnv( m_env ) );
    m_groupsLookupTable = stringList().toSet();
}

QString EnvironmentSelectionModel::reloadSelectedItem( const QString& currentProfile )
{
    if( m_groupsLookupTable.contains( currentProfile ) ) {
        return currentProfile;
    } else {
        return QString();
    }
}

} // namespace KDevelop


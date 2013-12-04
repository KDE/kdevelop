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

#ifndef KDEVPLATFORM_ENVIRONMENTGROUPMODEL_H
#define KDEVPLATFORM_ENVIRONMENTGROUPMODEL_H

#include <QtCore/QAbstractTableModel>
#include <QtCore/QStringList>
#include "util/environmentgrouplist.h"

class QVariant;
class QModelIndex;

namespace KDevelop
{


class EnvironmentGroupModel : public QAbstractTableModel, public EnvironmentGroupList
{
    Q_OBJECT
public:
    EnvironmentGroupModel();
    int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    int columnCount( const QModelIndex &parent = QModelIndex()  ) const;
    Qt::ItemFlags flags( const QModelIndex& idx ) const;
    QVariant data( const QModelIndex& idx, int role = Qt::DisplayRole) const;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    bool setData( const QModelIndex& idx, const QVariant&, int role = Qt::EditRole);
    void setCurrentGroup( const QString& group );
    void loadFromConfig( KConfig* );
    void saveToConfig( KConfig* );
    QModelIndex addVariable( const QString& var, const QString& value );
    void removeVariables( QModelIndexList variables );
    void removeGroup( const QString& grp );
    void changeDefaultGroup( const QString& grp );

private:
    QStringList m_varsByIndex;
    QString m_currentGroup;
};

}

#endif

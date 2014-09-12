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

#ifndef ENVIRONMENTSELECTIONMODEL_H
#define ENVIRONMENTSELECTIONMODEL_H

#include "environmentgrouplist.h"

#include <QStringListModel>

#include <KLocalizedString>

namespace KDevelop
{

class EnvironmentSelectionModel : public QStringListModel
{
    Q_OBJECT

public:
    enum SpecialRoles
    {
        EffectiveNameRole = Qt::UserRole + 1
    };

    explicit EnvironmentSelectionModel( QObject* parent = 0 );

    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

    virtual QVariant data( const QModelIndex& index, int role ) const;
    virtual bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole );

    /**
     * @returns The @ref EnvironmentGroupList which has bee used to populate this
     *          model.
     */
    EnvironmentGroupList environment() const;

    /**
     * Reloads the model from the global config.
     */
    void reload();

    /**
     * Shall be used by views to update selection (current item) after the model has been reloaded.
     *
     * @param currentProfile Previous selected item.
     * @returns              The item which shall become selected.
     */
    QString reloadSelectedItem( const QString& currentProfile );

private:
    EnvironmentGroupList m_env;
    QSet<QString> m_groupsLookupTable;
};

} // namespace KDevelop

#endif // ENVIRONMENTSELECTIONMODEL_H

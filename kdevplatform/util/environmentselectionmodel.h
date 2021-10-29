/*
    SPDX-FileCopyrightText: 2013 Ivan Shapovalov <intelfx100@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef ENVIRONMENTSELECTIONMODEL_H
#define ENVIRONMENTSELECTIONMODEL_H

#include "environmentprofilelist.h"

#include <QStringListModel>
#include <QSet>

namespace KDevelop {

class EnvironmentSelectionModel : public QStringListModel
{
    Q_OBJECT

public:
    enum SpecialRoles
    {
        EffectiveNameRole = Qt::UserRole + 1
    };

    explicit EnvironmentSelectionModel(QObject* parent = nullptr);

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    /**
     * @returns The @ref EnvironmentProfileList which has been used to populate this
     *          model.
     */
    EnvironmentProfileList environmentProfiles() const;

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
    QString reloadSelectedItem(const QString& currentProfile);

private:
    EnvironmentProfileList m_env;
    QSet<QString> m_profilesLookupTable;
};

} // namespace KDevelop

#endif // ENVIRONMENTSELECTIONMODEL_H

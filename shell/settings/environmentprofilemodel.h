/* This file is part of KDevelop
Copyright 2007 Andreas Pakulat <apaku@gmx.de>
Copyright 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

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

#ifndef KDEVPLATFORM_ENVIRONMENTPROFILEMODEL_H
#define KDEVPLATFORM_ENVIRONMENTPROFILEMODEL_H

#include <QAbstractTableModel>
#include <QStringList>

namespace KDevelop
{
class EnvironmentProfileListModel;

class EnvironmentProfileModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Role {
        VariableRole = Qt::UserRole + 1,
        ValueRole
    };

    enum Column {
        VariableColumn = 0,
        ValueColumn = 1
    };

    EnvironmentProfileModel(EnvironmentProfileListModel* profileListModel, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = {}) const override;
    int columnCount(const QModelIndex& parent = {}) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant&, int role = Qt::EditRole) override;

    void setCurrentProfile(const QString& profileName);

    void addVariable(const QString& variableName, const QString& value);
    /**
     * Load a set of environment variables from a plaintext string.
     *
     * @p plainText In the form "FOO=1\nBAR=2"
     */
    void setVariablesFromString(const QString& plainText);
    void removeVariable(const QString& variableName);
    void removeVariables(const QStringList& variableNames);

private:
    void onProfileAboutToBeRemoved(const QString& profileName);

private:
    QStringList m_varsByIndex;
    QString m_currentProfileName;
    EnvironmentProfileListModel* m_profileListModel;
};

}

#endif

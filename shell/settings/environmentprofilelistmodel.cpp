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

#include "environmentprofilelistmodel.h"

#include <KLocalizedString>

using namespace KDevelop;

EnvironmentProfileListModel::EnvironmentProfileListModel(QObject* parent)
    : QAbstractItemModel(parent)
{
}

int EnvironmentProfileListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return groups().count();
}

int EnvironmentProfileListModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return 1;
}

QVariant EnvironmentProfileListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() ||
        index.row() < 0 || index.row() >= rowCount() ||
        index.column() != 0) {
        return {};
    }

    if (role == Qt::DisplayRole) {
        auto profileName = groups().at(index.row());
        if (profileName == defaultGroup()) {
            profileName = i18n("%1 (default profile)", profileName);
        }
        return profileName;
    }

    return {};
}

QModelIndex EnvironmentProfileListModel::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return {};
    }
    return createIndex(row, column);
}

QModelIndex EnvironmentProfileListModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return {};
}

int EnvironmentProfileListModel::defaultProfileIndex() const
{
    return groups().indexOf(defaultGroup());
}

QString EnvironmentProfileListModel::profileName(int profileIndex) const
{
    return groups().value(profileIndex);
}

int EnvironmentProfileListModel::profileIndex(const QString& profileName) const
{
    return groups().indexOf(profileName);
}

bool EnvironmentProfileListModel::hasProfile(const QString& profileName) const
{
    return groups().contains(profileName);
}

QMap<QString, QString>& EnvironmentProfileListModel::variables(const QString& profileName)
{
    return EnvironmentGroupList::variables(profileName);
}

int EnvironmentProfileListModel::addProfile(const QString& newProfileName)
{
    const auto profileNames = groups();
    if (newProfileName.isEmpty() ||
        profileNames.contains(newProfileName)) {
        return -1;
    }

    // estimate insert position by comparison as used by QMap for the keys
    int insertPos = 0;
    while (insertPos < profileNames.size() &&
           profileNames.at(insertPos) < newProfileName) {
        ++insertPos;
    }
    beginInsertRows(QModelIndex(), insertPos, insertPos);

    // trigger creation of new profile
    variables(newProfileName);

    endInsertRows();
    return insertPos;
}

void EnvironmentProfileListModel::removeProfile(int profileIndex)
{
    const auto profileNames = groups();
    if (profileIndex < 0 || profileIndex >= profileNames.size()) {
        return;
    }
    const auto profileName = profileNames.at(profileIndex);
    if (defaultGroup() == profileName) {
        return;
    }

    emit profileAboutToBeRemoved(profileName);

    beginRemoveRows(QModelIndex(), profileIndex, profileIndex);

    EnvironmentGroupList::removeGroup(profileName);

    endRemoveRows();
}

int EnvironmentProfileListModel::cloneProfile(const QString& newProfileName, const QString& sourceProfileName)
{
    const auto profileNames = groups();
    if (newProfileName.isEmpty() ||
        profileNames.contains(newProfileName) ||
        !profileNames.contains(sourceProfileName)) {
        return -1;
    }

    // estimate insert position by comparison as used by QMap for the keys
    int insertPos = 0;
    while (insertPos < profileNames.size() &&
           profileNames.at(insertPos) < newProfileName) {
        ++insertPos;
    }

    beginInsertRows(QModelIndex(), insertPos, insertPos);

    variables(newProfileName) = variables(sourceProfileName);

    endInsertRows();

    return insertPos;
}

void EnvironmentProfileListModel::setDefaultProfile(int profileIndex)
{
    const auto profileNames = groups();
    const auto oldDefaultProfileName = defaultGroup();
    const int oldDefaultProfileIndex = profileNames.indexOf(oldDefaultProfileName);

    if (profileIndex < 0 || profileIndex >= profileNames.size() ||
        oldDefaultProfileIndex == profileIndex) {
        return;
    }

    const auto oldIndex = index(oldDefaultProfileIndex, 0);

    const auto profileName = profileNames.at(profileIndex);
    setDefaultGroup(profileName);

    const int newDefaultProfileIndex = profileNames.indexOf(profileName);
    const auto newIndex = index(newDefaultProfileIndex, 0);

    emit dataChanged(oldIndex, oldIndex);
    emit dataChanged(newIndex, newIndex);
    emit defaultProfileChanged(newDefaultProfileIndex);
}

void EnvironmentProfileListModel::loadFromConfig(KConfig* config)
{
    beginResetModel();

    loadSettings(config);

    endResetModel();
}


void EnvironmentProfileListModel::saveToConfig(KConfig* config)
{
    saveSettings(config);
}

/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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

    return profileNames().count();
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
        auto profileName = profileNames().at(index.row());
        if (profileName == defaultProfileName()) {
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
    return profileNames().indexOf(defaultProfileName());
}

QString EnvironmentProfileListModel::profileName(int profileIndex) const
{
    return profileNames().value(profileIndex);
}

int EnvironmentProfileListModel::profileIndex(const QString& profileName) const
{
    return profileNames().indexOf(profileName);
}

bool EnvironmentProfileListModel::hasProfile(const QString& profileName) const
{
    return profileNames().contains(profileName);
}

QMap<QString, QString>& EnvironmentProfileListModel::variables(const QString& profileName)
{
    return EnvironmentProfileList::variables(profileName);
}

int EnvironmentProfileListModel::addProfile(const QString& newProfileName)
{
    const auto profileNames = this->profileNames();
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
    const auto profileNames = this->profileNames();
    if (profileIndex < 0 || profileIndex >= profileNames.size()) {
        return;
    }
    const auto profileName = profileNames.at(profileIndex);
    if (defaultProfileName() == profileName) {
        return;
    }

    emit profileAboutToBeRemoved(profileName);

    beginRemoveRows(QModelIndex(), profileIndex, profileIndex);

    EnvironmentProfileList::removeProfile(profileName);

    endRemoveRows();
}

int EnvironmentProfileListModel::cloneProfile(const QString& newProfileName, const QString& sourceProfileName)
{
    const auto profileNames = this->profileNames();
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
    const auto profileNames = this->profileNames();
    const auto oldDefaultProfileName = defaultProfileName();
    const int oldDefaultProfileIndex = profileNames.indexOf(oldDefaultProfileName);

    if (profileIndex < 0 || profileIndex >= profileNames.size() ||
        oldDefaultProfileIndex == profileIndex) {
        return;
    }

    const auto oldIndex = index(oldDefaultProfileIndex, 0);

    const auto profileName = profileNames.at(profileIndex);
    EnvironmentProfileList::setDefaultProfile(profileName);

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

#include "moc_environmentprofilelistmodel.cpp"

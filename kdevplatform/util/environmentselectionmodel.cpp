/*
    SPDX-FileCopyrightText: 2013 Ivan Shapovalov <intelfx100@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "environmentselectionmodel.h"

#include <KLocalizedString>

namespace {

QStringList entriesFromEnv(const KDevelop::EnvironmentProfileList& env)
{
    // We add an empty (i. e. default profile) entry to the front of the model's list.
    return QStringList(QString()) + env.profileNames();
}

}

namespace KDevelop {

EnvironmentSelectionModel::EnvironmentSelectionModel(QObject* parent) :
    QStringListModel(parent)
    , m_env(KSharedConfig::openConfig())
{
    const QStringList entries = entriesFromEnv(m_env);
    setStringList(entries);
    m_profilesLookupTable = QSet<QString>(entries.begin(), entries.end());
}

QVariant EnvironmentSelectionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section != 0 ||
        orientation != Qt::Horizontal ||
        role != Qt::DisplayRole) {
        return QVariant();
    }

    return i18nc("@title:column", "Profile");
}

QVariant EnvironmentSelectionModel::data(const QModelIndex& index, int role) const
{
    QVariant nativeData = QStringListModel::data(index, Qt::DisplayRole);
    QString profileName = nativeData.toString();

    switch (role) {
    case Qt::DisplayRole:
        if (profileName.isEmpty()) {
            return i18nc("@item:inlistbox", "Use default profile (currently: %1)", m_env.defaultProfileName());
        }

        if (!m_profilesLookupTable.contains(profileName)) {
            return i18nc("@item:inlistbox", "%1 (does not exist)", profileName);
        }

        return nativeData;

    case EffectiveNameRole:
        if (profileName.isEmpty()) {
            return m_env.defaultProfileName();
        }

        return nativeData;

    default:
        return QStringListModel::data(index, role);
    }
}

bool EnvironmentSelectionModel::setData(const QModelIndex& /*index*/, const QVariant& /*value*/, int /*role*/)
{
    return false;
}

EnvironmentProfileList EnvironmentSelectionModel::environmentProfiles() const
{
    return m_env;
}

void EnvironmentSelectionModel::reload()
{
    m_env = EnvironmentProfileList(KSharedConfig::openConfig());

    const QStringList entries = entriesFromEnv(m_env);
    setStringList(entries);
    m_profilesLookupTable = QSet<QString>(entries.begin(), entries.end());
}

QString EnvironmentSelectionModel::reloadSelectedItem(const QString& currentProfile)
{
    if (m_profilesLookupTable.contains(currentProfile)) {
        return currentProfile;
    } else {
        return QString();
    }
}

} // namespace KDevelop

#include "moc_environmentselectionmodel.cpp"

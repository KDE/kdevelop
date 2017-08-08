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

#include "environmentprofilemodel.h"

#include "environmentprofilelistmodel.h"

#include <KLocalizedString>

#include <QVariant>
#include <QModelIndex>

using namespace KDevelop;

EnvironmentProfileModel::EnvironmentProfileModel(EnvironmentProfileListModel* profileListModel,
                                                 QObject* parent)
    : QAbstractTableModel(parent)
    , m_profileListModel(profileListModel)
{
    connect(m_profileListModel, &EnvironmentProfileListModel::profileAboutToBeRemoved,
            this, &EnvironmentProfileModel::onProfileAboutToBeRemoved);
}

int EnvironmentProfileModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_varsByIndex.count();
}

int EnvironmentProfileModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return 2;
}

Qt::ItemFlags EnvironmentProfileModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    return (Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
}

QVariant EnvironmentProfileModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() ||
        index.row() < 0 || index.row() >= rowCount() ||
        index.column() < 0 || index.column() >= columnCount(QModelIndex()) ||
        m_currentProfileName.isEmpty()) {
        return {};
    }

    const auto variable = m_varsByIndex[index.row()];
    if (role == VariableRole) {
        return variable;
    }
    if (role == ValueRole) {
        const auto& variables = m_profileListModel->variables(m_currentProfileName);
        return variables.value(variable);
    }
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        if (index.column() == VariableColumn) {
            return variable;
        }
        const auto& variables = m_profileListModel->variables(m_currentProfileName);
        return variables.value(variable);
    }
    return {};
}

QVariant EnvironmentProfileModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section < 0 || section >= columnCount(QModelIndex()) ||
        orientation != Qt::Horizontal ||
        role != Qt::DisplayRole) {
        return {};
    }

    if (section == VariableColumn) {
        return i18n("Variable");
    }
    return i18n("Value");
}

bool EnvironmentProfileModel::setData(const QModelIndex& index, const QVariant& data, int role)
{
    if (!index.isValid() ||
        index.row() < 0 || index.row() >= rowCount() ||
        index.column() < 0 || index.column() >= columnCount(QModelIndex()) ||
        m_currentProfileName.isEmpty()) {
        return false;
    }

    if (role == Qt::EditRole) {
        auto& variables = m_profileListModel->variables(m_currentProfileName);
        if (index.column() == VariableColumn) {
            const QString oldVariable = m_varsByIndex[index.row()];
            const QString value = variables.take(oldVariable);
            const QString newVariable = data.toString();
            variables.insert(newVariable, value);
            m_varsByIndex[index.row()] = newVariable;
        } else {
            variables.insert(m_varsByIndex[index.row()], data.toString());
        }
        emit dataChanged(index, index);
    }
    return true;
}

void EnvironmentProfileModel::addVariable(const QString& variableName, const QString& value)
{
    if (m_currentProfileName.isEmpty()) {
        return;
    }

    const int pos = m_varsByIndex.indexOf(variableName);
    if (pos != -1) {
        return; // No duplicates, first value
    }

    auto& variables = m_profileListModel->variables(m_currentProfileName);

    const int insertPos = rowCount();
    beginInsertRows(QModelIndex(), insertPos, insertPos);
    m_varsByIndex << variableName;
    variables.insert(variableName, value);
    endInsertRows();
}

void EnvironmentProfileModel::removeVariables(const QStringList& variableNames)
{
    for (const auto& variableName : variableNames) {
        removeVariable(variableName);
    }
}

void EnvironmentProfileModel::removeVariable(const QString& variableName)
{
    if (m_currentProfileName.isEmpty()) {
        return;
    }

    const int pos = m_varsByIndex.indexOf(variableName);
    if (pos == -1) {
        return;
    }

    auto& variables = m_profileListModel->variables(m_currentProfileName);

    beginRemoveRows(QModelIndex(), pos, pos);
    m_varsByIndex.removeAt(pos);
    variables.remove(variableName);
    endRemoveRows();
}

void EnvironmentProfileModel::setCurrentProfile(const QString& profileName)
{
    if (profileName == m_currentProfileName) {
        return;
    }

    beginResetModel();
    m_currentProfileName = profileName;
    m_varsByIndex.clear();

    if (!m_currentProfileName.isEmpty()) {
        const auto& variables = m_profileListModel->variables(m_currentProfileName);

        const auto endIt = variables.constEnd();
        for (auto it = variables.constBegin(); it != endIt; ++it) {
            m_varsByIndex << it.key();
        }
    }
    endResetModel();
}

void EnvironmentProfileModel::setVariablesFromString(const QString& plainText)
{
    if (m_currentProfileName.isEmpty()) {
        return;
    }

    beginResetModel();

    auto& variables = m_profileListModel->variables(m_currentProfileName);
    variables.clear();
    m_varsByIndex.clear();

    const auto lines = plainText.split(QLatin1Char('\n'), QString::SkipEmptyParts);
    for (const auto& line : lines) {
        const int pos = line.indexOf(QLatin1Char('='));
        // has a = and at least 1 char
        if (pos < 0) {
            continue;
        }
        const QString variableName = line.left(pos).trimmed();
        if (variableName.isEmpty()) {
            continue;
        }
        const QString value = line.mid(pos+1).trimmed();
        m_varsByIndex << variableName;
        variables.insert(variableName, value);
    }

    endResetModel();
}

void EnvironmentProfileModel::onProfileAboutToBeRemoved(const QString& profileName)
{
    if (m_currentProfileName == profileName) {
        setCurrentProfile(QString());
    }
}

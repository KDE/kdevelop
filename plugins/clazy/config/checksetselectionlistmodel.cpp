/*
    SPDX-FileCopyrightText: 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "checksetselectionlistmodel.h"

// plugin
#include "checksetselectionmanager.h"
// KF
#include <KLocalizedString>
// Qt
#include <QUuid>
// Std
#include <algorithm>


namespace Clazy {

CheckSetSelectionListModel::CheckSetSelectionListModel(CheckSetSelectionManager* checkSetSelectionManager,
                                                       QObject* parent)
    : QAbstractItemModel(parent)
    , m_checkSetSelectionManager(checkSetSelectionManager)
    , m_checkSetSelections(checkSetSelectionManager->checkSetSelections())
    , m_defaultCheckSetSelectionId(checkSetSelectionManager->defaultCheckSetSelectionId())
{
}

CheckSetSelectionListModel::~CheckSetSelectionListModel() = default;

int CheckSetSelectionListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_checkSetSelections.count();
}

int CheckSetSelectionListModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return 1;
}

QVariant CheckSetSelectionListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() ||
        index.row() < 0 || index.row() >= rowCount() ||
        index.column() != 0) {
        return {};
    }

    if (role == Qt::DisplayRole) {
        const int checkSetSelectionIndex = index.row();

        const auto& checkSetSelection = m_checkSetSelections.at(checkSetSelectionIndex);
        auto checkSetSelectionName = checkSetSelection.name();
        if (m_defaultCheckSetSelectionId == checkSetSelection.id()) {
            checkSetSelectionName = i18nc("@item:inlistbox", "%1 (default selection)", checkSetSelectionName);
        }
        return checkSetSelectionName;
    }

    return {};
}

QModelIndex CheckSetSelectionListModel::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return {};
    }
    return createIndex(row, column);
}

QModelIndex CheckSetSelectionListModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return {};
}

void CheckSetSelectionListModel::reload()
{
    beginResetModel();

    m_checkSetSelections = m_checkSetSelectionManager->checkSetSelections();
    m_defaultCheckSetSelectionId = m_checkSetSelectionManager->defaultCheckSetSelectionId();

    m_added.clear();
    m_edited.clear();
    m_removed.clear();
    m_defaultChanged = false;

    endResetModel();

    emit defaultCheckSetSelectionChanged(m_defaultCheckSetSelectionId);
}

void CheckSetSelectionListModel::store() const
{
    if (!m_edited.isEmpty()) {
        QVector<CheckSetSelection> m_checkSetSelectionsToSave;
        for (const auto& selection : std::as_const(m_checkSetSelections)) {
            const auto id = selection.id();
            if (m_edited.contains(id)) {
                m_checkSetSelectionsToSave.append(selection);
            }
        }
        m_checkSetSelectionManager->saveCheckSetSelections(m_checkSetSelectionsToSave);
    }
    m_checkSetSelectionManager->setDefaultCheckSetSelection(m_defaultCheckSetSelectionId);
    m_checkSetSelectionManager->removeCheckSetSelections(m_removed);

    m_added.clear();
    m_edited.clear();
    m_removed.clear();
    m_defaultChanged = false;
}


QString CheckSetSelectionListModel::checkSetSelectionId(const QModelIndex& index) const
{
    const int checkSetSelectionIndex = index.row();
    const bool isValidIndex =
        (0 <= checkSetSelectionIndex) && (checkSetSelectionIndex < m_checkSetSelections.count());

    return isValidIndex ? m_checkSetSelections.at(checkSetSelectionIndex).id() : QString();
}

QString CheckSetSelectionListModel::checkSetSelectionId(int row) const
{
    const bool isValidIndex =
        (0 <= row) && (row < m_checkSetSelections.count());

    return isValidIndex ? m_checkSetSelections.at(row).id() : QString();
}

QString CheckSetSelectionListModel::checkSetSelectionName(int row) const
{
    const bool isValidIndex =
        (0 <= row) && (row < m_checkSetSelections.count());

    return isValidIndex ? m_checkSetSelections.at(row).name() : QString();
}

QString CheckSetSelectionListModel::checkSetSelectionAsString(int row) const
{
    const bool isValidIndex =
        (0 <= row) && (row < m_checkSetSelections.count());

    return isValidIndex ? m_checkSetSelections.at(row).selectionAsString() : QString();
}

int CheckSetSelectionListModel::row(const QString& checkSetSelectionId) const
{
    int result = -1;

    const int checkSetSelectionsCount = m_checkSetSelections.count();
    for (int i = 0; i < checkSetSelectionsCount; ++i) {
        if (checkSetSelectionId == m_checkSetSelections.at(i).id()) {
            result = i;
            break;
        }
    }

    return result;
}

int CheckSetSelectionListModel::defaultCheckSetSelectionRow() const
{
    return row(m_defaultCheckSetSelectionId);
}

bool CheckSetSelectionListModel::hasCheckSetSelection(const QString& name) const
{
    return std::any_of(m_checkSetSelections.begin(), m_checkSetSelections.end(),
                       [name](const CheckSetSelection& checkSetSelection) {
        return checkSetSelection.name() == name;
    });
}


int CheckSetSelectionListModel::addCheckSetSelection(const QString& name)
{
    const int newRow = m_checkSetSelections.count();
    beginInsertRows(QModelIndex(), newRow, newRow);

    CheckSetSelection checkSetSelection;
    const QString id = QUuid::createUuid().toString();
    checkSetSelection.setId(id);
    checkSetSelection.setName(name);

    const bool isNewDefault = m_checkSetSelections.isEmpty();
    m_checkSetSelections.append(checkSetSelection);

    m_added.append(id);
    m_edited.insert(id);
    if (isNewDefault) {
        m_defaultChanged = true;
        m_defaultCheckSetSelectionId = id;
    }

    endInsertRows();
    if (isNewDefault) {
        emit defaultCheckSetSelectionChanged(m_defaultCheckSetSelectionId);
    }

    return newRow;
}

int CheckSetSelectionListModel::cloneCheckSetSelection(const QString& name, int row)
{
    const int newRow = m_checkSetSelections.count();
    beginInsertRows(QModelIndex(), newRow, newRow);

    const CheckSetSelection& original = m_checkSetSelections.at(row);
    CheckSetSelection checkSetSelection(original);
    const QString id = QUuid::createUuid().toString();
    checkSetSelection.setId(id);
    checkSetSelection.setName(name);

    m_checkSetSelections.append(checkSetSelection);

    m_added.append(id);
    m_edited.insert(id);

    endInsertRows();

    return newRow;
}

void CheckSetSelectionListModel::removeCheckSetSelection(int row)
{
    if ((row < 0) || m_checkSetSelections.count() <= row) {
        return;
    }

    beginRemoveRows(QModelIndex(), row, row);

    const QString id = m_checkSetSelections.at(row).id();

    m_checkSetSelections.removeAt(row);

    m_edited.remove(id);
    if (!m_added.removeOne(id)) {
        m_removed.append(id);
    }

    endRemoveRows();

    // set new default if this was the default before
    if (id == m_defaultCheckSetSelectionId) {
        if (m_checkSetSelections.isEmpty()) {
            m_defaultChanged = true;
            m_defaultCheckSetSelectionId.clear();
            emit defaultCheckSetSelectionChanged(m_defaultCheckSetSelectionId);
        } else {
            setDefaultCheckSetSelection(0);
        }
    }
}

void CheckSetSelectionListModel::setDefaultCheckSetSelection(int row)
{
    if ((row < 0) || m_checkSetSelections.count() <= row) {
        return;
    }

    const QString newDefaultId = m_checkSetSelections.at(row).id();
    if (newDefaultId == m_defaultCheckSetSelectionId) {
        return;
    }

    m_defaultCheckSetSelectionId = newDefaultId;
    m_defaultChanged = true;

    const QModelIndex modelIndex = index(row, 0);
    emit dataChanged(modelIndex, modelIndex);
    emit defaultCheckSetSelectionChanged(m_defaultCheckSetSelectionId);
}

void CheckSetSelectionListModel::setName(int row, const QString& name)
{
    if ((row < 0) || m_checkSetSelections.count() <= row) {
        return;
    }

    auto& checkSetSelection = m_checkSetSelections[row];
    if (checkSetSelection.name() == name) {
        return;
    }

    checkSetSelection.setName(name);

    const QString id = checkSetSelection.id();
    m_edited.insert(id);

    const QModelIndex modelIndex = index(row, 0);
    emit dataChanged(modelIndex, modelIndex);
    emit checkSetSelectionChanged(id);
}

void CheckSetSelectionListModel::setSelection(int row, const QString& selection)
{
    if ((row < 0) || m_checkSetSelections.count() <= row) {
        return;
    }

    auto& checkSetSelection = m_checkSetSelections[row];
    if (checkSetSelection.selectionAsString() == selection) {
        return;
    }

    checkSetSelection.setSelection(selection);

    const QString id = checkSetSelection.id();
    m_edited.insert(id);

    emit checkSetSelectionChanged(id);
}

}

#include "moc_checksetselectionlistmodel.cpp"

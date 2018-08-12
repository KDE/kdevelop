/*
 * This file is part of KDevelop
 *
 * Copyright 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "checklistmodel.h"

// plugin
#include <checkset.h>
#include <debug.h>
// KF
#include <KLocalizedString>


namespace {

Qt::CheckState checkState(ClangTidy::CheckGroup::EnabledState enabledState)
{
    return
        (enabledState == ClangTidy::CheckGroup::EnabledInherited) ? Qt::PartiallyChecked :
        (enabledState == ClangTidy::CheckGroup::Disabled) ?         Qt::Unchecked :
        /* else */                                                  Qt::Checked;
}

ClangTidy::CheckGroup::EnabledState enabledState(int checkState)
{
    return
        (checkState == Qt::PartiallyChecked) ? ClangTidy::CheckGroup::EnabledInherited :
        (checkState == Qt::Unchecked) ?        ClangTidy::CheckGroup::Disabled  :
        /* else */                             ClangTidy::CheckGroup::Enabled;
}

}

namespace ClangTidy
{

CheckListModel::CheckListModel(QObject* parent)
    : QAbstractItemModel(parent)
{
}

CheckListModel::~CheckListModel() = default;

QVariant CheckListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() ||
        (index.column() < NameColumnId) || (index.column() > CountColumnId)) {
        return QVariant();
    }

    auto* checkGroup = static_cast<const CheckGroup*>(index.internalPointer());

    // root?
    if (!checkGroup) {
        if (index.row() != 0) {
            return QVariant();
        }
        if (index.column() == NameColumnId) {
            if (role == Qt::DisplayRole) {
                return i18n("All checks");
            } else
            if (role == Qt::CheckStateRole) {
                return checkState(m_rootCheckGroup->groupEnabledState());
            } else
            if (role == EffectiveEnabledStateRole) {
                return m_rootCheckGroup->effectiveGroupEnabledState();
            }
        } else {
            if (role == Qt::DisplayRole) {
                const int enabledChecksCount = m_rootCheckGroup->enabledChecksCount();
                if (enabledChecksCount > 0) {
                    return enabledChecksCount;
                }
            } else
            if (role == Qt::TextAlignmentRole) {
                return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
            }
        }
    } else {
        const int childIndex = index.row();
        if (childIndex < 0 || childCount(checkGroup) <= childIndex) {
            return QVariant();
        }
        const int subGroupsCount = checkGroup->subGroups().count();
        if (childIndex < subGroupsCount) {
            const int subGroupIndex = childIndex;
            if (index.column() == NameColumnId) {
                if (role == Qt::DisplayRole) {
                    auto* subGroup = checkGroup->subGroups().at(subGroupIndex);
                    return subGroup->wildCardText();
                } else
                if (role == Qt::CheckStateRole) {
                    auto* subGroup = checkGroup->subGroups().at(subGroupIndex);
                    return checkState(subGroup->groupEnabledState());
                } else
                if (role == EffectiveEnabledStateRole) {
                    auto* subGroup = checkGroup->subGroups().at(subGroupIndex);
                    return subGroup->effectiveGroupEnabledState();
                }
            } else {
                if (role == Qt::DisplayRole) {
                    auto* subGroup = checkGroup->subGroups().at(subGroupIndex);
                    const int enabledChecksCount = subGroup->enabledChecksCount();
                    if (enabledChecksCount > 0) {
                        return enabledChecksCount;
                    }
                } else
                if (role == Qt::TextAlignmentRole) {
                    return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
                }
            }
        } else {
            if (index.column() == NameColumnId) {
                const int checkIndex = childIndex - subGroupsCount;
                if (role == Qt::DisplayRole) {
                    return checkGroup->checkNames().at(checkIndex);
                } else
                if (role == Qt::CheckStateRole) {
                    return checkState(checkGroup->checkEnabledState(checkIndex));
                } else
                if (role == EffectiveEnabledStateRole) {
                    return checkGroup->effectiveCheckEnabledState(checkIndex);
                }
            }
        }
    }

    return QVariant();
}

bool CheckListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() ||
        (role != Qt::CheckStateRole)) {
        return false;
    }

    const auto enabledState = ::enabledState(value.toInt());
    auto* checkGroup = static_cast<CheckGroup*>(index.internalPointer());

    // root?
    if (!checkGroup) {
        if (index.row() != 0) {
            return false;
        }

        m_rootCheckGroup->setGroupEnabledState(enabledState);

        m_isDefault = false;
        emit dataChanged(index, index);
        emitSubGroupDataChanged(m_rootCheckGroup.data());
        emit enabledChecksChanged();
        return true;
    } else {
        const int childIndex = index.row();
        if (childIndex < 0 || childCount(checkGroup) <= childIndex) {
            return false;
        }

        CheckGroup* changedSubGroup = nullptr;
        const int subGroupsCount = checkGroup->subGroups().count();
        if (childIndex < subGroupsCount) {
            const int subGroupIndex = childIndex;
            auto* subGroup = checkGroup->subGroups().at(subGroupIndex);
            const auto oldEffectiveGroupEnabledState = subGroup->effectiveGroupEnabledState();
            subGroup->setGroupEnabledState(enabledState);
            if (oldEffectiveGroupEnabledState != subGroup->effectiveGroupEnabledState()) {
                changedSubGroup = subGroup;
            }
        } else {
            const int checkIndex = childIndex - subGroupsCount;
            checkGroup->setCheckEnabledState(checkIndex, enabledState);
        }

        m_isDefault = false;
        // TODO: does this result in the subtree being updated, needed as effective state could have changed?
        emit dataChanged(index, index);
        if (changedSubGroup) {
            emitSubGroupDataChanged(changedSubGroup);
        }
        emit enabledChecksChanged();
        return true;
    }

    return false;
}

int CheckListModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return 2;
}

int CheckListModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid()) {
        return m_rootCheckGroup ? 1 : 0;
    }

    auto* parentCheckGroup = static_cast<const CheckGroup*>(parent.internalPointer());

    if (!parentCheckGroup) {
        return childCount(m_rootCheckGroup.data());
    }

    // is subgroup?
    const int subGroupIndex = parent.row();
    const auto& subGroups = parentCheckGroup->subGroups();
    if (0 <= subGroupIndex && subGroupIndex < subGroups.count()) {
        return childCount(subGroups.at(subGroupIndex));
    }

    return 0;
}

QModelIndex CheckListModel::parent(const QModelIndex& child) const
{
    if (!child.isValid()) {
        return QModelIndex();
    }

    auto* childCheckGroup = static_cast<CheckGroup*>(child.internalPointer());
    if (!childCheckGroup) {
        return QModelIndex();
    }
    if (childCheckGroup == m_rootCheckGroup.data()) {
        return createIndex(0, 0, nullptr);
    }
    auto* parentCheckGroup = childCheckGroup->superGroup();
    const auto& subGroups = parentCheckGroup->subGroups();
    const int parentRow = subGroups.indexOf(childCheckGroup);
    return createIndex(parentRow, 0, parentCheckGroup);
}

QModelIndex CheckListModel::index(int row, int column, const QModelIndex& parent) const
{
    if ((column < NameColumnId) ||(column > CountColumnId) ||
        (row < 0) ||
        !m_rootCheckGroup) {
        return QModelIndex();
    }

    if (!parent.isValid()) {
        if (1 <= row) {
            return QModelIndex();
        }
        return createIndex(row, column, nullptr);
    }

    auto* superCheckGroup = checkGroup(parent);
    if (row >= childCount(superCheckGroup)) {
        return QModelIndex();
    }

    return createIndex(row, column, superCheckGroup);
}

Qt::ItemFlags CheckListModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    if (index.column() == CountColumnId) {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }

    auto* checkGroup = static_cast<const CheckGroup*>(index.internalPointer());

    // root?
    if (!checkGroup) {
        return  Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
    }

    return  Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsUserTristate;
}

int CheckListModel::childCount(const CheckGroup* checkGroup) const
{
    return checkGroup->subGroups().count() + checkGroup->checkNames().count();
}

CheckGroup* CheckListModel::checkGroup(const QModelIndex& index) const
{
    auto* superCheckGroup = static_cast<CheckGroup*>(index.internalPointer());
    if (!superCheckGroup) {
        return m_rootCheckGroup.data();
    }

    const int subGroupIndex = index.row();
    const auto& subGroups = superCheckGroup->subGroups();
    Q_ASSERT(0 <= subGroupIndex && subGroupIndex < subGroups.count());
    return subGroups.at(subGroupIndex);
}


void CheckListModel::setCheckSet(const CheckSet* checkSet)
{
    beginResetModel();

    m_checkSet = checkSet;

    m_rootCheckGroup.reset(CheckGroup::fromPlainList(m_checkSet->all()));

    if (m_isDefault) {
        m_rootCheckGroup->setEnabledChecks(m_checkSet->defaults());
    }

    endResetModel();
}


QStringList CheckListModel::enabledChecks() const
{
    if (m_isDefault) {
        return QStringList();
    }

    return m_rootCheckGroup->enabledChecksRules();
}

void CheckListModel::setEnabledChecks(const QStringList& enabledChecks)
{
    beginResetModel();

    if (enabledChecks.isEmpty() && m_checkSet) {
        m_rootCheckGroup->setEnabledChecks(m_checkSet->defaults());
        m_isDefault = true;
    } else {
        m_rootCheckGroup->setEnabledChecks(enabledChecks);
        m_isDefault = false;
    }

    endResetModel();
}

void CheckListModel::emitSubGroupDataChanged(CheckGroup* checkGroup)
{
    const int rowCount = childCount(checkGroup);
    if (rowCount > 0) {
        const auto firstIndex = createIndex(0, NameColumnId, checkGroup);
        const auto lastIndex = createIndex(rowCount-1, CountColumnId, checkGroup);
        emit dataChanged(firstIndex, lastIndex);
    }

    for (auto* subGroup : checkGroup->subGroups()) {
        emitSubGroupDataChanged(subGroup);
    }
}

}

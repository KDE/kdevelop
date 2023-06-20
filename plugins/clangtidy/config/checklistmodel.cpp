/*
    SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
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
        if (role == HasExplicitEnabledStateRole) {
            return m_rootCheckGroup->hasSubGroupWithExplicitEnabledState();
        } else {
            if (index.column() == NameColumnId) {
                if (role == Qt::DisplayRole) {
                    return i18nc("@item", "All checks");
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
        }
    } else {
        const int childIndex = index.row();
        if (childIndex < 0 || childCount(checkGroup) <= childIndex) {
            return QVariant();
        }
        const int subGroupsCount = checkGroup->subGroups().count();
        if (childIndex < subGroupsCount) {
            const int subGroupIndex = childIndex;
            if (role == HasExplicitEnabledStateRole) {
                auto* subGroup = checkGroup->subGroups().at(subGroupIndex);
                return subGroup->hasSubGroupWithExplicitEnabledState();
            } else {
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
        emitSubGroupDataChanged(index);
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
        if (changedSubGroup) {
            emitSubGroupDataChanged(index);
        } else {
            emit dataChanged(index, index, {Qt::CheckStateRole});
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

    if (!m_isEditable) {
        return  Qt::ItemIsEnabled | Qt::ItemIsSelectable;
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

void CheckListModel::setEditable(bool editable)
{
    if (m_isEditable == editable) {
        return;
    }

    beginResetModel();
    m_isEditable = editable;
    endResetModel();
}

void CheckListModel::emitSubGroupDataChanged(const QModelIndex& subGroupIndex)
{
    // first group itself
    emit dataChanged(subGroupIndex, subGroupIndex, {Qt::CheckStateRole});
    const auto countIndex = subGroupIndex.sibling(subGroupIndex.row(), CountColumnId);
    emit dataChanged(countIndex, countIndex, {Qt::DisplayRole});

    auto* checkGroup = this->checkGroup(subGroupIndex);
    const int subGroupsCount = checkGroup->subGroups().count();

    // subgroups
    for (int i = 0; i < subGroupsCount; ++i) {
        const auto subSubGroupInxex = index(i, NameColumnId, subGroupIndex);
        emitSubGroupDataChanged(subSubGroupInxex);
    }

    // checks
    const int checksCount = checkGroup->checkNames().count();
    if (checksCount > 0) {
        const int firstChecksRow = subGroupsCount;
        const int lastChecksRow = firstChecksRow + checksCount - 1;
        const auto firstCheckIndex = index(firstChecksRow, NameColumnId, subGroupIndex);
        const auto lastCheckIndex = index(lastChecksRow, NameColumnId, subGroupIndex);
        emit dataChanged(firstCheckIndex, lastCheckIndex, {Qt::CheckStateRole});
    }
}

}

#include "moc_checklistmodel.cpp"

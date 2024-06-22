/*
    SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "checkgroup.h"

// plugin
#include <debug.h>
// Qt
#include <QRegularExpression>

namespace ClangTidy
{

CheckGroup* CheckGroup::fromPlainList(const QStringList& checks)
{
    auto* result = new CheckGroup;
    // root group cannot inherit
    result->m_groupEnabledState = Disabled;

    for (const auto& checkName : checks) {
        result->addCheck(checkName);
    }

    return result;
}

CheckGroup::CheckGroup(const QString& name, CheckGroup* superGroup)
    : m_superGroup(superGroup)
    , m_prefix(name)
{
}

CheckGroup::~CheckGroup()
{
    qDeleteAll(m_subGroups);
}


void CheckGroup::addCheck(const QString& checkName)
{
    const int nextSplitOffset = checkName.indexOf(QRegularExpression(QStringLiteral("[-.]")), m_prefix.length()); 

    // 1. check if looking at last section, if so add to current group
    if (nextSplitOffset < 0) {
        m_checks.append(checkName);
        m_checksEnabledStates.append(EnabledInherited);
        return;
    }

    // 2. check if existing subgroup for prefix, if so add to that
    // include separator into subgroup name
    const auto subGroupName = QStringView{checkName}.left(nextSplitOffset + 1);
    for (auto* subGroup : std::as_const(m_subGroups)) {
        if (subGroup->prefix() == subGroupName) {
            subGroup->addCheck(checkName);
            return;
        }
    }

    // 3. check if existing check with same prefix, if so create subgroup for them
    for (int i = 0; i < m_checks.size(); ++i) {
        const auto& listedCheckName = m_checks.at(i);
        if (listedCheckName.startsWith(subGroupName)) {
            auto* newSubGroup = new CheckGroup(subGroupName.toString(), this);
            newSubGroup->addCheck(listedCheckName);
            newSubGroup->addCheck(checkName);
            m_subGroups.append(newSubGroup);
            m_checks.removeAt(i);
            m_checksEnabledStates.removeAt(i);
            return;
        }
    }

    // 4. add to current group
    m_checks.append(checkName);
    m_checksEnabledStates.append(EnabledInherited);
}

void CheckGroup::setEnabledChecks(const QStringList& rules)
{
    // TODO: optimize & check first rule if not matching all
    resetEnabledState(Disabled);

    for (const auto& rule : rules) {
        int matchStartPos = 0;
        EnabledState enabledState = Enabled;
        if (rule.startsWith(QLatin1Char('-'))) {
            matchStartPos = 1;
            enabledState = Disabled;
        }
        applyEnabledRule(QStringView{rule}.mid(matchStartPos), enabledState);
    }

    m_enabledChecksCountDirty = true;
    setEnabledChecksCountDirtyInSubGroups();
}

void CheckGroup::applyEnabledRule(QStringView rule, EnabledState enabledState)
{
    // this group?
    if (rule == wildCardText()) {
        resetEnabledState(enabledState);
        return;
    }

    for (auto* subGroup : std::as_const(m_subGroups)) {
        if (rule.startsWith(subGroup->prefix())) {
            subGroup->applyEnabledRule(rule, enabledState);
            return;
        }
    }

    for (int i = 0; i < m_checks.size(); ++i) {
        if (m_checks.at(i) == rule) {
            m_checksEnabledStates[i] = enabledState;
            return;
        }
    }
}

void CheckGroup::resetEnabledState(EnabledState enabledState)
{
    m_groupEnabledState = enabledState;

    for (auto* subGroup : std::as_const(m_subGroups)) {
        subGroup->resetEnabledState(EnabledInherited);
    }
    m_checksEnabledStates.fill(EnabledInherited);
}

QStringList CheckGroup::enabledChecksRules() const
{
    QStringList result;
    collectEnabledChecks(result);
    return result;
}

void CheckGroup::collectEnabledChecks(QStringList& enabledChecks) const
{
    const auto effectiveGroupEnabledState = this->effectiveGroupEnabledState();

    const bool appendGroupRule =
        (!m_superGroup) ||
        (m_superGroup->effectiveGroupEnabledState() != effectiveGroupEnabledState);
    if (appendGroupRule) {
        QString rule = wildCardText();
        if (effectiveGroupEnabledState == CheckGroup::Disabled) {
            rule.prepend(QLatin1Char('-'));
        }
        enabledChecks.append(rule);
    }

    for (const auto* subGroup : m_subGroups) {
        subGroup->collectEnabledChecks(enabledChecks);
    }

    for (int i = 0; i < m_checks.size(); ++i) {
        const auto effectiveCheckEnabledState = this->effectiveCheckEnabledState(i);
        if (effectiveGroupEnabledState != effectiveCheckEnabledState) {
            QString rule = m_checks.at(i);
            if (effectiveCheckEnabledState == CheckGroup::Disabled) {
                rule.prepend(QLatin1Char('-'));
            }
            enabledChecks.append(rule);
        }
    }
}

const QString& CheckGroup::prefix() const
{
    return m_prefix;
}

QString CheckGroup::wildCardText() const
{
    return m_prefix + QLatin1Char('*');
}

const QStringList& CheckGroup::checkNames() const
{
    return m_checks;
}

const QVector<CheckGroup*>& CheckGroup::subGroups() const
{
    return m_subGroups;
}

CheckGroup * CheckGroup::superGroup() const
{
    return m_superGroup;
}

CheckGroup::EnabledState CheckGroup::groupEnabledState() const
{
    return m_groupEnabledState;
}

CheckGroup::EnabledState CheckGroup::effectiveGroupEnabledState() const
{
    EnabledState result = m_groupEnabledState;
    if (result == EnabledInherited) {
        Q_ASSERT(m_superGroup);
        result = m_superGroup->effectiveGroupEnabledState();
    }
    return result;
}

CheckGroup::EnabledState CheckGroup::checkEnabledState(int index) const
{
    return m_checksEnabledStates.at(index);
}

CheckGroup::EnabledState CheckGroup::effectiveCheckEnabledState(int index) const
{
    EnabledState result = m_checksEnabledStates.at(index);
    if (result == EnabledInherited) {
        result = effectiveGroupEnabledState();
    }
    return result;
}

void CheckGroup::setGroupEnabledState(CheckGroup::EnabledState groupEnabledState)
{
    const int oldEffectiveGroupEnabledState = effectiveGroupEnabledState();

    m_groupEnabledState = groupEnabledState;

    if (oldEffectiveGroupEnabledState != effectiveGroupEnabledState()) {
        setEnabledChecksCountDirtyInSuperGroups();
        setEnabledChecksCountDirtyInSubGroups();
    }
}


void CheckGroup::setCheckEnabledState(int index, CheckGroup::EnabledState checkEnabledState)
{
    const int oldEffectiveCheckEnabledState = effectiveCheckEnabledState(index);

    m_checksEnabledStates[index] = checkEnabledState;

    if (oldEffectiveCheckEnabledState != effectiveCheckEnabledState(index)) {
        setEnabledChecksCountDirtyInSuperGroups();
    }
}

void CheckGroup::updateData() const
{
    if (m_enabledChecksCountDirty) {
        m_enabledChecksCount = 0;
        m_hasSubGroupWithExplicitEnabledState = false;

        for (auto* subGroup : m_subGroups) {
            m_enabledChecksCount += subGroup->enabledChecksCount();
            m_hasSubGroupWithExplicitEnabledState |= subGroup->hasSubGroupWithExplicitEnabledState();
            m_hasSubGroupWithExplicitEnabledState |= (subGroup->groupEnabledState() != EnabledInherited);
        }

        for (int i = 0; i < m_checks.size(); ++i) {
            if (effectiveCheckEnabledState(i) == Enabled) {
                ++m_enabledChecksCount;
            }
            m_hasSubGroupWithExplicitEnabledState |= (m_checksEnabledStates[i] != EnabledInherited);
        }
        m_enabledChecksCountDirty = false;
    }
}

int CheckGroup::enabledChecksCount() const
{
    updateData();
    return m_enabledChecksCount;
}

bool CheckGroup::hasSubGroupWithExplicitEnabledState() const
{
    updateData();
    return m_hasSubGroupWithExplicitEnabledState;
}

void CheckGroup::setEnabledChecksCountDirtyInSuperGroups()
{
    auto* checkGroup = this;
    while (checkGroup) {
        checkGroup->m_enabledChecksCountDirty = true;
        checkGroup = checkGroup->superGroup();
    }
}

void CheckGroup::setEnabledChecksCountDirtyInSubGroups()
{
    for (auto* subGroup : std::as_const(m_subGroups)) {
        subGroup->m_enabledChecksCountDirty = true;
        subGroup->setEnabledChecksCountDirtyInSubGroups();
    }
}

}

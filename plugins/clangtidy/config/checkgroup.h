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

#ifndef CLANGTIDY_CHECKGROUP_H
#define CLANGTIDY_CHECKGROUP_H

// Qt
#include <QStringList>
#include <QVector>

namespace ClangTidy
{

class CheckGroup
{
public:
    enum EnabledState {
        Disabled,
        Enabled,
        EnabledInherited,
    };

    static CheckGroup* fromPlainList(const QStringList& checks);

    ~CheckGroup();

private:
    explicit CheckGroup(const QString& name, CheckGroup* superGroup = nullptr);
    CheckGroup() = default;
    Q_DISABLE_COPY(CheckGroup)

public:
    const QString& prefix() const;
    CheckGroup* superGroup() const;
    const QStringList& checkNames() const;
    const QVector<CheckGroup*>& subGroups() const;

    EnabledState groupEnabledState() const;
    EnabledState effectiveGroupEnabledState() const;

    EnabledState checkEnabledState(int index) const;
    EnabledState effectiveCheckEnabledState(int index) const;

    QString wildCardText() const;
    QStringList enabledChecksRules() const;
    int enabledChecksCount() const;
    bool hasSubGroupWithExplicitEnabledState() const;

    void setGroupEnabledState(EnabledState groupEnabledState);
    void setCheckEnabledState(int index, EnabledState checkEnabledState);

    void setEnabledChecks(const QStringList& rules);

private:
    void addCheck(const QString& checkName);
    void applyEnabledRule(const QStringRef& rule, EnabledState enabledState);
    void resetEnabledState(EnabledState enabledState);
    void collectEnabledChecks(QStringList& enabledChecks) const;

    void setEnabledChecksCountDirtyInSuperGroups();
    void setEnabledChecksCountDirtyInSubGroups();
    void updateData() const;

private:
    CheckGroup* m_superGroup = nullptr;

    EnabledState m_groupEnabledState = EnabledInherited;
    QVector<EnabledState> m_checksEnabledStates;
    QString m_prefix;
    QVector<CheckGroup*> m_subGroups;
    QStringList m_checks;

    mutable int m_enabledChecksCount = 0;
    mutable bool m_enabledChecksCountDirty = false;
    mutable bool m_hasSubGroupWithExplicitEnabledState = false;
};

}

#endif

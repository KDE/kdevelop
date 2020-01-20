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

#ifndef CLANGTIDY_CHECKLISTMODEL_H
#define CLANGTIDY_CHECKLISTMODEL_H

// plugin
#include "checkgroup.h"
// Qt
#include <QAbstractItemModel>

namespace ClangTidy
{

class CheckSet;

class CheckListModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum Roles {
        EffectiveEnabledStateRole = Qt::UserRole+1,
        HasExplicitEnabledStateRole
    };

    enum ColumIds {
        NameColumnId = 0,
        CountColumnId = 1
    };

public:
    explicit CheckListModel(QObject* parent = nullptr);
    ~CheckListModel() override;

public: // QAbstractItemModel API
    QVariant data(const QModelIndex& index, int role) const override;
    int columnCount(const QModelIndex& parent) const override;
    int rowCount(const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;

public:
    void setCheckSet(const CheckSet* checkSet);
    void setEnabledChecks(const QStringList& enabledChecks);
    QStringList enabledChecks() const;
    void setEditable(bool editable);

Q_SIGNALS:
    void enabledChecksChanged();

private:
    int childCount(const CheckGroup* checkGroup) const;
    CheckGroup* checkGroup(const QModelIndex& index) const;
    void emitSubGroupDataChanged(const QModelIndex& subGroupIndex);

private:
    const CheckSet* m_checkSet = nullptr;

    QScopedPointer<CheckGroup> m_rootCheckGroup;
    bool m_isDefault = true;
    bool m_isEditable = true;
};

}

#endif // CHECKLISTMODEL_H

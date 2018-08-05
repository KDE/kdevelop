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

// Qt
#include <QAbstractItemModel>

namespace ClangTidy
{

class CheckSet;

class CheckListModel : public QAbstractItemModel
{
    Q_OBJECT

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
    void setSelectedChecks(const QStringList& checks);
    QStringList selectedChecks() const;

Q_SIGNALS:
    void selectedChecksChanged();

private:
    const CheckSet* m_checkSet = nullptr;
    QStringList m_selectedChecks;
    bool m_isDefault = true;
};

}

#endif // CHECKLISTMODEL_H

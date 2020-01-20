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

#ifndef CLANGTIDY_CHECKSELECTION_H
#define CLANGTIDY_CHECKSELECTION_H

// Qt
#include <QWidget>

class QTreeView;
class QSortFilterProxyModel;

namespace ClangTidy
{

class CheckSet;
class CheckListModel;
class CheckListItemProxyStyle;

class CheckSelection : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString checks READ checks WRITE setChecks NOTIFY checksChanged USER true)

public:
    explicit CheckSelection(QWidget* parent = nullptr);
    ~CheckSelection() override;

public:
    void setCheckSet(const CheckSet* checkSet);

    void setChecks(const QString& checks);
    QString checks() const;

    void setEditable(bool editable);

protected: // QObject API
    bool event(QEvent *event) override;

Q_SIGNALS:
    void checksChanged(const QString& checks);

private:
    void expandSubGroupsWithExplicitlyEnabledStates();
    void expandSubGroupsWithExplicitlyEnabledStates(const QModelIndex& groupIndex);

private Q_SLOTS:
    void onEnabledChecksChanged();

private:
    const CheckSet* m_checkSet = nullptr;
    CheckListModel* m_checkListModel;
    QSortFilterProxyModel* m_checksFilterProxyModel;
    QTreeView* m_checkListView;
    CheckListItemProxyStyle* m_proxyStyle;
};

}
#endif

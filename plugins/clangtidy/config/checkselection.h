/*
    SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
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
    CheckListModel* m_checkListModel;
    QSortFilterProxyModel* m_checksFilterProxyModel;
    QTreeView* m_checkListView;
    CheckListItemProxyStyle* m_proxyStyle;
};

}
#endif

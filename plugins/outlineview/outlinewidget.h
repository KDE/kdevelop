/*
    SPDX-FileCopyrightText: 2010, 2015 Alex Richardson <alex.richardson@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#pragma once

#include <QWidget>

class QSortFilterProxyModel;
class QModelIndex;
class QAction;
class QTreeView;
class QLineEdit;
class OutlineModel;
class OutlineViewPlugin;

class OutlineWidget : public QWidget
{
    Q_OBJECT

public:
    OutlineWidget(QWidget* parent, OutlineViewPlugin* plugin);
    ~OutlineWidget() override;

private:
    OutlineViewPlugin* m_plugin;
    OutlineModel* m_model;
    QTreeView* m_tree;
    QSortFilterProxyModel* m_proxy;
    QLineEdit* m_filter;
    QAction* m_sortAlphabeticallyAction;

    Q_DISABLE_COPY(OutlineWidget)
public Q_SLOTS:
    void activated(const QModelIndex& index);
    void expandFirstLevel();
};

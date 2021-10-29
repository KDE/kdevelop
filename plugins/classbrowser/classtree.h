/*
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Lior Mualem <lior.m.kde@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_CLASSTREE_H
#define KDEVPLATFORM_PLUGIN_CLASSTREE_H

#include <QTreeView>
#include <QPointer>

#include <language/duchain/identifier.h>

#include "language/util/navigationtooltip.h"

class ClassBrowserPlugin;
class ClassModel;

class ClassTree
    : public QTreeView
{
    Q_OBJECT

public:
    ClassTree(QWidget* parent, ClassBrowserPlugin* plugin);
    ~ClassTree() override;

public:
    /// Find the given a_id in the tree and highlight it.
    void highlightIdentifier(const KDevelop::IndexedQualifiedIdentifier& a_id);

    static bool populatingClassBrowserContextMenu();

protected:
    void contextMenuEvent(QContextMenuEvent* e) override;
    ClassModel* model();
    bool event(QEvent* event) override;

private Q_SLOTS:
    void itemActivated(const QModelIndex& index);

private:
    ClassBrowserPlugin* m_plugin;
    QPointer<KDevelop::NavigationToolTip> m_tooltip;
};

#endif

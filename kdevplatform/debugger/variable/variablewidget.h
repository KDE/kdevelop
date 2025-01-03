/*
    SPDX-FileCopyrightText: 1999 John Birch <jbb@kdevelop.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_VARIABLEWIDGET_H
#define KDEVPLATFORM_VARIABLEWIDGET_H

#include <debugger/debuggerexport.h>
#include "../util/treeview.h"

class KHistoryComboBox;
class QSortFilterProxyModel;
class QAction;

namespace KDevelop
{

class IDebugController;
class Variable;
class VariablesRoot;
class VariableTree;

class KDEVPLATFORMDEBUGGER_EXPORT VariableWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VariableWidget( IDebugController *controller, QWidget *parent=nullptr );

Q_SIGNALS:
    void addWatchVariable(const QString& indent);
    void evaluateExpression(const QString& indent);

public Q_SLOTS:
    void slotAddWatch(const QString &ident);

protected:
    void showEvent(QShowEvent* e) override;
    void hideEvent(QHideEvent* e) override;

private:
    VariableTree *m_varTree;
    KHistoryComboBox *m_watchVarEditor;
    VariablesRoot *m_variablesRoot;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

class VariableTree : public KDevelop::AsyncTreeView
{
    Q_OBJECT
public:
    VariableTree(IDebugController *controller, VariableWidget *parent, QSortFilterProxyModel *proxy);
    ~VariableTree() override;

private:
    void setupActions();
    void contextMenuEvent(QContextMenuEvent* event) override;
    QModelIndex mapViewIndexToTreeModelIndex(const QModelIndex& viewIndex) const override;
    Variable *selectedVariable() const;

private Q_SLOTS:
    void changeVariableFormat(int);
    void watchDelete();
    void copyVariableValue();
    void stopOnChange();

private:
    QAction *m_contextMenuTitle;
    QMenu *m_formatMenu;
    QAction *m_watchDelete;
    QAction *m_copyVariableValue;
    QAction *m_stopOnChange;
    QSortFilterProxyModel *m_proxy;
};

}

#endif

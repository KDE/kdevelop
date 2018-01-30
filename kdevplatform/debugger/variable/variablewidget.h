/***************************************************************************
    begin                : Sun Aug 8 1999
    copyright            : (C) 1999 by John Birch
    email                : jbb@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_VARIABLEWIDGET_H
#define KDEVPLATFORM_VARIABLEWIDGET_H

#include <debugger/debuggerexport.h>
#include "../util/treeview.h"
#include "variablecollection.h"

class KHistoryComboBox;
class QSortFilterProxyModel;
class QAction;

namespace KDevelop
{

class IDebugController;
class TreeModel;
class VariableTree;
class AbstractVariableItem;

class KDEVPLATFORMDEBUGGER_EXPORT VariableWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VariableWidget( IDebugController *controller, QWidget *parent=nullptr );

Q_SIGNALS:
    void requestRaise();
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
    QSortFilterProxyModel *m_proxy;
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

    VariableCollection* collection() const;

private:
    void setupActions();
    void contextMenuEvent(QContextMenuEvent* event) override;
    Variable *selectedVariable() const;

private Q_SLOTS:
    void changeVariableFormat(int);
    void watchDelete();
    void copyVariableValue();
    void stopOnChange();

#if 0
Q_SIGNALS:
    void toggleWatchpoint(const QString &varName);

protected:
    virtual void contextMenuEvent(QContextMenuEvent* event);
    virtual void keyPressEvent(QKeyEvent* e);
    virtual void showEvent(QShowEvent* event);

private: // helper functions
    void handleAddressComputed(const GDBMI::ResultRecord& r);

    void updateCurrentFrame();

    void copyToClipboard(AbstractVariableItem* item);

    QMenu* activePopup_;
    QAction* toggleWatch_;
#endif
private:
    QAction *m_contextMenuTitle;
    QMenu *m_formatMenu;
    QAction *m_watchDelete;
    QAction *m_copyVariableValue;
    QAction *m_stopOnChange;
    QSortFilterProxyModel *m_proxy;
    TreeModel *m_model;
};

}

#endif

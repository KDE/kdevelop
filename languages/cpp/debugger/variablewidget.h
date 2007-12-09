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

#ifndef _VARIABLEWIDGET_H_
#define _VARIABLEWIDGET_H_

#include <QTreeView>

#include <kcombobox.h>

#include "gdbcontroller.h"
#include "mi/gdbmi.h"

class KMenu;
class KLineEdit;
class KHistoryComboBox;

namespace GDBDebugger
{

class VariableTree;
class CppDebuggerPlugin;
class AbstractVariableItem;

class VariableWidget : public QWidget
{
    Q_OBJECT

public:
    VariableWidget( CppDebuggerPlugin* plugin, GDBController* controller,
                    QWidget *parent=0 );

Q_SIGNALS:
    void requestRaise();
    void addWatchVariable(const QString& indent);
    void evaluateExpression(const QString& indent);

public Q_SLOTS:
    void slotAddWatchVariable();
    void slotAddWatchVariable(const QString &ident);
    void slotEvaluateExpression();
    void slotEvaluateExpression(const QString &ident);

private:
    VariableTree *varTree_;
//    KLineEdit *watchVarEntry_;

    KHistoryComboBox *watchVarEditor_;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

class VariableTree : public QTreeView
{
    Q_OBJECT
public:
    VariableTree(VariableWidget *parent, GDBController* controller);
    virtual ~VariableTree();

    GDBController* controller() const;
    VariableCollection* collection() const;

Q_SIGNALS:
    void toggleWatchpoint(const QString &varName);

protected:
    virtual void contextMenuEvent(QContextMenuEvent* event);
    virtual void keyPressEvent(QKeyEvent* e);

private: // helper functions
    void handleAddressComputed(const GDBMI::ResultRecord& r);

    void updateCurrentFrame();

    void copyToClipboard(AbstractVariableItem* item);

private:
    GDBController*  controller_;

    KMenu* activePopup_;
    QAction* toggleWatch_;
};

}

#endif

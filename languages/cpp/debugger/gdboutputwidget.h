// *************************************************************************
//                          gdboutputwidget.cpp  -  description
//                             -------------------
//    begin                : 10th April 2003
//    copyright            : (C) 2003 by John Birch
//    email                : jbb@kdevelop.org
// **************************************************************************
//
// **************************************************************************
// *                                                                        *
// *   This program is free software; you can redistribute it and/or modify *
// *   it under the terms of the GNU General Public License as published by *
// *   the Free Software Foundation; either version 2 of the License, or    *
// *   (at your option) any later version.                                  *
// *                                                                        *
// **************************************************************************

#ifndef _GDBOUTPUTWIDGET_H_
#define _GDBOUTPUTWIDGET_H_

#include <qwidget.h>
#include <q3textedit.h>
#include <QTimer>
#include <QStringList>
#include <Q3PopupMenu>
#include <QFocusEvent>
#include <QStringList>

#include "gdbglobal.h"

class KHistoryComboBox;

class Q3TextEdit;
class QToolButton;

namespace GDBDebugger
{

class GDBController;
class CppDebuggerPlugin;

class GDBOutputWidget : public QWidget
{
    Q_OBJECT

public:
    GDBOutputWidget(CppDebuggerPlugin* plugin, GDBController* controller, QWidget *parent=0 );
    ~GDBOutputWidget();

    void savePartialProjectSession();
    void restorePartialProjectSession();

public Q_SLOTS:
    void clear();

    void slotInternalCommandStdout(const QString& line);
    void slotUserCommandStdout(const QString& line);
    void slotReceivedStderr(const char* line);
    void slotStateChanged(DBGStateFlags oldStatus, DBGStateFlags newStatus);

    void slotGDBCmd();

    void flushPending();

protected:
    virtual void focusInEvent(QFocusEvent *e);

Q_SIGNALS:
    void userGDBCmd(const QString &cmd);
    void breakInto();

private:

    QString html_escape(const QString& s);

    void newStdoutLine(const QString& line, bool internal);

    /** Arranges for 'line' to be shown to the user.
        Adds 'line' to pendingOutput_ and makes sure
        updateTimer_ is running. */
    void showLine(const QString& line);

    /** Makes 'l' no longer then 'max_size' by
        removing excessive elements from the top.
    */
    void trimList(QStringList& l, int max_size);

    GDBController* m_controller;
    KHistoryComboBox*  m_userGDBCmdEditor;
    QToolButton*    m_Interrupt;
    Q3TextEdit*      m_gdbView;

    void setShowInternalCommands(bool);
    friend class OutputText;

    /** The output from user commands only and from
        all commands. We keep it here so that if we switch
        "Show internal commands" on, we can show previous 
        internal commands. 
    */
    QStringList userCommands_, allCommands_;
    /** Same output, without any fancy formatting.  Keeping it
        here because I can't find any way to extract raw text,
        without formatting, out of QTextEdit except for
        selecting everything and calling 'copy()'. The latter
        approach is just ugly.  */
    QStringList userCommandsRaw_, allCommandsRaw_;


    /** For performance reasons, we don't immediately add new text
        to QTExtEdit. Instead we add it to pendingOutput_ and 
        flush it on timer.
    */
    QString pendingOutput_;
    QTimer updateTimer_;

    bool showInternalCommands_;

    int maxLines_;
};

/** Add popup menu specific to gdb output window to QTextEdit.
*/
class OutputText : public Q3TextEdit
{
    Q_OBJECT
public:
    OutputText(GDBOutputWidget* parent) 
    : Q3TextEdit(parent), 
      parent_(parent)
    {}

    Q3PopupMenu* createPopupMenu(const QPoint& pos);

private Q_SLOTS:
    void copyAll();
    void toggleShowInternalCommands();

private:
    GDBOutputWidget* parent_;
};

}

#endif

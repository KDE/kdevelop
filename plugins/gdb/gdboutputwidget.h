/*
    SPDX-FileCopyrightText: 2003 John Birch <jbb@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _GDBOUTPUTWIDGET_H_
#define _GDBOUTPUTWIDGET_H_

#include "dbgglobal.h"

#include <QColor>
#include <QStringList>
#include <QTimer>
#include <QWidget>

namespace KDevelop {
class IDebugSession;
}

class KHistoryComboBox;

class QMenu;
class QPlainTextEdit;
class QPoint;
class QToolButton;

namespace KDevMI
{
namespace GDB
{

class GDBController;
class CppDebuggerPlugin;

class GDBOutputWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GDBOutputWidget(CppDebuggerPlugin* plugin, QWidget *parent=nullptr );
    ~GDBOutputWidget() override;

    void savePartialProjectSession();
    void restorePartialProjectSession();

public Q_SLOTS:
    void clear();

    void slotInternalCommandStdout(const QString& line);
    void slotUserCommandStdout(const QString& line);
    void slotReceivedStderr(const char* line);

    void slotGDBCmd();

    void flushPending();

    void copyAll();
    void toggleShowInternalCommands();

private Q_SLOTS:
    void updateColors();

protected:
    void focusInEvent(QFocusEvent *e) override;
    void contextMenuEvent(QContextMenuEvent* e) override;

Q_SIGNALS:
    void userGDBCmd(const QString &cmd);
    void breakInto();

private:
    void currentSessionChanged(KDevelop::IDebugSession* iSession, KDevelop::IDebugSession* iPreviousSession);
    /**
     * Call this function when the debugger state of the current session changes to update the UI accordingly.
     */
    void debuggerStateChanged(DBGStateFlags newState);
    /**
     * Call this function when the debugger is not running to update the UI accordingly.
     */
    void handleDebuggerNotRunning();

    void newStdoutLine(const QString& line, bool internal);

    /** Arranges for 'line' to be shown to the user.
        Adds 'line' to pendingOutput_ and makes sure
        updateTimer_ is running. */
    void showLine(const QString& line);

    /** Makes 'l' no longer than 'max_size' by
        removing excessive elements from the top.
    */
    void trimList(QStringList& l, int max_size);

    void gdbViewContextMenuRequested(const QPoint& viewportPosition);
    void addActionsAndShowContextMenu(QMenu* menu, const QPoint& globalPosition);

    GDBController* m_controller;
    KHistoryComboBox*  m_userGDBCmdEditor;
    QToolButton*    m_Interrupt;
    QPlainTextEdit*      m_gdbView;

    bool m_cmdEditorHadFocus;

    void setShowInternalCommands(bool);

    /** The output from user commands only and from
        all commands. We keep it here so that if we switch
        "Show internal commands" on, we can show previous 
        internal commands. 
    */
    QStringList m_userCommands_, m_allCommands;

    /** For performance reasons, we don't immediately add new text
        to QTExtEdit. Instead we add it to pendingOutput_ and 
        flush it on timer.
    */
    QString m_pendingOutput;
    QTimer m_updateTimer;

    bool m_showInternalCommands;

    int m_maxLines;

    QColor m_gdbColor;
    QColor m_errorColor;
};

} // end of namespace GDB
} // end of namespace KDevMI

#endif

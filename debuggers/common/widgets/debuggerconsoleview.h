/*
 * Debugger Console View
 *
 * Copyright 2003 John Birch <jbb@kdevelop.org>
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2016  Aetf <aetf@unlimitedcodeworks.xyz>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef DEBUGGERCONSOLEVIEW_H
#define DEBUGGERCONSOLEVIEW_H

#include <QWidget>
#include <QPoint>
#include <QStringList>
#include <QTimer>

#include "dbgglobal.h"

class QMenu;
class QTextEdit;
class QToolBar;
class KHistoryComboBox;

namespace KDevelop {
class IDebugSession;
}

namespace KDevMI {
class MIDebuggerPlugin;

/**
 * @brief A debugger console gives the user direct access to the debugger command line interface.
 */
class DebuggerConsoleView : public QWidget
{
    Q_OBJECT
public:
    DebuggerConsoleView(MIDebuggerPlugin *plugin, QWidget *parent = nullptr);
    ~DebuggerConsoleView();

    /**
     * Whether show a button allowing user to interrput debugger execution.
     */
    void setShowInterrupt(bool enable);

    void setShowInternalCommands(bool enable);

Q_SIGNALS:
    void requestRaise();
    /**
     * Proxy signals for DebugSession
     */
    void interruptDebugger();
    void sendCommand(const QString &cmd);


protected:
    void setupUi();
    void setupToolBar();

    /**
     * Arranges for 'line' to be shown to the user.
     * Adds 'line' to m_pendingOutput and makes sure
     * m_updateTimer is running.
     */
    void appendLine(const QString &line);
    void updateColors();

    /**
     * escape html meta characters and handle line break
     */
    QString toHtmlEscaped(QString text);

    QString colorify(QString text, const QColor &color);

    /**
     * Makes 'l' no longer than 'max_size' by
     * removing excessive elements from the top.
     */
    void trimList(QStringList& l, int max_size);

    void changeEvent(QEvent *e) override;
    void focusInEvent(QFocusEvent *e) override;

protected Q_SLOTS:
    void showContextMenu(const QPoint &pos);
    void toggleRepeat(bool checked);
    void toggleShowInternalCommands(bool checked);
    void flushPending();
    void clear();

    void handleSessionChanged(KDevelop::IDebugSession *session);
    void handleDebuggerStateChange(DBGStateFlags oldStatus, DBGStateFlags newStatus);
    void receivedInternalCommandStdout(const QString &line);
    void receivedUserCommandStdout(const QString &line);
    void receivedStdout(const QString &line, bool internal);
    void receivedStderr(const QString &line);

    void trySendCommand(QString cmd);
private:
    QAction *m_actRepeat;
    QAction *m_actInterrupt;
    QAction *m_actShowInternal;
    QAction *m_actCmdEditor;

    QTextEdit *m_textView;
    QMenu *m_contextMenu;
    QToolBar *m_toolBar;
    KHistoryComboBox *m_cmdEditor;

    bool m_repeatLastCommand;
    bool m_showInternalCommands;
    bool m_cmdEditorHadFocus;

    /**
     * The output from user commands only and from all
     * commands. We keep it here so that if we switch
     * "Show internal commands" on, we can show previous
     * internal commands.
     */
    QStringList m_allOutput;
    QStringList m_userOutput;

    /**
     * For performance reasons, we don't immediately add new text
     * to QTExtEdit. Instead we add it to m_pendingOutput and
     * flush it on timer.
     */
    QString m_pendingOutput;
    QTimer m_updateTimer;

    QColor m_stdColor;
    QColor m_errorColor;

    int m_maxLines;
};

} // end of namespace KDevMI

#endif // DEBUGGERCONSOLEVIEW_H

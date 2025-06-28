/*
    SPDX-FileCopyrightText: 2003 John Birch <jbb@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef DEBUGGERCONSOLEVIEW_H
#define DEBUGGERCONSOLEVIEW_H

#include <QWidget>
#include <QStringList>
#include <QTimer>

#include "dbgglobal.h"

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
    explicit DebuggerConsoleView(MIDebuggerPlugin *plugin, QWidget *parent = nullptr);
    ~DebuggerConsoleView() override;

    /**
     * Whether show a button allowing user to interrupt debugger execution.
     */
    void setShowInterrupt(bool enable);

    /**
     * If set to a nonempty string, the default "(gdb)" prompt will be replaced.
     * This only affects output lines after the call.
     */
    void setReplacePrompt(const QString &prompt);

    void setShowInternalCommands(bool enable);

Q_SIGNALS:
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
    void textViewContextMenuRequested(const QPoint& viewportPosition);

    QAction *m_actRepeat;
    QAction *m_actInterrupt;
    QAction *m_actShowInternal;
    QAction *m_actCmdEditor;

    QTextEdit *m_textView;
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

    QString m_alterPrompt;
};

} // end of namespace KDevMI

#endif // DEBUGGERCONSOLEVIEW_H

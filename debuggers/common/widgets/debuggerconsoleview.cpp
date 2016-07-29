/*
 * Debugger Console View
 *
 * Copyright 2003 John Birch <jbb@kdevelop.org>
 * Copyright 2006 Vladimir Prus <ghost@cs.msu.su>
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

#include "debuggerconsoleview.h"

#include "debuglog.h"
#include "midebuggerplugin.h"
#include "midebugsession.h"

#include <interfaces/icore.h>
#include <interfaces/idebugcontroller.h>

#include <KColorScheme>
#include <KHistoryComboBox>
#include <KLocalizedString>

#include <QAction>
#include <QClipboard>
#include <QEvent>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMenu>
#include <QScopedPointer>
#include <QScrollBar>
#include <QStyle>
#include <QTextEdit>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>

using namespace KDevMI;

DebuggerConsoleView::DebuggerConsoleView(MIDebuggerPlugin *plugin, QWidget *parent)
    : QWidget(parent)
    , m_repeatLastCommand(false)
    , m_showInternalCommands(false)
    , m_cmdEditorHadFocus(false)
    , m_maxLines(5000)
{
    setWindowIcon(QIcon::fromTheme("dialog-scripts"));
    setWindowTitle(i18n("Debugger Console"));
    setWhatsThis(i18n("<b>Debugger Console</b><p>"
                      "Shows all debugger commands being executed. "
                      "You can also issue any other debugger command while debugging.</p>"));

    setupUi();

    m_actRepeat = new QAction(QIcon::fromTheme("edit-redo"),
                              i18n("Repeat last command when hit Return"),
                              this);
    m_actRepeat->setCheckable(true);
    m_actRepeat->setChecked(m_repeatLastCommand);
    connect(m_actRepeat, &QAction::toggled, this, &DebuggerConsoleView::toggleRepeat);
    m_toolBar->insertAction(m_actCmdEditor, m_actRepeat);

    m_actInterrupt = new QAction(QIcon::fromTheme("media-playback-pause"),
                                 i18n("Pause execution of the app to enter gdb commands"),
                                 this);
    connect(m_actInterrupt, &QAction::triggered, this, &DebuggerConsoleView::interruptDebugger);
    m_toolBar->insertAction(m_actCmdEditor, m_actInterrupt);
    setShowInterrupt(true);

    m_actShowInternal = new QAction(i18n("Show Internal Commands"));
    m_actShowInternal->setCheckable(true);
    m_actShowInternal->setChecked(m_showInternalCommands);
    m_actShowInternal->setWhatsThis(i18n(
        "Controls if commands issued internally by KDevelop "
        "will be shown or not.<br>"
        "This option will affect only future commands, it will not "
        "add or remove already issued commands from the view."));
    connect(m_actShowInternal, &QAction::toggled,
            this, &DebuggerConsoleView::toggleShowInternalCommands);

    handleDebuggerStateChange(s_none, s_dbgNotStarted);

    m_updateTimer.setSingleShot(true);
    connect(&m_updateTimer, &QTimer::timeout, this, &DebuggerConsoleView::flushPending);

    connect(plugin->core()->debugController(), &KDevelop::IDebugController::currentSessionChanged,
            this, &DebuggerConsoleView::handleSessionChanged);

    connect(plugin, &MIDebuggerPlugin::reset, this, &DebuggerConsoleView::clear);
    connect(plugin, &MIDebuggerPlugin::raiseDebuggerConsoleViews,
            this, &DebuggerConsoleView::requestRaise);

    handleSessionChanged(plugin->core()->debugController()->currentSession());

    updateColors();
}

void DebuggerConsoleView::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::PaletteChange) {
        updateColors();
    }
}

void DebuggerConsoleView::updateColors()
{
    KColorScheme scheme(QPalette::Active);
    m_stdColor = scheme.foreground(KColorScheme::LinkText).color();
    m_errorColor = scheme.foreground(KColorScheme::NegativeText).color();
}

void DebuggerConsoleView::setupUi()
{
    setupToolBar();

    m_textView = new QTextEdit;
    m_textView->setReadOnly(true);
    m_textView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_textView, &QTextEdit::customContextMenuRequested,
            this, &DebuggerConsoleView::showContextMenu);

    auto vbox = new QVBoxLayout;
    vbox->addWidget(m_textView);
    vbox->addWidget(m_toolBar);

    setLayout(vbox);

    m_cmdEditor = new KHistoryComboBox(this);
    m_cmdEditor->setDuplicatesEnabled(false);
    connect(m_cmdEditor,
            static_cast<void(KHistoryComboBox::*)(const QString&)>(&KHistoryComboBox::returnPressed),
            this, &DebuggerConsoleView::trySendCommand);

    auto label = new QLabel(i18n("&Command:"), this);
    label->setBuddy(m_cmdEditor);

    auto hbox = new QHBoxLayout;
    hbox->addWidget(label);
    hbox->addWidget(m_cmdEditor);
    hbox->setStretchFactor(m_cmdEditor, 1);
    hbox->setContentsMargins(0, 0, 0, 0);

    auto cmdEditor = new QWidget(this);
    cmdEditor->setLayout(hbox);
    m_actCmdEditor = m_toolBar->addWidget(cmdEditor);
}

void DebuggerConsoleView::setupToolBar()
{
    m_toolBar = new QToolBar(this);
    int iconSize = style()->pixelMetric(QStyle::PM_SmallIconSize);
    m_toolBar->setIconSize(QSize(iconSize, iconSize));
    m_toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_toolBar->setFloatable(false);
    m_toolBar->setMovable(false);
    m_toolBar->setWindowTitle(i18n("%1 Command Bar", windowTitle()));
    m_toolBar->setContextMenuPolicy(Qt::PreventContextMenu);

    // remove margins, to make command editor nicely aligned with the output
    m_toolBar->layout()->setContentsMargins(0, 0, 0, 0);
}

void DebuggerConsoleView::focusInEvent(QFocusEvent*)
{
    m_textView->verticalScrollBar()->setValue(m_textView->verticalScrollBar()->maximum());
    m_cmdEditor->setFocus();
}

DebuggerConsoleView::~DebuggerConsoleView()
{
}

void DebuggerConsoleView::setShowInterrupt(bool enable)
{
    m_actInterrupt->setVisible(enable);
}

void DebuggerConsoleView::setShowInternalCommands(bool enable)
{
    if (enable != m_showInternalCommands)
    {
        m_showInternalCommands = enable;

        // Set of strings to show changes, text edit still has old
        // set. Refresh.
        m_textView->clear();
        QStringList& newList = m_showInternalCommands ? m_allOutput : m_userOutput;

        for (const auto &line : newList) {
            // Note that color formatting is already applied to 'line'.
            appendLine(line);
        }
    }
}

void DebuggerConsoleView::showContextMenu(const QPoint &pos)
{
    QScopedPointer<QMenu> popup(m_textView->createStandardContextMenu(pos));

    popup->addSeparator();
    popup->addAction(m_actShowInternal);

    popup->exec(m_textView->mapToGlobal(pos));
}

void DebuggerConsoleView::toggleRepeat(bool checked)
{
    m_repeatLastCommand = checked;
}

void DebuggerConsoleView::toggleShowInternalCommands(bool checked)
{
    setShowInternalCommands(checked);
}

void DebuggerConsoleView::appendLine(const QString& line)
{
    m_pendingOutput += line;

    // To improve performance, we update the view after some delay.
    if (!m_updateTimer.isActive())
    {
        m_updateTimer.start(100);
    }
}

void DebuggerConsoleView::flushPending()
{
    m_textView->setUpdatesEnabled(false);

    QTextDocument *document = m_textView->document();
    QTextCursor cursor(document);
    cursor.movePosition(QTextCursor::End);
    cursor.insertHtml(m_pendingOutput);
    m_pendingOutput.clear();

    m_textView->verticalScrollBar()->setValue(m_textView->verticalScrollBar()->maximum());
    m_textView->setUpdatesEnabled(true);
    m_textView->update();
    if (m_cmdEditorHadFocus) {
        m_cmdEditor->setFocus();
    }
}

void DebuggerConsoleView::clear()
{
    if (m_textView)
        m_textView->clear();

    if (m_cmdEditor)
        m_cmdEditor->clear();

    m_userOutput.clear();
    m_allOutput.clear();
}

void DebuggerConsoleView::handleDebuggerStateChange(DBGStateFlags, DBGStateFlags newStatus)
{
    if (newStatus & s_dbgNotStarted) {
        m_actInterrupt->setEnabled(false);
        m_cmdEditor->setEnabled(false);
        return;
    } else {
        m_actInterrupt->setEnabled(true);
    }

    if (newStatus & s_dbgBusy) {
        if (m_cmdEditor->isEnabled()) {
            m_cmdEditorHadFocus = m_cmdEditor->hasFocus();
        }
        m_cmdEditor->setEnabled(false);
    } else {
        m_cmdEditor->setEnabled(true);
    }
}

QString DebuggerConsoleView::toHtmlEscaped(QString text)
{
    text = text.toHtmlEscaped();

    text.replace('\n', "<br>");
    return text;
}


QString DebuggerConsoleView::colorify(QString text, const QColor& color)
{
    text = "<font color=\"" + color.name() +  "\">" + text + "</font>";
    return text;
}

void DebuggerConsoleView::receivedInternalCommandStdout(const QString& line)
{
    receivedStdout(line, true);
}

void DebuggerConsoleView::receivedUserCommandStdout(const QString& line)
{
    receivedStdout(line, false);
}

void DebuggerConsoleView::receivedStdout(const QString& line, bool internal)
{
    QString colored = toHtmlEscaped(line);
    if (colored.startsWith("(gdb)")) {
        colored = colorify(colored, m_stdColor);
    }

    m_allOutput.append(colored);
    trimList(m_allOutput, m_maxLines);

    if (!internal) {
        m_userOutput.append(colored);
        trimList(m_userOutput, m_maxLines);
    }

    if (!internal || m_showInternalCommands)
        appendLine(colored);
}

void DebuggerConsoleView::receivedStderr(const QString& line)
{
    QString colored = toHtmlEscaped(line);
    colored = colorify(colored, m_errorColor);

    // Errors are shown inside user commands too.
    m_allOutput.append(colored);
    trimList(m_allOutput, m_maxLines);

    m_userOutput.append(colored);
    trimList(m_userOutput, m_maxLines);

    appendLine(colored);
}

void DebuggerConsoleView::trimList(QStringList& l, int max_size)
{
    int length = l.count();
    if (length > max_size)
    {
        for(int to_delete = length - max_size; to_delete; --to_delete)
        {
            l.erase(l.begin());
        }
    }
}

void DebuggerConsoleView::trySendCommand(QString cmd)
{
    if (m_repeatLastCommand && cmd.isEmpty()) {
        cmd = m_cmdEditor->historyItems().last();
    }
    if (!cmd.isEmpty())
    {
        m_cmdEditor->addToHistory(cmd);
        m_cmdEditor->clearEditText();

        emit sendCommand(cmd);
    }
}

void DebuggerConsoleView::handleSessionChanged(KDevelop::IDebugSession* s)
{
    MIDebugSession *session = qobject_cast<MIDebugSession*>(s);
    if (!session) return;

    connect(this, &DebuggerConsoleView::sendCommand,
             session, &MIDebugSession::addUserCommand);
    connect(this, &DebuggerConsoleView::interruptDebugger,
             session, &MIDebugSession::interruptDebugger);

     connect(session, &MIDebugSession::debuggerInternalCommandOutput,
             this, &DebuggerConsoleView::receivedInternalCommandStdout);
     connect(session, &MIDebugSession::debuggerUserCommandOutput,
             this, &DebuggerConsoleView::receivedUserCommandStdout);
     connect(session, &MIDebugSession::debuggerInternalOutput,
             this, &DebuggerConsoleView::receivedStderr);

     connect(session, &MIDebugSession::debuggerStateChanged,
             this, &DebuggerConsoleView::handleDebuggerStateChange);

     handleDebuggerStateChange(s_none, session->debuggerState());
}

/*
    SPDX-FileCopyrightText: 2003 John Birch <jbb@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Vladimir Prus <ghost@cs.msu.su>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "gdboutputwidget.h"

#include "dbgglobal.h"
#include "debuggerplugin.h"
#include "debuglog.h"
#include "debugsession.h"

#include <interfaces/icore.h>
#include <interfaces/idebugcontroller.h>
#include <interfaces/iuicontroller.h>
#include <util/shellutils.h>

#include <KColorScheme>
#include <KConfigGroup>
#include <KHistoryComboBox>
#include <KLocalizedString>

#include <QApplication>
#include <QClipboard>
#include <QFocusEvent>
#include <QMenu>
#include <QLabel>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QToolButton>
#include <QToolTip>
#include <QScrollBar>

using namespace KDevMI::GDB;

/***************************************************************************/

GDBOutputWidget::GDBOutputWidget(CppDebuggerPlugin* plugin, QWidget *parent) :
    QWidget(parent),
    m_userGDBCmdEditor(nullptr),
    m_Interrupt(nullptr),
    m_gdbView(nullptr),
    m_showInternalCommands(false),
    m_maxLines(5000)
{
    setWindowIcon(QIcon::fromTheme(QStringLiteral("dialog-scripts"), windowIcon()));
    setWindowTitle(i18nc("@title:window", "GDB Output"));
    setWhatsThis(i18nc("@info:whatsthis", "<b>GDB output</b><p>"
                    "Shows all GDB commands being executed. "
                    "You can also issue any other GDB command while debugging.</p>"));

    m_gdbView = new QPlainTextEdit(this);
    m_gdbView->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    m_gdbView->setReadOnly(true);

    m_gdbView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_gdbView, &QWidget::customContextMenuRequested, this, &GDBOutputWidget::gdbViewContextMenuRequested);

    m_userGDBCmdEditor = new KHistoryComboBox (this);

    auto* label = new QLabel(i18nc("@label:listbox", "&GDB command:"), this);
    label->setBuddy(m_userGDBCmdEditor);

    m_Interrupt = new QToolButton( this );
    m_Interrupt->setIcon ( QIcon::fromTheme( QStringLiteral("media-playback-pause") ) );
    m_Interrupt->setToolTip(i18nc("@info:tooltip", "Pause execution of the app to enter GDB commands"));

    auto *topLayout = new QVBoxLayout(this);
    topLayout->addWidget(m_gdbView);
    topLayout->setStretchFactor(m_gdbView, 1);
    topLayout->setContentsMargins(0, 0, 0, 0);

    QBoxLayout *userGDBCmdEntry = new QHBoxLayout();
    userGDBCmdEntry->addWidget(label);
    userGDBCmdEntry->addWidget(m_userGDBCmdEditor);
    userGDBCmdEntry->setStretchFactor(m_userGDBCmdEditor, 1);
    userGDBCmdEntry->addWidget(m_Interrupt);
    topLayout->addLayout(userGDBCmdEntry);

    setLayout(topLayout);

    connect(m_userGDBCmdEditor, QOverload<const QString&>::of(&KHistoryComboBox::returnPressed),
            this, &GDBOutputWidget::slotGDBCmd);
    connect(m_Interrupt, &QToolButton::clicked, this, &GDBOutputWidget::breakInto);

    m_updateTimer.setSingleShot(true);
    m_updateTimer.setInterval(100);
    connect(&m_updateTimer, &QTimer::timeout,
             this, &GDBOutputWidget::flushPending);

    connect(KDevelop::ICore::self()->debugController(), &KDevelop::IDebugController::currentSessionChanged,
            this, &GDBOutputWidget::currentSessionChanged);

    if (plugin) {
        connect(plugin, &CppDebuggerPlugin::reset, this, &GDBOutputWidget::clear);
        connect(plugin, &CppDebuggerPlugin::raiseDebuggerConsoleViews, this, [plugin, this] {
            plugin->core()->uiController()->raiseToolView(this);
        });
    }

    currentSessionChanged(KDevelop::ICore::self()->debugController()->currentSession(), nullptr);

//     TODO Port to KF5
//     connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()),
//             this, SLOT(updateColors()));
    updateColors();

}

void GDBOutputWidget::updateColors()
{
    KColorScheme scheme(QPalette::Active);
    m_gdbColor = scheme.foreground(KColorScheme::LinkText).color();
    m_stderrOrLogColor = scheme.foreground(KColorScheme::NeutralText).color();
}

void GDBOutputWidget::currentSessionChanged(KDevelop::IDebugSession* iSession,
                                            KDevelop::IDebugSession* iPreviousSession)
{
    if (auto* const previousSession = qobject_cast<DebugSession*>(iPreviousSession)) {
        disconnect(this, nullptr, previousSession, nullptr);
        disconnect(previousSession, nullptr, this, nullptr);
    }

    auto* const session = qobject_cast<DebugSession*>(iSession);
    if (!session) {
        handleDebuggerNotRunning();
        return;
    }

    connect(this, &GDBOutputWidget::userGDBCmd, session, &DebugSession::addUserCommand);
    connect(this, &GDBOutputWidget::breakInto, session, &DebugSession::interruptDebugger);

    connect(session, &DebugSession::debuggerInternalCommandOutput, this, &GDBOutputWidget::slotInternalCommandStdout);
    connect(session, &DebugSession::debuggerUserCommandOutput, this, &GDBOutputWidget::slotUserCommandStdout);
    connect(session, &DebugSession::debuggerInternalOutput, this, &GDBOutputWidget::receivedStderrOrLog);

    connect(session, &DebugSession::debuggerStateChanged, this, [this](DBGStateFlags oldState, DBGStateFlags newState) {
        Q_UNUSED(oldState);
        debuggerStateChanged(newState);
    });

    debuggerStateChanged(session->debuggerState());
}


/***************************************************************************/

GDBOutputWidget::~GDBOutputWidget()
{
    delete m_gdbView;
    delete m_userGDBCmdEditor;
}

/***************************************************************************/

void GDBOutputWidget::clear()
{
    if (m_gdbView)
        m_gdbView->clear();

    m_userCommands_.clear();
    m_allCommands.clear();

    m_pendingOutput.clear();
    m_updateTimer.stop();
}

/***************************************************************************/

void GDBOutputWidget::slotInternalCommandStdout(const QString& line)
{
    newStdoutLine(line, true);
}

void GDBOutputWidget::slotUserCommandStdout(const QString& line)
{
    qCDebug(DEBUGGERGDB) << "User command stdout: " << line;
    newStdoutLine(line, false);
}

namespace {
    void colorify(QString& text, const QColor& color)
    {
        if (text.endsWith(QLatin1Char('\n'))) {
            text.chop(1);
        }
        text = QLatin1String("<font color=\"") + color.name() + QLatin1String("\">") + text + QLatin1String("</font><br>");
    }
}

void GDBOutputWidget::receivedStderrOrLog(const QString& line)
{
    // Multiple regular messages like the following arrive through the GDB 'log' MI channel:
    // * handle SIG32 pass nostop noprint
    // * python sys.path.insert(0, "/path/to/installed-kdevelop/share/kdevgdb/printers")
    // * Quit
    // The following complaint about a breakpoint location that looks like
    // an error or a warning also arrives through the GDB 'log' MI channel:
    // * Function "nonexistentFunction" not defined.
    // So color the diverse log messages as warnings rather than as errors
    // and do not show them among user commands to avoid clutter.

    auto colored = line.toHtmlEscaped();
    colorify(colored, m_stderrOrLogColor);
    addFormattedLine(colored, true);
}

void GDBOutputWidget::newStdoutLine(const QString& line,
                                    bool internal)
{
    QString s = line.toHtmlEscaped();
    if (s.startsWith(QLatin1String("(gdb)")))
    {
        colorify(s, m_gdbColor);
    }
    else
        s.replace(QLatin1Char('\n'), QLatin1String("<br>"));

    addFormattedLine(s, internal);
}

void GDBOutputWidget::addFormattedLine(const QString& line, bool internal)
{
    m_allCommands.append(line);
    trimList(m_allCommands, m_maxLines);
    if (!internal)
    {
        m_userCommands_.append(line);
        trimList(m_userCommands_, m_maxLines);
    }

    if (!internal || m_showInternalCommands)
        showLine(line);
}

void GDBOutputWidget::showLine(const QString& line)
{
    m_pendingOutput += line;

    // To improve performance, we update the view after some delay.
    if (!m_updateTimer.isActive())
    {
        m_updateTimer.start();
    }
}

void GDBOutputWidget::trimList(QStringList& l, int max_size)
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

void GDBOutputWidget::setShowInternalCommands(bool show)
{
    if (show != m_showInternalCommands)
    {
        m_showInternalCommands = show;

        // Set of strings to show changes, text edit still has old
        // set. Refresh.
        m_gdbView->clear();
        const QStringList& newList =
            m_showInternalCommands ? m_allCommands : m_userCommands_;

        for (auto& line : newList) {
            // Note that color formatting is already applied to 'line'.
            showLine(line);
        }
    }
}

void GDBOutputWidget::slotGDBCmd()
{
    QString GDBCmd(m_userGDBCmdEditor->currentText());
    if (!GDBCmd.isEmpty())
    {
        m_userGDBCmdEditor->addToHistory(GDBCmd);
        m_userGDBCmdEditor->clearEditText();
        emit userGDBCmd(GDBCmd);
    }
}

void GDBOutputWidget::flushPending()
{
    m_gdbView->setUpdatesEnabled(false);

    // QTextEdit adds newline after paragraph automatically.
    // So, remove trailing newline to avoid double newlines.
    if (m_pendingOutput.endsWith(QLatin1Char('\n')))
        m_pendingOutput.chop(1);
    Q_ASSERT(!m_pendingOutput.endsWith(QLatin1Char('\n')));

    QTextDocument *document = m_gdbView->document();
    QTextCursor cursor(document);
    cursor.movePosition(QTextCursor::End);
    cursor.insertHtml(m_pendingOutput);
    m_pendingOutput.clear();

    m_gdbView->verticalScrollBar()->setValue(m_gdbView->verticalScrollBar()->maximum());
    m_gdbView->setUpdatesEnabled(true);
    m_gdbView->update();
    if (m_cmdEditorHadFocus) {
        m_userGDBCmdEditor->setFocus();
    }
}

void GDBOutputWidget::debuggerStateChanged(DBGStateFlags newState)
{
    if (newState & s_dbgNotStarted) {
        handleDebuggerNotRunning();
        return;
    }
    else
    {
        m_Interrupt->setEnabled(true);
    }

    if (newState & s_dbgBusy) {
        if (m_userGDBCmdEditor->isEnabled()) {
            m_cmdEditorHadFocus = m_userGDBCmdEditor->hasFocus();
        }
        m_userGDBCmdEditor->setEnabled(false);
    }
    else
    {
        m_userGDBCmdEditor->setEnabled(true);
    }
}

void GDBOutputWidget::handleDebuggerNotRunning()
{
    m_Interrupt->setEnabled(false);
    m_userGDBCmdEditor->setEnabled(false);
}

void GDBOutputWidget::focusInEvent(QFocusEvent* /*e*/)
{
    m_gdbView->verticalScrollBar()->setValue(m_gdbView->verticalScrollBar()->maximum());
    m_userGDBCmdEditor->setFocus();
}

void GDBOutputWidget::savePartialProjectSession()
{
    KConfigGroup config(KSharedConfig::openConfig(), QStringLiteral("GDB Debugger"));

    config.writeEntry("showInternalCommands", m_showInternalCommands);
}

void GDBOutputWidget::restorePartialProjectSession()
{
    KConfigGroup config(KSharedConfig::openConfig(), QStringLiteral("GDB Debugger"));

    m_showInternalCommands = config.readEntry("showInternalCommands", false);
}

void GDBOutputWidget::gdbViewContextMenuRequested(const QPoint& viewportPosition)
{
    // Even though the documentation for QPlainTextEdit::createStandardContextMenu() asks
    // for a position in the document coordinates, only passing a position in the viewport
    // coordinates works correctly in practice. This bug is reported in QTBUG-138099.
    auto* menu = m_gdbView->createStandardContextMenu(viewportPosition);
    KDevelop::prepareStandardContextMenuToAddingCustomActions(menu, m_gdbView);
    addActionsAndShowContextMenu(menu, m_gdbView->viewport()->mapToGlobal(viewportPosition));
}

void GDBOutputWidget::contextMenuEvent(QContextMenuEvent * e)
{
    addActionsAndShowContextMenu(new QMenu(this), e->globalPos());
}

void GDBOutputWidget::addActionsAndShowContextMenu(QMenu* menu, const QPoint& globalPosition)
{
    auto* const action = menu->addAction(i18nc("@action:inmenu", "Show Internal Commands"), this,
                                         &GDBOutputWidget::toggleShowInternalCommands);

    action->setCheckable(true);
    action->setChecked(m_showInternalCommands);
    action->setWhatsThis(i18nc("@info:tooltip",
            "Controls if commands issued internally by KDevelop "
            "will be shown or not.<br>"
            "This option will affect only future commands, it will not "
            "add or remove already issued commands from the view."));

    menu->addAction(i18nc("@action:inmenu", "Copy All"), this, &GDBOutputWidget::copyAll);

    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->popup(globalPosition);
}

void GDBOutputWidget::copyAll()
{
    const auto text = m_gdbView->toPlainText();
    // Make sure the text is pastable both with Ctrl-C and with
    // middle click.
    QApplication::clipboard()->setText(text, QClipboard::Clipboard);
    QApplication::clipboard()->setText(text, QClipboard::Selection);
}

void GDBOutputWidget::toggleShowInternalCommands()
{
    setShowInternalCommands(!m_showInternalCommands);
}

#include "moc_gdboutputwidget.cpp"

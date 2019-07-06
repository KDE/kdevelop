/* This file is part of KDevelop
 *
 * Copyright 1999-2001 John Birch <jbb@kdevelop.org>
 * Copyright 2001 by Bernd Gehrmann <bernd@kdevelop.org>
 * Copyright 2006 Vladimir Prus <ghost@cs.msu.su>
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2009 Niko Sams <niko.sams@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "debugcontroller.h"

#include <QAction>

#include <KActionCollection>
#include <KLocalizedString>
#include <KTextEditor/Document>
#include <KTextEditor/MarkInterface>
#include <ktexteditor_version.h>
#include <KXMLGUIFactory>

#include "../interfaces/idocument.h"
#include "../interfaces/icore.h"
#include "../interfaces/idocumentcontroller.h"
#include "../interfaces/ipartcontroller.h"
#include "../interfaces/contextmenuextension.h"
#include "../interfaces/context.h"
#include "../language/interfaces/editorcontext.h"
#include "../sublime/view.h"
#include "../sublime/mainwindow.h"
#include "../sublime/area.h"
#include "../debugger/breakpoint/breakpointmodel.h"
#include "../debugger/breakpoint/breakpointwidget.h"
#include "../debugger/variable/variablewidget.h"
#include "../debugger/framestack/framestackmodel.h"
#include "../debugger/framestack/framestackwidget.h"
#include "core.h"
#include "debug.h"
#include "uicontroller.h"
#include "iruncontroller.h"

namespace KDevelop {

template<class T>
class DebuggerToolFactory : public KDevelop::IToolViewFactory
{
public:
  DebuggerToolFactory(DebugController* controller, const QString &id, Qt::DockWidgetArea defaultArea)
  : m_controller(controller), m_id(id), m_defaultArea(defaultArea) {}

  QWidget* create(QWidget *parent = nullptr) override
  {
    return new T(m_controller, parent);
  }

  QString id() const override
  {
    return m_id;
  }

  Qt::DockWidgetArea defaultPosition() override
  {
    return m_defaultArea;
  }

  void viewCreated(Sublime::View* view) override
  {
      if (view->widget()->metaObject()->indexOfSignal("requestRaise()") != -1)
          QObject::connect(view->widget(), SIGNAL(requestRaise()), view, SLOT(requestRaise()));
  }

  /* At present, some debugger widgets (e.g. breakpoint) contain actions so that shortcuts
     work, but they don't need any toolbar.  So, suppress toolbar action.  */
  QList<QAction*> toolBarActions( QWidget* viewWidget ) const override
  {
      Q_UNUSED(viewWidget);
      return QList<QAction*>();
  }

private:
    DebugController* const m_controller;
    const QString m_id;
    const Qt::DockWidgetArea m_defaultArea;
};

DebugController::DebugController(QObject *parent)
    : IDebugController(parent), KXMLGUIClient(),
      m_breakpointModel(new BreakpointModel(this)),
      m_variableCollection(new VariableCollection(this))
{
    setComponentName(QStringLiteral("kdevdebugger"), i18n("Debugger"));
    setXMLFile(QStringLiteral("kdevdebuggershellui.rc"));
}

void DebugController::initialize()
{
    m_breakpointModel->load();
}

void DebugController::initializeUi()
{
    if (m_uiInitialized) return;
    m_uiInitialized = true;

    if((Core::self()->setupFlags() & Core::NoUi)) return;
    setupActions();

    ICore::self()->uiController()->addToolView(
        i18n("Frame Stack"),
        new DebuggerToolFactory<FramestackWidget>(
            this, QStringLiteral("org.kdevelop.debugger.StackView"),
            Qt::BottomDockWidgetArea));

    ICore::self()->uiController()->addToolView(
        i18n("Breakpoints"),
        new DebuggerToolFactory<BreakpointWidget>(
            this, QStringLiteral("org.kdevelop.debugger.BreakpointsView"),
            Qt::BottomDockWidgetArea));

    ICore::self()->uiController()->addToolView(
        i18n("Variables"),
        new DebuggerToolFactory<VariableWidget>(
            this, QStringLiteral("org.kdevelop.debugger.VariablesView"),
            Qt::LeftDockWidgetArea));

    const auto parts = KDevelop::ICore::self()->partController()->parts();
    for (KParts::Part* p : parts) {
        partAdded(p);
    }
    connect(KDevelop::ICore::self()->partController(),
            &IPartController::partAdded,
            this,
            &DebugController::partAdded);


    ICore::self()->uiController()->activeMainWindow()->guiFactory()->addClient(this);

    stateChanged(QStringLiteral("ended"));
}


void DebugController::cleanup()
{
    if (m_currentSession) m_currentSession.data()->stopDebugger();
}

DebugController::~DebugController()
{
}

BreakpointModel* DebugController::breakpointModel()
{
    return m_breakpointModel;
}

VariableCollection* DebugController::variableCollection()
{
    return m_variableCollection;
}

void DebugController::partAdded(KParts::Part* part)
{
    if (auto* doc = qobject_cast<KTextEditor::Document*>(part)) {
        auto* iface = qobject_cast<KTextEditor::MarkInterface*>(doc);
        if( !iface )
            return;

        iface->setMarkPixmap(KTextEditor::MarkInterface::Execution, *executionPointPixmap());
    }
}

IDebugSession* DebugController::currentSession()
{
    return m_currentSession.data();
}

void DebugController::setupActions()
{
    KActionCollection* ac = actionCollection();

    QAction* action = m_continueDebugger = new QAction(this);
    setContinueStartsDebug(true);
    ac->addAction(QStringLiteral("debug_continue"), action);
    connect(action, &QAction::triggered, this, &DebugController::run);

    #if 0
    m_restartDebugger = action = new QAction(QIcon::fromTheme("media-seek-backward"), i18n("&Restart"), this);
    action->setToolTip( i18n("Restart program") );
    action->setWhatsThis( i18n("Restarts applications from the beginning.") );
    action->setEnabled(false);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(restartDebugger()));
    ac->addAction("debug_restart", action);
    #endif

    m_interruptDebugger = action = new QAction(QIcon::fromTheme(QStringLiteral("media-playback-pause")), i18n("Interrupt"), this);
    action->setToolTip( i18n("Interrupt application") );
    action->setWhatsThis(i18n("Interrupts the debugged process or current debugger command."));
    connect(action, &QAction::triggered, this, &DebugController::interruptDebugger);
    ac->addAction(QStringLiteral("debug_pause"), action);

    m_runToCursor = action = new QAction(QIcon::fromTheme(QStringLiteral("debug-run-cursor")), i18n("Run to &Cursor"), this);
    action->setToolTip( i18n("Run to cursor") );
    action->setWhatsThis(i18n("Continues execution until the cursor position is reached."));
    connect(action, &QAction::triggered, this, &DebugController::runToCursor);
    ac->addAction(QStringLiteral("debug_runtocursor"), action);


    m_jumpToCursor = action = new QAction(QIcon::fromTheme(QStringLiteral("debug-execute-to-cursor")), i18n("Set E&xecution Position to Cursor"), this);
    action->setToolTip( i18n("Jump to cursor") );
    action->setWhatsThis(i18n("Continue execution from the current cursor position."));
    connect(action, &QAction::triggered, this, &DebugController::jumpToCursor);
    ac->addAction(QStringLiteral("debug_jumptocursor"), action);

    m_stepOver = action = new QAction(QIcon::fromTheme(QStringLiteral("debug-step-over")), i18n("Step &Over"), this);
    ac->setDefaultShortcut( action, Qt::Key_F10);
    action->setToolTip( i18n("Step over the next line") );
    action->setWhatsThis( i18n("Executes one line of source in the current source file. "
                               "If the source line is a call to a function the whole "
                               "function is executed and the app will stop at the line "
                               "following the function call.") );
    connect(action, &QAction::triggered, this, &DebugController::stepOver);
    ac->addAction(QStringLiteral("debug_stepover"), action);


    m_stepOverInstruction = action = new QAction(QIcon::fromTheme(QStringLiteral("debug-step-instruction")), i18n("Step over Ins&truction"), this);
    action->setToolTip( i18n("Step over instruction") );
    action->setWhatsThis(i18n("Steps over the next assembly instruction."));
    connect(action, &QAction::triggered, this, &DebugController::stepOverInstruction);
    ac->addAction(QStringLiteral("debug_stepoverinst"), action);


    m_stepInto = action = new QAction(QIcon::fromTheme(QStringLiteral("debug-step-into")), i18n("Step &Into"), this);
    ac->setDefaultShortcut( action, Qt::Key_F11);
    action->setToolTip( i18n("Step into the next statement") );
    action->setWhatsThis( i18n("Executes exactly one line of source. If the source line "
                               "is a call to a function then execution will stop after "
                               "the function has been entered.") );
    connect(action, &QAction::triggered, this, &DebugController::stepInto);
    ac->addAction(QStringLiteral("debug_stepinto"), action);


    m_stepIntoInstruction = action = new QAction(QIcon::fromTheme(QStringLiteral("debug-step-into-instruction")), i18n("Step into I&nstruction"), this);
    action->setToolTip( i18n("Step into instruction") );
    action->setWhatsThis(i18n("Steps into the next assembly instruction."));
    connect(action, &QAction::triggered, this, &DebugController::stepIntoInstruction);
    ac->addAction(QStringLiteral("debug_stepintoinst"), action);

    m_stepOut = action = new QAction(QIcon::fromTheme(QStringLiteral("debug-step-out")), i18n("Step O&ut"), this);
    ac->setDefaultShortcut( action, Qt::Key_F12);
    action->setToolTip( i18n("Step out of the current function") );
    action->setWhatsThis( i18n("Executes the application until the currently executing "
                               "function is completed. The debugger will then display "
                               "the line after the original call to that function. If "
                               "program execution is in the outermost frame (i.e. in "
                               "main()) then this operation has no effect.") );
    connect(action, &QAction::triggered, this, &DebugController::stepOut);
    ac->addAction(QStringLiteral("debug_stepout"), action);

    m_toggleBreakpoint = action = new QAction(QIcon::fromTheme(QStringLiteral("breakpoint")), i18n("Toggle Breakpoint"), this);
    ac->setDefaultShortcut( action, i18n("Ctrl+Alt+B") );
    action->setToolTip(i18n("Toggle breakpoint"));
    action->setWhatsThis(i18n("Toggles the breakpoint at the current line in editor."));
    connect(action, &QAction::triggered, this, &DebugController::toggleBreakpoint);
    ac->addAction(QStringLiteral("debug_toggle_breakpoint"), action);

    m_showCurrentLine = action = new QAction(QIcon::fromTheme(QStringLiteral("go-jump")), i18n("Show Current Line"), this);
    action->setToolTip(i18n("Show the current execution position"));
    action->setWhatsThis(i18n("Jumps to the execution line in the editor."));
    connect(action, &QAction::triggered, this, &DebugController::showCurrentLine);
    ac->addAction(QStringLiteral("debug_showcurrentline"), action);
}

void DebugController::addSession(IDebugSession* session)
{
    qCDebug(SHELL) << session;
    Q_ASSERT(session->variableController());
    Q_ASSERT(session->breakpointController());
    Q_ASSERT(session->frameStackModel());

    //TODO support multiple sessions
    if (m_currentSession) {
        m_currentSession.data()->stopDebugger();
    }
    m_currentSession = session;

    connect(session, &IDebugSession::stateChanged, this, &DebugController::debuggerStateChanged);
    connect(session, &IDebugSession::showStepInSource, this, &DebugController::showStepInSource);
    connect(session, &IDebugSession::clearExecutionPoint, this, &DebugController::clearExecutionPoint);
    connect(session, &IDebugSession::raiseFramestackViews, this, &DebugController::raiseFramestackViews);

    updateDebuggerState(session->state(), session);

    emit currentSessionChanged(session);

    if((Core::self()->setupFlags() & Core::NoUi)) return;


    Sublime::MainWindow* mainWindow = Core::self()->uiControllerInternal()->activeSublimeWindow();
    if (mainWindow->area()->objectName() != QLatin1String("debug")) {
        QString workingSet = mainWindow->area()->workingSet();
        ICore::self()->uiController()->switchToArea(QStringLiteral("debug"), IUiController::ThisWindow);
        mainWindow->area()->setWorkingSet(workingSet);
        connect(mainWindow, &Sublime::MainWindow::areaChanged, this, &DebugController::areaChanged);
    }
}

void DebugController::clearExecutionPoint()
{
    qCDebug(SHELL);
    const auto documents = KDevelop::ICore::self()->documentController()->openDocuments();
    for (KDevelop::IDocument* document : documents) {
        auto* iface = qobject_cast<KTextEditor::MarkInterface*>(document->textDocument());
        if (!iface)
            continue;

        const auto oldMarks = iface->marks();
        for (KTextEditor::Mark* mark : oldMarks) {
            if (mark->type & KTextEditor::MarkInterface::Execution) {
                iface->removeMark( mark->line, KTextEditor::MarkInterface::Execution );
            }
        }
    }
}

void DebugController::showStepInSource(const QUrl &url, int lineNum)
{
    if((Core::self()->setupFlags() & Core::NoUi)) return;

    clearExecutionPoint();
    qCDebug(SHELL) << url << lineNum;

    Q_ASSERT(qobject_cast<IDebugSession*>(sender()));
    QPair<QUrl,int> openUrl = static_cast<IDebugSession*>(sender())->convertToLocalUrl(qMakePair<QUrl,int>( url, lineNum ));
    KDevelop::IDocument* document = KDevelop::ICore::self()
        ->documentController()
        ->openDocument(openUrl.first, KTextEditor::Cursor(openUrl.second, 0), IDocumentController::DoNotFocus);

    if( !document )
        return;

    auto* iface = qobject_cast<KTextEditor::MarkInterface*>(document->textDocument());
    if( !iface )
        return;

    {
        QSignalBlocker blocker(document->textDocument());
        iface->addMark( lineNum, KTextEditor::MarkInterface::Execution );
    }
}


void DebugController::debuggerStateChanged(KDevelop::IDebugSession::DebuggerState state)
{
    Q_ASSERT(qobject_cast<IDebugSession*>(sender()));
    auto* session = static_cast<IDebugSession*>(sender());
    qCDebug(SHELL) << session << state << "current" << m_currentSession.data();
    if (session == m_currentSession.data()) {
        updateDebuggerState(state, session);
    }

    if (state == IDebugSession::EndedState) {
        if (session == m_currentSession.data()) {
            m_currentSession.clear();
            emit currentSessionChanged(nullptr);
            if (!Core::self()->shuttingDown()) {
                Sublime::MainWindow* mainWindow = Core::self()->uiControllerInternal()->activeSublimeWindow();
                if (mainWindow && mainWindow->area()->objectName() != QLatin1String("code")) {
                    QString workingSet = mainWindow->area()->workingSet();
                    ICore::self()->uiController()->switchToArea(QStringLiteral("code"), IUiController::ThisWindow);
                    mainWindow->area()->setWorkingSet(workingSet);
                }
                ICore::self()->uiController()->findToolView(i18n("Debug"), nullptr, IUiController::Raise);
            }
        }
        session->deleteLater();
    }
}

void DebugController::updateDebuggerState(IDebugSession::DebuggerState state, IDebugSession *session)
{
    Q_UNUSED(session);
    if((Core::self()->setupFlags() & Core::NoUi)) return;

    qCDebug(SHELL) << state;
    switch (state) {
        case IDebugSession::StoppedState:
        case IDebugSession::NotStartedState:
        case IDebugSession::StoppingState:
            qCDebug(SHELL) << "new state: stopped";
            stateChanged(QStringLiteral("stopped"));
            setContinueStartsDebug(true);
            //m_restartDebugger->setEnabled(session->restartAvailable());
            break;
        case IDebugSession::StartingState:
        case IDebugSession::PausedState:
            qCDebug(SHELL) << "new state: paused";
            stateChanged(QStringLiteral("paused"));
            setContinueStartsDebug(false);
            //m_restartDebugger->setEnabled(session->restartAvailable());
            break;
        case IDebugSession::ActiveState:
            qCDebug(SHELL) << "new state: active";
            stateChanged(QStringLiteral("active"));
            setContinueStartsDebug(false);
            //m_restartDebugger->setEnabled(false);
            break;
        case IDebugSession::EndedState:
            qCDebug(SHELL) << "new state: ended";
            stateChanged(QStringLiteral("ended"));
            setContinueStartsDebug(true);
            //m_restartDebugger->setEnabled(false);
            break;
    }
    if (state == IDebugSession::PausedState && ICore::self()->uiController()->activeMainWindow()) {
        ICore::self()->uiController()->activeMainWindow()->activateWindow();
    }
}

void DebugController::setContinueStartsDebug(bool startsDebug)
{
    if (startsDebug) {
        m_continueDebugger->setText(i18n("Debug Launch"));
        m_continueDebugger->setIcon(QIcon::fromTheme(QStringLiteral("debug-run")));
        m_continueDebugger->setToolTip(i18n("Debug current launch"));
        m_continueDebugger->setWhatsThis(i18n("Executes the target or the program specified in "
                                              "currently active launch configuration inside a Debugger."));
    } else {
        m_continueDebugger->setText(i18n("&Continue"));
        m_continueDebugger->setIcon(QIcon::fromTheme(QStringLiteral("media-playback-start")));
        m_continueDebugger->setToolTip(i18n("Continue application execution") );
        m_continueDebugger->setWhatsThis(i18n("Continues the execution of your application in the "
                                              "debugger. This only takes effect when the application "
                                              "has been halted by the debugger (i.e. a breakpoint has "
                                              "been activated or the interrupt was pressed).") );
    }
}

ContextMenuExtension DebugController::contextMenuExtension(Context* context, QWidget* parent)
{
    Q_UNUSED(parent);

    ContextMenuExtension menuExt;

    if( context->type() != Context::EditorContext )
        return menuExt;

    auto *econtext = dynamic_cast<KDevelop::EditorContext*>(context);
    if (!econtext)
        return menuExt;

    if (m_currentSession && m_currentSession.data()->isRunning()) {
        menuExt.addAction( KDevelop::ContextMenuExtension::DebugGroup, m_runToCursor);
    }

    if (econtext->url().isLocalFile()) {
        menuExt.addAction( KDevelop::ContextMenuExtension::DebugGroup, m_toggleBreakpoint);
    }
    return menuExt;
}

#if 0
void DebugController::restartDebugger() {
    if (m_currentSession) {
        m_currentSession.data()->restartDebugger();
    }
}
#endif

void DebugController::stopDebugger() {
    if (m_currentSession) {
        m_currentSession.data()->stopDebugger();
    }
}
void DebugController::interruptDebugger() {
    if (m_currentSession) {
        m_currentSession.data()->interruptDebugger();
    }
}

void DebugController::run() {
    if (m_currentSession) {
        m_currentSession.data()->run();
    } else {
        auto runController = ICore::self()->runController();
        if (runController->launchConfigurations().isEmpty()) {
            runController->showConfigurationDialog();
        }
        runController->executeDefaultLaunch(QStringLiteral("debug"));
    }
}

void DebugController::runToCursor() {
    if (m_currentSession) {
        m_currentSession.data()->runToCursor();
    }
}
void DebugController::jumpToCursor() {
    if (m_currentSession) {
        m_currentSession.data()->jumpToCursor();
    }
}
void DebugController::stepOver() {
    if (m_currentSession) {
        m_currentSession.data()->stepOver();
    }
}
void DebugController::stepIntoInstruction() {
    if (m_currentSession) {
        m_currentSession.data()->stepIntoInstruction();
    }
}
void DebugController::stepInto() {
    if (m_currentSession) {
        m_currentSession.data()->stepInto();
    }
}
void DebugController::stepOverInstruction() {
    if (m_currentSession) {
        m_currentSession.data()->stepOverInstruction();
    }
}
void DebugController::stepOut() {
    if (m_currentSession) {
        m_currentSession.data()->stepOut();
    }
}

void DebugController::areaChanged(Sublime::Area* newArea)
{
    if (newArea->objectName()!=QLatin1String("debug")) {
        stopDebugger();
    }
}

void DebugController::toggleBreakpoint()
{
    if (KDevelop::IDocument* document = KDevelop::ICore::self()->documentController()->activeDocument()) {
        KTextEditor::Cursor cursor = document->cursorPosition();
        if (!cursor.isValid()) return;
        breakpointModel()->toggleBreakpoint(document->url(), cursor);
    }
}

void DebugController::showCurrentLine()
{
    const auto location = qMakePair(m_currentSession->currentUrl(), m_currentSession->currentLine());

    if (location.second != -1) {
        const auto localLocation = m_currentSession->convertToLocalUrl(location);
        ICore::self()->documentController()->openDocument(localLocation.first,
                                                          KTextEditor::Cursor(localLocation.second, 0),
                                                          IDocumentController::DefaultMode);
    }

}

const QPixmap* DebugController::executionPointPixmap()
{
#if KTEXTEDITOR_VERSION >= QT_VERSION_CHECK(5,50,0)
    constexpr int markPixmapSize = 32;
#else
    constexpr int markPixmapSize = 22;
#endif
    static QPixmap pixmap=QIcon::fromTheme(QStringLiteral("go-next")).pixmap(QSize(markPixmapSize, markPixmapSize), QIcon::Normal, QIcon::Off);
    return &pixmap;
}

}

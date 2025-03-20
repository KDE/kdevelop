/*
    SPDX-FileCopyrightText: 1999-2001 John Birch <jbb@kdevelop.org>
    SPDX-FileCopyrightText: 2001 Bernd Gehrmann <bernd@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Vladimir Prus <ghost@cs.msu.su>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "debugcontroller.h"

#include <QAction>

#include <KActionCollection>
#include <KLocalizedString>
#include <KMessageBox>
#include <KParts/MainWindow>
#include <KTextEditor/Document>
#include <KXMLGUIFactory>

#include "../interfaces/idocument.h"
#include "../interfaces/icore.h"
#include "../interfaces/idocumentcontroller.h"
#include "../interfaces/contextmenuextension.h"
#include "../interfaces/context.h"
#include "../language/interfaces/editorcontext.h"
#include "../sublime/mainwindow.h"
#include "../sublime/area.h"
#include "../debugger/breakpoint/breakpointmodel.h"
#include "../debugger/breakpoint/breakpointwidget.h"
#include "../debugger/variable/variablewidget.h"
#include "../debugger/framestack/framestackmodel.h"
#include "../debugger/framestack/framestackwidget.h"
#include "../debugger/variable/variablecollection.h"
#include "core.h"
#include "debug.h"
#include "uicontroller.h"
#include "iruncontroller.h"

namespace {
constexpr auto executionMark = KTextEditor::Document::MarkTypes::Execution;
}

namespace KDevelop {

class DebuggerToolFactoryBase : public KDevelop::IToolViewFactory
{
public:
    DebuggerToolFactoryBase(DebugController* controller, const QString& id, Qt::DockWidgetArea defaultArea)
        : m_controller(controller)
        , m_id(id)
        , m_defaultArea(defaultArea)
    {
    }

    QString id() const override
    {
        return m_id;
    }

    Qt::DockWidgetArea defaultPosition() const override
    {
        return m_defaultArea;
    }

protected:
    auto controller() const
    {
        return m_controller;
    }

private:
    DebugController* const m_controller;
    const QString m_id;
    const Qt::DockWidgetArea m_defaultArea;
};

template<class T>
class DebuggerToolFactory : public DebuggerToolFactoryBase
{
public:
    using DebuggerToolFactoryBase::DebuggerToolFactoryBase;

    QWidget* create(QWidget* parent = nullptr) override
    {
        return new T(controller(), parent);
    }
};

template<class T>
class DebuggerToolWithoutToolbarFactory : public DebuggerToolFactory<T>
{
public:
    using DebuggerToolFactory<T>::DebuggerToolFactory;

    // At present, some debugger widgets (e.g. breakpoint) contain actions so that shortcuts
    // work, but they don't need any toolbar.  So, suppress toolbar action.
    QList<QAction*> toolBarActions(QWidget* viewWidget) const override
    {
        Q_UNUSED(viewWidget);
        return {};
    }
};

DebugController::DebugController(QObject *parent)
    : IDebugController(parent), KXMLGUIClient(),
      m_breakpointModel(new BreakpointModel(this)),
      m_variableCollection(new VariableCollection(this))
{
    qCDebug(SHELL) << "creating debug controller";

    setComponentName(QStringLiteral("kdevdebugger"), i18n("Debugger"));
    setXMLFile(QStringLiteral("kdevdebuggershellui.rc"));

    if (const auto* mainWindow = Core::self()->uiControllerInternal()->activeSublimeWindow()) {
        connect(mainWindow, &Sublime::MainWindow::areaChanged, this, &DebugController::areaChanged);
    }

    auto* const documentController = KDevelop::ICore::self()->documentController();
    Q_ASSERT(documentController); // DebugController is created after DocumentController.

    // This constructor is invoked before controllers are initialized, and thus before any documents can be opened.
    // So our textDocumentCreated() slot will be invoked for all documents.
    Q_ASSERT(documentController->openDocuments().empty());
    connect(documentController, &IDocumentController::textDocumentCreated, this, &DebugController::textDocumentCreated);
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
        i18nc("@title:window", "Frame Stack"),
        new DebuggerToolWithoutToolbarFactory<FramestackWidget>(this, QStringLiteral("org.kdevelop.debugger.StackView"),
                                                                Qt::BottomDockWidgetArea));

    ICore::self()->uiController()->addToolView(
        i18nc("@title:window", "Breakpoints"),
        new DebuggerToolWithoutToolbarFactory<BreakpointWidget>(
            this, QStringLiteral("org.kdevelop.debugger.BreakpointsView"), Qt::BottomDockWidgetArea));

    ICore::self()->uiController()->addToolView(
        i18nc("@title:window", "Variables"),
        new DebuggerToolFactory<VariableWidget>(this, QStringLiteral("org.kdevelop.debugger.VariablesView"),
                                                Qt::LeftDockWidgetArea));

    ICore::self()->uiController()->activeMainWindow()->guiFactory()->addClient(this);

    stateChanged(QStringLiteral("ended"));
}


void DebugController::cleanup()
{
    if (m_currentSession) m_currentSession.data()->stopDebugger();
}

DebugController::~DebugController()
{
    qCDebug(SHELL) << "destroying debug controller";

    // The longest possible time interval has been allotted for previous and
    // the current debug sessions to stop their debugger processes: stopDebugger()
    // was called on the sessions in addSession() and cleanup() respectively.
    // Now is the last safe moment for a DebugSession to finalize its state and
    // invoke DebugController::debuggerStateChanged(), which in turn schedules the
    // session's deletion. This finalization not only ensures that debug sessions
    // are destroyed, but also prevents crashes: a DebugSession's state transition
    // signals lead to accesses to DebugController and its children, such as a
    // call to BreakpointModel::updateState(). Therefore we must force all debug
    // sessions to synchronously finalize their states now.
    emit killAllDebuggersNow();

    qCDebug(SHELL) << "destroyed debug controller";
}

BreakpointModel* DebugController::breakpointModel()
{
    return m_breakpointModel;
}

VariableCollection* DebugController::variableCollection()
{
    return m_variableCollection;
}

void DebugController::textDocumentCreated(KDevelop::IDocument* document)
{
    Q_ASSERT(document->textDocument());
    static const auto executionMarkIcon = QIcon::fromTheme(QStringLiteral("go-next"));
    document->textDocument()->setMarkIcon(executionMark, executionMarkIcon);
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

    m_interruptDebugger = action = new QAction(QIcon::fromTheme(QStringLiteral("media-playback-pause")), i18nc("@action", "Interrupt"), this);
    action->setToolTip( i18nc("@info:tooltip", "Interrupt application") );
    action->setWhatsThis(i18nc("@info:whatsthis", "Interrupts the debugged process or current debugger command."));
    connect(action, &QAction::triggered, this, &DebugController::interruptDebugger);
    ac->addAction(QStringLiteral("debug_pause"), action);

    m_runToCursor = action = new QAction(QIcon::fromTheme(QStringLiteral("debug-run-cursor")), i18nc("@action", "Run to &Cursor"), this);
    action->setToolTip( i18nc("@info:tooltip", "Run to cursor") );
    action->setWhatsThis(i18nc("@info:whatsthis", "Continues execution until the cursor position is reached."));
    connect(action, &QAction::triggered, this, &DebugController::runToCursor);
    ac->addAction(QStringLiteral("debug_runtocursor"), action);


    m_jumpToCursor = action = new QAction(QIcon::fromTheme(QStringLiteral("debug-execute-to-cursor")), i18nc("@action", "Set E&xecution Position to Cursor"), this);
    action->setToolTip( i18nc("@info:tooltip", "Jump to cursor") );
    action->setWhatsThis(i18nc("@info:whatsthis", "Continue execution from the current cursor position."));
    connect(action, &QAction::triggered, this, &DebugController::jumpToCursor);
    ac->addAction(QStringLiteral("debug_jumptocursor"), action);

    m_stepOver = action = new QAction(QIcon::fromTheme(QStringLiteral("debug-step-over")), i18nc("@action", "Step &Over"), this);
    ac->setDefaultShortcut( action, Qt::Key_F10);
    action->setToolTip( i18nc("@info:tooltip", "Step over the next line") );
    action->setWhatsThis( i18nc("@info:whatsthis", "Executes one line of source in the current source file. "
                               "If the source line is a call to a function the whole "
                               "function is executed and the app will stop at the line "
                               "following the function call.") );
    connect(action, &QAction::triggered, this, &DebugController::stepOver);
    ac->addAction(QStringLiteral("debug_stepover"), action);


    m_stepOverInstruction = action = new QAction(QIcon::fromTheme(QStringLiteral("debug-step-instruction")), i18nc("@action", "Step over Ins&truction"), this);
    action->setToolTip( i18nc("@info:tooltip", "Step over instruction") );
    action->setWhatsThis(i18nc("@info:whatsthis", "Steps over the next assembly instruction."));
    connect(action, &QAction::triggered, this, &DebugController::stepOverInstruction);
    ac->addAction(QStringLiteral("debug_stepoverinst"), action);


    m_stepInto = action = new QAction(QIcon::fromTheme(QStringLiteral("debug-step-into")), i18nc("@action", "Step &Into"), this);
    ac->setDefaultShortcut( action, Qt::Key_F11);
    action->setToolTip( i18nc("@info:tooltip", "Step into the next statement") );
    action->setWhatsThis( i18nc("@info:whatsthis", "Executes exactly one line of source. If the source line "
                               "is a call to a function then execution will stop after "
                               "the function has been entered.") );
    connect(action, &QAction::triggered, this, &DebugController::stepInto);
    ac->addAction(QStringLiteral("debug_stepinto"), action);


    m_stepIntoInstruction = action = new QAction(QIcon::fromTheme(QStringLiteral("debug-step-into-instruction")), i18nc("@action", "Step into I&nstruction"), this);
    action->setToolTip( i18nc("@info:tooltip", "Step into instruction") );
    action->setWhatsThis(i18nc("@info:whatsthis", "Steps into the next assembly instruction."));
    connect(action, &QAction::triggered, this, &DebugController::stepIntoInstruction);
    ac->addAction(QStringLiteral("debug_stepintoinst"), action);

    m_stepOut = action = new QAction(QIcon::fromTheme(QStringLiteral("debug-step-out")), i18nc("@action", "Step O&ut"), this);
    ac->setDefaultShortcut( action, Qt::Key_F12);
    action->setToolTip( i18nc("@info:tooltip", "Step out of the current function") );
    action->setWhatsThis( i18nc("@whatsthis", "Executes the application until the currently executing "
                               "function is completed. The debugger will then display "
                               "the line after the original call to that function. If "
                               "program execution is in the outermost frame (i.e. in "
                               "main()) then this operation has no effect.") );
    connect(action, &QAction::triggered, this, &DebugController::stepOut);
    ac->addAction(QStringLiteral("debug_stepout"), action);

    m_toggleBreakpoint = action = new QAction(QIcon::fromTheme(QStringLiteral("breakpoint")), i18nc("@action", "Toggle Breakpoint"), this);
    ac->setDefaultShortcut( action, i18n("Ctrl+Alt+B") );
    action->setToolTip(i18nc("@info:tooltip", "Toggle breakpoint"));
    action->setWhatsThis(i18nc("@info:whatsthis", "Toggles the breakpoint at the current line in editor."));
    connect(action, &QAction::triggered, this, &DebugController::toggleBreakpoint);
    ac->addAction(QStringLiteral("debug_toggle_breakpoint"), action);

    m_showCurrentLine = action = new QAction(QIcon::fromTheme(QStringLiteral("go-jump")), i18nc("@action", "Show Current Line"), this);
    action->setToolTip(i18nc("@info:tooltip", "Show the current execution position"));
    action->setWhatsThis(i18nc("@info:whatsthis", "Jumps to the execution line in the editor."));
    connect(action, &QAction::triggered, this, &DebugController::showCurrentLine);
    ac->addAction(QStringLiteral("debug_showcurrentline"), action);
}

bool DebugController::canAddSession(const QString& replaceSessionQuestionText) const
{
    if (!m_currentSession) {
        return true;
    }

    const auto answer = KMessageBox::warningTwoActions(
        ICore::self()->uiController()->activeMainWindow(), replaceSessionQuestionText, {},
        KGuiItem(i18nc("@action:button", "Abort Current Session"), QStringLiteral("application-exit")),
        KStandardGuiItem::cancel());
    return answer == KMessageBox::PrimaryAction;
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
    connect(this, &DebugController::killAllDebuggersNow, session, &IDebugSession::killDebuggerNow);

    updateDebuggerState(session->state(), session);

    emit currentSessionChanged(session);

    if((Core::self()->setupFlags() & Core::NoUi)) return;


    Sublime::MainWindow* mainWindow = Core::self()->uiControllerInternal()->activeSublimeWindow();
    auto oldArea = mainWindow->area();
    if (oldArea->objectName() != QLatin1String("debug")) {
        ICore::self()->uiController()->switchToArea(QStringLiteral("debug"), IUiController::ThisWindow);
        mainWindow->area()->setWorkingSet(oldArea->workingSet(), oldArea->workingSetPersistent(), oldArea);
    }
}

void DebugController::clearExecutionPoint()
{
    const auto* const documentController = KDevelop::ICore::self()->documentController();
    if (!documentController) {
        qCDebug(SHELL) << "Cannot clear execution point without the document controller. "
                          "KDevelop must be exiting and the document controller already destroyed.";
        return;
    }

    auto* const document = m_lastExecMarkDocument.data();
    // Do we even have a document with execution mark?
    if (!document) {
        return;
    }
    m_lastExecMarkDocument = nullptr;

    constexpr auto markTypeToRemove = executionMark;

    // Is the execution mark still at the line it was added to?
    if (document->mark(m_lastExecMarkLine) & markTypeToRemove) {
        document->removeMark(m_lastExecMarkLine, markTypeToRemove);
        return;
    }

    // The execution mark is on some other line (probably because the document was edited
    // during the current debug session). Iterate over all marks in the document to find and remove it.
    const auto& marks = document->marks();
    const auto it = std::find_if(marks.begin(), marks.end(), [](const KTextEditor::Mark* mark) -> bool {
        // TODO: remove the following line once building KDevelop with Visual Studio 2019 is no longer supported.
        constexpr auto markTypeToRemove = executionMark;
        return mark->type & markTypeToRemove;
    });

    if (it != marks.end()) {
        document->removeMark(it.key(), markTypeToRemove);
        return;
    }

    qCWarning(SHELL) << "failed to remove execution mark from" << document->url().toString(QUrl::PreferLocalFile);
}

void DebugController::showStepInSource(const QUrl& originalUrl, int originalLine)
{
    if((Core::self()->setupFlags() & Core::NoUi)) return;

    clearExecutionPoint();
    qCDebug(SHELL) << "showing debugger step in source:" << originalUrl << originalLine;

    Q_ASSERT(qobject_cast<IDebugSession*>(sender()));
    const auto [url, line] = static_cast<IDebugSession*>(sender())->convertToLocalUrl({originalUrl, originalLine});

    const auto* const document = ICore::self()->documentController()->openDocument(url, KTextEditor::Cursor(line, 0),
                                                                                   IDocumentController::DoNotFocus);
    if( !document )
        return;

    auto* const textDocument = document->textDocument();
    if (!textDocument)
        return;

    m_lastExecMarkDocument = textDocument;
    m_lastExecMarkLine = line;
    textDocument->addMark(line, executionMark);
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
                    auto oldArea = mainWindow->area();
                    QString workingSet = oldArea->workingSet();
                    ICore::self()->uiController()->switchToArea(QStringLiteral("code"), IUiController::ThisWindow);
                    mainWindow->area()->setWorkingSet(workingSet, oldArea->workingSetPersistent(), oldArea);
                }
                ICore::self()->uiController()->raiseToolView(QStringLiteral("org.kdevelop.OutputView.Debug"));
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
        m_continueDebugger->setText(i18nc("@action", "Debug Launch"));
        m_continueDebugger->setIcon(QIcon::fromTheme(QStringLiteral("debug-run")));
        m_continueDebugger->setToolTip(i18nc("@info:tooltip", "Debug current launch"));
        m_continueDebugger->setWhatsThis(i18nc("@info:whatsthis", "Executes the target or the program specified in "
                                              "currently active launch configuration inside a Debugger."));
    } else {
        m_continueDebugger->setText(i18nc("@action", "&Continue"));
        m_continueDebugger->setIcon(QIcon::fromTheme(QStringLiteral("media-playback-start")));
        m_continueDebugger->setToolTip(i18nc("@info:tooltip", "Continue application execution") );
        m_continueDebugger->setWhatsThis(i18nc("@info:whatsthis", "Continues the execution of your application in the "
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
    if (newArea->objectName() != QLatin1String("debug") && newArea->objectName() != QLatin1String("review")) {
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
}

#include "moc_debugcontroller.cpp"

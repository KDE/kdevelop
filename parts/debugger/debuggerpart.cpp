/***************************************************************************
 *   Copyright (C) 1999-2001 by John Birch                                 *
 *   jbb@kdevelop.org                                                      *
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qdir.h>
#include <qvbox.h>
#include <qwhatsthis.h>
#include <qpopupmenu.h>

#include <kaction.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmainwindow.h>
#include <kstatusbar.h>
#include <kparts/part.h>
#include <ktexteditor/viewcursorinterface.h>
#include <kmessagebox.h>

#include "kdevcore.h"
#include "kdevproject.h"
#include "kdevmainwindow.h"
#include "kdevappfrontend.h"
#include "kdevpartcontroller.h"
#include "kdevdebugger.h"
#include "domutil.h"
#include "variablewidget.h"
#include "breakpointwidget.h"
#include "framestackwidget.h"
#include "disassemblewidget.h"
#include "processwidget.h"
#include "gdbcontroller.h"
#include "breakpoint.h"
#include "dbgpsdlg.h"
#include "dbgtoolbar.h"
#include "memviewdlg.h"
#include "gdbparser.h"
#include "gdboutputwidget.h"
#include "debuggerconfigwidget.h"
#include "processlinemaker.h"

#include <iostream>

#include "debuggerpart.h"

namespace GDBDebugger
{

typedef KGenericFactory<DebuggerPart> DebuggerFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevdebugger, DebuggerFactory( "kdevdebugger" ) );

DebuggerPart::DebuggerPart( QObject *parent, const char *name, const QStringList & )
    : KDevPlugin( "CppDebugger", "debugger", parent, name ? name : "DebuggerPart" ),
      controller(0)
{
    setInstance(DebuggerFactory::instance());

    setXMLFile("kdevdebugger.rc");

    statusBarIndicator = new QLabel(" ", mainWindow()->statusBar());
    statusBarIndicator->setFixedWidth(15);
    mainWindow()->statusBar()->addWidget(statusBarIndicator, 0, true);
    statusBarIndicator->show();

    //
    // Setup widgets and dbgcontroller
    //

    variableWidget = new VariableWidget();
    variableWidget->setEnabled(false);
    variableWidget->setIcon(SmallIcon("math_brace"));
    variableWidget->setCaption(i18n("Variable Tree"));
    QWhatsThis::add(variableWidget, i18n("Variable tree\n\n"
                                         "The variable tree allows you to see "
                                         "the variable values as you step "
                                         "through your program using the internal "
                                         "debugger. Click the RMB on items in "
                                         "this view to get a popup menu.\n"
                                         "To speed up stepping through your code "
                                         "leave the tree items closed and add the "
                                         "variable(s) to the watch section.\n"
                                         "To change a variable value in your "
                                         "running app use a watch variable (eg a=5)."));
    mainWindow()->embedSelectView(variableWidget, i18n("Variables / Watch"), i18n("debugger variable-view"));
    mainWindow()->setViewAvailable(variableWidget, false);

    breakpointWidget = new BreakpointWidget();
    breakpointWidget->setCaption(i18n("Breakpoint List"));
    QWhatsThis::add(breakpointWidget, i18n("Breakpoint list\n\n"
                                           "Displays a list of breakpoints with "
                                           "their current status. Clicking on a "
                                           "breakpoint item with the RMB displays "
                                           "a popupmenu so you may manipulate the "
                                           "breakpoint. Double clicking will take you "
                                           "to the source in the editor window."));
    mainWindow()->embedOutputView(breakpointWidget, i18n("Breakpoints"), i18n("debugger breakpoints"));

    framestackWidget = new FramestackWidget();
    framestackWidget->setEnabled(false);
    framestackWidget->setCaption(i18n("Frame Stack"));
    QWhatsThis::add(framestackWidget, i18n("Frame stack\n\n"
                                           "Often referred to as the \"call stack\", "
                                           "this is a list showing what function is "
                                           "currently active and who called each "
                                           "function to get to this point in your "
                                           "program. By clicking on an item you "
                                           "can see the values in any of the "
                                           "previous calling functions."));
    mainWindow()->embedOutputView(framestackWidget, i18n("Frame Stack"), i18n("debugger function call stack"));
    mainWindow()->setViewAvailable(framestackWidget, false);

    disassembleWidget = new DisassembleWidget();
    disassembleWidget->setEnabled(false);
    disassembleWidget->setCaption(i18n("Machine Code Display"));
    QWhatsThis::add(disassembleWidget, i18n("Machine code display\n\n"
                                            "A machine code view into your running "
                                            "executable with the current instruction "
                                            "highlighted. You can step instruction by "
                                            "instruction using the debuggers toolbar "
                                            "buttons of \"step over\" instruction and "
                                            "\"step into\" instruction."));
    mainWindow()->embedOutputView(disassembleWidget, i18n("Disassemble"),
                                        i18n("debugger disassemble view"));
    mainWindow()->setViewAvailable(disassembleWidget, false);

    gdbOutputWidget = new GDBOutputWidget;
    gdbOutputWidget->setEnabled(false);
    mainWindow()->embedOutputView(gdbOutputWidget, i18n("GDB"),
                                                i18n("GDB output"));
    mainWindow()->setViewAvailable(gdbOutputWidget, false);

    VariableTree *variableTree = variableWidget->varTree();

    // variableTree -> framestackWidget
    connect( variableTree,     SIGNAL(selectFrame(int, int)),
             framestackWidget, SLOT(slotSelectFrame(int, int)));

    // breakpointWidget -> this
    connect( breakpointWidget, SIGNAL(refreshBPState(const Breakpoint&)),
             this,             SLOT(slotRefreshBPState(const Breakpoint&)));
    connect( breakpointWidget, SIGNAL(publishBPState(const Breakpoint&)),
             this,             SLOT(slotRefreshBPState(const Breakpoint&)));
    connect( breakpointWidget, SIGNAL(gotoSourcePosition(const QString&, int)),
             this,             SLOT(slotGotoSource(const QString&, int)) );

    //
    // Now setup the actions
    //

    KAction *action;

    action = new KAction(i18n("&Start"), "1rightarrow", 0,
                         this, SLOT(slotRun()),
                         actionCollection(), "debug_run");
    action->setStatusText( i18n("Runs the program in the debugger") );
    action->setWhatsThis( i18n("Start in debugger\n\n"
                               "Starts the debugger with the project's main "
                               "executable. You may set some breakpoints "
                               "before this, or you can interrupt the program "
                               "while it is running, in order to get information "
                               "about variables, frame stack, and so on.") );

    action = new KAction(i18n("Interrupt"), "player_pause", 0,
                         this, SLOT(slotPause()),
                         actionCollection(), "debug_pause");
    action->setStatusText( i18n("Interrupts the application") );

    action = new KAction(i18n("Run to &Cursor"), "dbgrunto", 0,
                         this, SLOT(slotRunToCursor()),
                         actionCollection(), "debug_runtocursor");
    action->setStatusText( i18n("Continues execution until the cursor position is reached") );


    action = new KAction(i18n("Step &Over"), "dbgnext", 0,
                         this, SLOT(slotStepOver()),
                         actionCollection(), "debug_stepover");
    action->setStatusText( i18n("Steps over the next line") );
    action->setWhatsThis( i18n("Step over\n\n"
                               "Executes one line of source in the current source file. "
                               "If the source line is a call to a function the whole "
                               "function is executed and the app will stop at the line "
                               "following the function call.") );


    action = new KAction(i18n("Step over Ins&truction"), "dbgnextinst", 0,
                         this, SLOT(slotStepOverInstruction()),
                         actionCollection(), "debug_stepoverinst");
    action->setStatusText( i18n("Steps over the next assembly instruction") );


    action = new KAction(i18n("Step &Into"), "dbgstep", 0,
                         this, SLOT(slotStepInto()),
                         actionCollection(), "debug_stepinto");
    action->setStatusText( i18n("Steps into the next statement") );
    action->setWhatsThis( i18n("Step into\n\n"
                               "Executes exactly one line of source. If the source line "
                               "is a call to a function then execution will stop after "
                               "the function has been entered.") );



    action = new KAction(i18n("Step into I&nstruction"), "dbgstepinst", 0,
                         this, SLOT(slotStepIntoInstruction()),
                         actionCollection(), "debug_stepintoinst");
    action->setStatusText( i18n("Steps into the next assembly instruction") );


    action = new KAction(i18n("Step O&ut"), "dbgstepout", 0,
                         this, SLOT(slotStepOut()),
                         actionCollection(), "debug_stepout");
    action->setStatusText( i18n("Steps out of the current function") );
    action->setWhatsThis( i18n("Step out of\n\n"
                               "Executes the application until the currently executing "
                               "function is completed. The debugger will then display "
                               "the line after the original call to that function. If "
                               "program execution is in the outermost frame (i.e. in "
                               "main()) then this operation has no effect.") );


    action = new KAction(i18n("Viewers"), "dbgmemview", 0,
                         this, SLOT(slotMemoryView()),
                         actionCollection(), "debug_memview");
    action->setStatusText( i18n("Various views into the application") );


    action = new KAction(i18n("Examine Core File"), "core", 0,
                         this, SLOT(slotExamineCore()),
                         actionCollection(), "debug_core");
    action->setStatusText( i18n("Loads a core file into the debugger") );
    action->setWhatsThis( i18n("Examine core file\n\n"
                               "This loads a core file, which is typically created "
                               "after the application has crashed, e.g. with a "
                               "segmentation fault. The core file contains an "
                               "image of the program memory at the time it crashed, "
                               "allowing you to do a post-mortem analysis.") );


    action = new KAction(i18n("Attach to Process"), "connect_creating", 0,
                         this, SLOT(slotAttachProcess()),
                         actionCollection(), "debug_attach");
    action->setStatusText( i18n("Attaches the debugger to a running process") );


    action = new KAction(i18n("Toggle Breakpoint"), 0, 0,
                         this, SLOT(toggleBreakpoint()),
                         actionCollection(), "debug_toggle_breakpoint");

    connect( mainWindow()->main()->guiFactory(), SIGNAL(clientAdded(KXMLGUIClient*)),
             this, SLOT(guiClientAdded(KXMLGUIClient*)) );

    connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
             this, SLOT(projectConfigWidget(KDialogBase*)) );

    connect( partController(), SIGNAL(loadedFile(const QString &)),
             breakpointWidget, SLOT(refreshBP(const QString &)) );
    connect( debugger(), SIGNAL(toggledBreakpoint(const QString &, int)),
             breakpointWidget, SLOT(slotToggleBreakpoint(const QString &, int)) );
    connect( debugger(), SIGNAL(editedBreakpoint(const QString &, int)),
             breakpointWidget, SLOT(slotEditBreakpoint(const QString &, int)) );
    connect( debugger(), SIGNAL(toggledBreakpointEnabled(const QString &, int)),
             breakpointWidget, SLOT(slotToggleBreakpointEnabled(const QString &, int)) );

    connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
             this, SLOT(contextMenu(QPopupMenu *, const Context *)) );
    connect( core(), SIGNAL(stopButtonClicked()),
             this, SLOT(slotStop()) );

    connect( partController(), SIGNAL(activePartChanged(KParts::Part*)),
             this, SLOT(slotActivePartChanged(KParts::Part*)) );

    procLineMaker = new ProcessLineMaker();

    connect( procLineMaker, SIGNAL(receivedStdoutLine(const QString&)),
             appFrontend(), SLOT(insertStdoutLine(const QString&)) );
    connect( procLineMaker, SIGNAL(receivedStderrLine(const QString&)),
             appFrontend(), SLOT(insertStderrLine(const QString&)) );

    setupController();
}


DebuggerPart::~DebuggerPart()
{
    if (variableWidget)
        mainWindow()->removeView(variableWidget);
    if (breakpointWidget)
        mainWindow()->removeView(breakpointWidget);
    if (framestackWidget)
        mainWindow()->removeView(framestackWidget);
    if (disassembleWidget)
        mainWindow()->removeView(disassembleWidget);
    if(gdbOutputWidget)
        mainWindow()->removeView(gdbOutputWidget);

    delete variableWidget;
    delete breakpointWidget;
    delete framestackWidget;
    delete disassembleWidget;
    delete gdbOutputWidget;
    delete controller;
    delete floatingToolBar;
    delete statusBarIndicator;
    delete procLineMaker;

    GDBParser::destroy();
}


void DebuggerPart::guiClientAdded( KXMLGUIClient* client )
{
    // Can't change state until after XMLGUI has been loaded...
    // Anyone know of a better way of doing this?
    if( client == this )
        stateChanged( QString("stopped") );
}


//void DebuggerPart::projectOpened()
//{
//    QDomElement bpsElement = DomUtil::elementByPath( *projectDom(), "/kdevdebugger/breakpoints" );
//    QDomElement bpElement = bpsElement.firstChild().toElement();
//    while( !bpElement.isNull() ) {
//        QString filename = bpElement.attribute( "file" );
//        int lineNum = bpElement.attribute( "line" ).toInt();
//        // TODO: restore condition
//        breakpointWidget->slotToggleBreakpoint( filename, lineNum );
//        bpElement = bpElement.nextSibling().toElement();
//    }
//}
//
//
//void DebuggerPart::projectClosed()
//{
//    QDomElement bpsElement = DomUtil::createElementByPath( *projectDom(), "/kdevdebugger/breakpoints" );
//    DomUtil::makeEmpty( bpsElement );
//    const QPtrList<Breakpoint> bps = breakpointWidget->breakpoints();
//    for( QPtrListIterator<Breakpoint> it( bps ); it.current(); ++it ) {
//        Breakpoint* bp = it.current();
//        QDomElement bpElement = bpsElement.ownerDocument().createElement( "breakpoint" );
//        bpElement.setAttribute( "file", bp->fileName() );
//        bpElement.setAttribute( "line", bp->lineNum() - 1 );
//        bpElement.setAttribute( "condition", bp->conditional() );
//        bpsElement.appendChild( bpElement );
//    }
//}
//

void DebuggerPart::contextMenu(QPopupMenu *popup, const Context *context)
{
    if (!context->hasType("editor"))
      return;

    const EditorContext *econtext = static_cast<const EditorContext*>(context);
    m_contextIdent = econtext->currentWord();

    popup->insertSeparator();
    if (econtext->url().isLocalFile())
        popup->insertItem( i18n("Toggle Breakpoint"), this, SLOT(toggleBreakpoint()) );
    if (!m_contextIdent.isEmpty())
        popup->insertItem( i18n("Watch: %1").arg(m_contextIdent), this, SLOT(contextWatch()) );
}


void DebuggerPart::toggleBreakpoint()
{
    KParts::ReadWritePart *rwpart
        = dynamic_cast<KParts::ReadWritePart*>(partController()->activePart());
    KTextEditor::ViewCursorInterface *cursorIface
        = dynamic_cast<KTextEditor::ViewCursorInterface*>(partController()->activeWidget());

    if (!rwpart || !cursorIface)
        return;

    uint line, col;
    cursorIface->cursorPositionReal(&line, &col);

    breakpointWidget->slotToggleBreakpoint(rwpart->url().path(), line);
}


void DebuggerPart::contextWatch()
{
    variableWidget->slotAddWatchVariable(m_contextIdent);
}


void DebuggerPart::projectConfigWidget(KDialogBase *dlg)
{
    QVBox *vbox = dlg->addVBoxPage(i18n("Debugger"));
    DebuggerConfigWidget *w = new DebuggerConfigWidget(this, vbox, "debugger config widget");
    connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
    connect( dlg, SIGNAL(finished()), controller, SLOT(reConfig()) );
}


void DebuggerPart::setupController()
{
    VariableTree *variableTree = variableWidget->varTree();

    controller = new GDBController(variableTree, framestackWidget, *projectDom());

    // variableTree -> controller
//    connect( variableTree,     SIGNAL(expandItem(VarItem*)),
//             controller,       SLOT(slotExpandItem(VarItem*)));
    connect( variableTree,     SIGNAL(expandItem(TrimmableItem*)),
             controller,       SLOT(slotExpandItem(TrimmableItem*)));
    connect( variableTree,     SIGNAL(expandUserItem(VarItem*, const QCString&)),
             controller,       SLOT(slotExpandUserItem(VarItem*, const QCString&)));
    connect( variableTree,     SIGNAL(setLocalViewState(bool)),
             controller,       SLOT(slotSetLocalViewState(bool)));
    // jw
    connect( variableTree,     SIGNAL(varItemConstructed(VarItem*)),
             controller,       SLOT(slotVarItemConstructed(VarItem*)));

    // framestackWidget -> controller
    connect( framestackWidget, SIGNAL(selectFrame(int,int,bool)),
             controller,       SLOT(slotSelectFrame(int,int,bool)));

    // breakpointWidget -> controller
    connect( breakpointWidget, SIGNAL(clearAllBreakpoints()),
             controller,       SLOT(slotClearAllBreakpoints()));

    // disassembleWidget -> controller
    connect( disassembleWidget,SIGNAL(disassemble(const QString&, const QString&)),
             controller,       SLOT(slotDisassemble(const QString&, const QString&)));

    connect( gdbOutputWidget,  SIGNAL(userGDBCmd(const QString &)),
             controller,       SLOT(slotUserGDBCmd(const QString&)));

    // controller -> breakpointWidget
    connect( controller,       SIGNAL(acceptPendingBPs()),
             breakpointWidget, SLOT(slotSetPendingBPs()));
    connect( controller,       SIGNAL(unableToSetBPNow(int)),
             breakpointWidget, SLOT(slotUnableToSetBPNow(int)));
    connect( controller,       SIGNAL(rawGDBBreakpointList (char*)),
             breakpointWidget, SLOT(slotParseGDBBrkptList(char*)));
    connect( controller,       SIGNAL(rawGDBBreakpointSet(char*, int)),
             breakpointWidget, SLOT(slotParseGDBBreakpointSet(char*, int)));
    connect( breakpointWidget, SIGNAL(publishBPState(const Breakpoint&)),
             controller,       SLOT(slotBPState(const Breakpoint &)));

    // controller -> disassembleWidget
    connect( controller,       SIGNAL(showStepInSource(const QString&, int, const QString&)),
             disassembleWidget,SLOT(slotShowStepInSource(const QString&, int, const QString&)));
    connect( controller,       SIGNAL(rawGDBDisassemble(char*)),
             disassembleWidget,SLOT(slotDisassemble(char*)));

    // controller -> this
    connect( controller,       SIGNAL(dbgStatus(const QString&, int)),
             this,             SLOT(slotStatus(const QString&, int)));
    connect( controller,       SIGNAL(showStepInSource(const QString&, int, const QString&)),
             this,             SLOT(slotShowStep(const QString&, int)));

    connect( controller,       SIGNAL(ttyStdout(const char*)),
             procLineMaker,    SLOT(slotReceivedStdout(const char*)));
    connect( controller,       SIGNAL(ttyStderr(const char*)),
             procLineMaker,    SLOT(slotReceivedStderr(const char*)));

    connect( controller,       SIGNAL(gdbStdout(const char*)),
             gdbOutputWidget,  SLOT(slotReceivedStdout(const char*)) );
    connect( controller,       SIGNAL(gdbStderr(const char*)),
             gdbOutputWidget,  SLOT(slotReceivedStderr(const char*)) );
}


void DebuggerPart::startDebugger()
{
    QString program;
    if (project())
        program = project()->mainProgram();

    QString shell = DomUtil::readEntry(*projectDom(), "/kdevdebugger/general/dbgshell");
    if( !shell.isEmpty() ) {
        QFileInfo info( shell );
        if( info.isRelative() ) {
            shell = project()->buildDirectory() + "/" + shell;
            info.setFile( shell );
        }
        if( !info.exists() ) {
            KMessageBox::error(
                mainWindow()->main(),
                i18n("Could not locate the debugging shell '%1'.").arg( shell ),
                i18n("Debugging shell not found.") );
            return;
        }
    }

    core()->running(this, true);

    stateChanged( QString("active") );

    KActionCollection *ac = actionCollection();
    ac->action("debug_run")->setText( i18n("&Continue") );
    ac->action("debug_run")->setIcon( "dbgrun" );
    ac->action("debug_run")->setStatusText( i18n("Continues the application execution") );
    ac->action("debug_run")->setWhatsThis( i18n("Continue application execution\n\n"
                               "Continues the execution of your application in the "
                               "debugger. This only takes effect when the application "
                               "has been halted by the debugger (i.e. a breakpoint has "
                               "been activated or the interrupt was pressed).") );


    mainWindow()->setViewAvailable(variableWidget, true);
    mainWindow()->setViewAvailable(framestackWidget, true);
    mainWindow()->setViewAvailable(disassembleWidget, true);
    mainWindow()->setViewAvailable(gdbOutputWidget, true);

    variableWidget->setEnabled(true);
    framestackWidget->setEnabled(true);
    disassembleWidget->setEnabled(true);

    gdbOutputWidget->clear();
    gdbOutputWidget->setEnabled(true);

    controller->slotStart(shell, program);

    //This sets the breakpoints again, so when you remove them during
    //a debugging session, the app will still stop at those breakpoints...
    //    breakpointWidget->slotSetPendingBPs();
}

void DebuggerPart::stopDebugger()
{
    controller->slotStop();
    debugger()->clearExecutionPoint();

    delete floatingToolBar;
    floatingToolBar = 0;

    breakpointWidget->reset();
    framestackWidget->clear();
    variableWidget->clear();
    disassembleWidget->clear();
    disassembleWidget->slotActivate(false);

    variableWidget->setEnabled(false);
    framestackWidget->setEnabled(false);
    disassembleWidget->setEnabled(false);
    gdbOutputWidget->setEnabled(false);

    mainWindow()->setViewAvailable(variableWidget, false);
    mainWindow()->setViewAvailable(framestackWidget, false);
    mainWindow()->setViewAvailable(disassembleWidget, false);
    mainWindow()->setViewAvailable(gdbOutputWidget, false);

    KActionCollection *ac = actionCollection();
    ac->action("debug_run")->setText( i18n("&Start") );
    ac->action("debug_run")->setIcon( "1rightarrow" );
    ac->action("debug_run")->setStatusText( i18n("Runs the program in the debugger") );
    ac->action("debug_run")->setWhatsThis( i18n("Start in debugger\n\n"
                               "Starts the debugger with the project's main "
                               "executable. You may set some breakpoints "
                               "before this, or you can interrupt the program "
                               "while it is running, in order to get information "
                               "about variables, frame stack, and so on.") );

    stateChanged( QString("stopped") );

    core()->running(this, false);
}

void DebuggerPart::slotRun()
{
    if( controller->stateIsOn( s_dbgNotStarted ) ) {
      mainWindow()->statusBar()->message(i18n("Debugging program"), 1000);
      mainWindow()->raiseView(gdbOutputWidget);
      startDebugger();
    } else {
      mainWindow()->statusBar()->message(i18n("Continuing program"), 1000);
    }
    controller->slotRun();
}


void DebuggerPart::slotExamineCore()
{
    mainWindow()->statusBar()->message(i18n("Choose a core file to examine..."), 1000);

    QString dirName = project()? project()->projectDirectory() : QDir::homeDirPath();
    QString coreFile = KFileDialog::getOpenFileName(dirName);
    if (coreFile.isNull())
        return;

    mainWindow()->statusBar()->message(i18n("Examining core file %1").arg(coreFile), 1000);

    startDebugger();
    controller->slotCoreFile(coreFile);
}


void DebuggerPart::slotAttachProcess()
{
    mainWindow()->statusBar()->message(i18n("Choose a process to attach to..."), 1000);

    Dbg_PS_Dialog dlg;
    if (!dlg.exec() || !dlg.pidSelected())
        return;

    int pid = dlg.pidSelected();
    mainWindow()->statusBar()->message(i18n("Attaching to process %1").arg(pid), 1000);

    startDebugger();
    controller->slotAttachTo(pid);
}


void DebuggerPart::slotStop()
{
    if( !controller->stateIsOn( s_dbgNotStarted ) && !controller->stateIsOn( s_shuttingDown ) )
        stopDebugger();
}


void DebuggerPart::slotPause()
{
    controller->slotBreakInto();
}


void DebuggerPart::slotRunToCursor()
{
    KParts::ReadWritePart *rwpart
        = dynamic_cast<KParts::ReadWritePart*>(partController()->activePart());
    KTextEditor::ViewCursorInterface *cursorIface
        = dynamic_cast<KTextEditor::ViewCursorInterface*>(partController()->activeWidget());

    if (!rwpart || !rwpart->url().isLocalFile() || !cursorIface)
        return;

    uint line, col;
    cursorIface->cursorPosition(&line, &col);

    controller->slotRunUntil(rwpart->url().path(), line);
}

void DebuggerPart::slotStepOver()
{
    controller->slotStepOver();
}


void DebuggerPart::slotStepOverInstruction()
{
    controller->slotStepOver();
}


void DebuggerPart::slotStepIntoInstruction()
{
    controller->slotStepIntoIns();
}


void DebuggerPart::slotStepInto()
{
    controller->slotStepInto();
}


void DebuggerPart::slotStepOut()
{
    controller->slotStepOutOff();
}


void DebuggerPart::slotMemoryView()
{
    // Hmm, couldn't this be made non-modal?

    MemoryViewDialog *dlg = new MemoryViewDialog();
    connect( dlg,        SIGNAL(disassemble(const QString&, const QString&)),
             controller, SLOT(slotDisassemble(const QString&, const QString&)));
    connect( dlg,        SIGNAL(memoryDump(const QString&, const QString&)),
             controller, SLOT(slotMemoryDump(const QString&, const QString&)));
    connect( dlg,        SIGNAL(registers()),
             controller, SLOT(slotRegisters()));
    connect( dlg,        SIGNAL(libraries()),
             controller, SLOT(slotLibraries()));

    connect( controller, SIGNAL(rawGDBMemoryDump(char*)),
             dlg,        SLOT(slotRawGDBMemoryView(char*)));
    connect( controller, SIGNAL(rawGDBDisassemble(char*)),
             dlg,        SLOT(slotRawGDBMemoryView(char*)));
    connect( controller, SIGNAL(rawGDBRegisters(char*)),
             dlg,        SLOT(slotRawGDBMemoryView(char*)));
    connect( controller, SIGNAL(rawGDBLibraries(char*)),
             dlg,        SLOT(slotRawGDBMemoryView(char*)));

    dlg->exec();
    delete dlg;
}


void DebuggerPart::slotRefreshBPState( const Breakpoint& BP)
{
    if (BP.isActionDie())
        debugger()->setBreakpoint(BP.fileName(), BP.lineNum()-1,
                              -1, true, false);
    else
        debugger()->setBreakpoint(BP.fileName(), BP.lineNum()-1,
                              1/*BP->id()*/, BP.isEnabled(), BP.isPending() );
}


void DebuggerPart::slotStatus(const QString &msg, int state)
{
    QString stateIndicator;

    if (state & s_dbgNotStarted) {
        stateIndicator = " ";
    } else if (state & s_appBusy) {
        stateIndicator = "A";
        debugger()->clearExecutionPoint();
        stateChanged( QString("active") );
    } else if (state & s_programExited) {
        stateIndicator = "E";
        slotStop();
    } else {
        stateIndicator = "P";
        stateChanged( QString("paused") );
    }

    // And now? :-)
    kdDebug(9012) << "Debugger state: " << stateIndicator << ": " << endl;
    kdDebug(9012) << "   " << msg << endl;

    statusBarIndicator->setText(stateIndicator);
    if (!msg.isEmpty())
        mainWindow()->statusBar()->message(msg, 3000);
}


void DebuggerPart::slotShowStep(const QString &fileName, int lineNum)
{
    // Debugger counts lines from 1
    debugger()->gotoExecutionPoint(fileName, lineNum-1);
}


void DebuggerPart::slotGotoSource(const QString &fileName, int lineNum)
{
    partController()->editDocument(fileName, lineNum);
}


void DebuggerPart::slotActivePartChanged( KParts::Part* part )
{
  KAction* action = actionCollection()->action("debug_toggle_breakpoint");
  if(!action)
      return;
  if(!part) {
      action->setEnabled(false);
      return;
  }
  KTextEditor::ViewCursorInterface *iface
      = dynamic_cast<KTextEditor::ViewCursorInterface*>(part->widget());
  action->setEnabled( iface != 0 );
}

void DebuggerPart::restorePartialProjectSession(const QDomElement* el)
{
  QDomElement generalEl = el->namedItem("general").toElement();
  if (!generalEl.isNull()) {
    bool enableFloatingToolBar = bool(generalEl.attribute( "dbgToolbar", "0").toInt());
    if (enableFloatingToolBar) {
      floatingToolBar = new DbgToolBar(this, mainWindow()->main());
      floatingToolBar->show();
    }
  }

  QDomElement breakpointListEl = el->namedItem("breakpointList").toElement();
  if (!breakpointListEl.isNull()) {
    QDomElement breakpointEl;
    int id;
    for (breakpointEl = breakpointListEl.firstChild().toElement(), id = 10000;
         !breakpointEl.isNull();
         breakpointEl = breakpointEl.nextSibling().toElement(), id++)
    {
      // read from session
      QString fileName = breakpointEl.attribute( "fileName", "");
      if (fileName.isEmpty()) {
        continue;
      }
      int lineNum = breakpointEl.attribute( "lineNum", "0").toInt();
      bool bEnabled = bool(breakpointEl.attribute( "enabled", "1").toInt());
      int dbgId = breakpointEl.attribute( "dbgId", "-1").toInt();
      QString condition = breakpointEl.attribute( "condition", "");

      // recreate breakpoint
      breakpointWidget->slotToggleBreakpoint( fileName, lineNum );
      if (!bEnabled) {
        breakpointWidget->slotToggleBreakpointEnabled( fileName, lineNum );
      }

      // make it visible in the editor view
      debugger()->setBreakpoint(fileName, lineNum,
                                0, // hack: the real dbgId is -1 but we need to get it set. 0 does no harm
                                bEnabled, true);

      QPtrList<Breakpoint> bpList = breakpointWidget->breakpoints();
      QPtrListIterator<Breakpoint> it(bpList);
      for (; it.current(); ++it) {
        Breakpoint* bp = it.current();
        if (bp->fileName() == fileName && (bp->lineNum()-1) == lineNum) {
          bp->setConditional(condition);
          bp->setDbgId(dbgId);
          break;
        }
      }
    }
  }
}

void DebuggerPart::savePartialProjectSession(QDomElement* el)
{
  QDomDocument domDoc = el->ownerDocument();
  if (domDoc.isNull()) { return; }

  if (floatingToolBar) {
    // (hmm...shouldn't this be written to the application config file (gideonrc)?
    //  otherwise you would need to set it again and again in every project)
    QDomElement generalEl = domDoc.createElement("general");
    generalEl.setAttribute("dbgToolbar", true);
    el->appendChild(generalEl);
  }

  QDomElement breakpointListEl = domDoc.createElement("breakpointList");
  QPtrList<Breakpoint> bpList = breakpointWidget->breakpoints();
  QPtrListIterator<Breakpoint> it(bpList);
  for (int i = 0; it.current(); ++it, i++) {
    Breakpoint* bp = it.current();
    QString bpIndex;
    bpIndex.setNum(i);
    QDomElement breakpointEl = domDoc.createElement("breakpoint"+bpIndex);

    breakpointEl.setAttribute("fileName", bp->fileName());
    breakpointEl.setAttribute("lineNum", bp->lineNum() - 1); // why -1?
    breakpointEl.setAttribute("enabled", bp->isEnabled());
    breakpointEl.setAttribute("condition", bp->conditional());
    breakpointEl.setAttribute("dbgId", bp->dbgId());

    breakpointListEl.appendChild(breakpointEl);
  }
  if (!breakpointListEl.isNull()) {
    el->appendChild(breakpointListEl);
  }
}

}

#include "debuggerpart.moc"

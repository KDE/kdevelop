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

#include "debuggerpart.h"

#include <qdir.h>
#include <qvbox.h>
#include <qwhatsthis.h>
#include <qpopupmenu.h>

#include <kaction.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmainwindow.h>
#include <kstatusbar.h>
#include <kgenericfactory.h>

#include "kdevcore.h"
#include "kdevproject.h"
#include "kdevtoplevel.h"
#include "kdevappfrontend.h"
#include "kdevpartcontroller.h"
#include "kdevdebugger.h"
#include "domutil.h"
#include "variablewidget.h"
#include "breakpointwidget.h"
#include "framestackwidget.h"
#include "disassemblewidget.h"
#include "gdbcontroller.h"
#include "breakpoint.h"
#include "dbgpsdlg.h"
#include "dbgtoolbar.h"
#include "memviewdlg.h"
#include "gdbparser.h"
#include "debuggerconfigwidget.h"


typedef KGenericFactory<DebuggerPart> DebuggerFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevdebugger, DebuggerFactory( "kdevdebugger" ) );

DebuggerPart::DebuggerPart( QObject *parent, const char *name, const QStringList & )
    : KDevPlugin( parent, name ),
      controller(0), m_editorContext(KURL(), 0, "", 0)
{
    setInstance(DebuggerFactory::instance());
    
    setXMLFile("kdevdebugger.rc");

    statusBarIndicator = new QLabel(" ", topLevel()->statusBar());
    statusBarIndicator->setFixedWidth(15);
    topLevel()->statusBar()->addWidget(statusBarIndicator, 0, true);
    
    //
    // Setup widgets and dbgcontroller
    //
    
    variableWidget = new VariableWidget();
    variableWidget->setEnabled(false);
    variableWidget->setIcon(SmallIcon("brace"));
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
    topLevel()->embedSelectView(variableWidget, i18n("Watch"));
    
    breakpointWidget = new BreakpointWidget();
    breakpointWidget->setCaption(i18n("Breakpoint List"));
    QWhatsThis::add(breakpointWidget, i18n("Breakpoint list\n\n"
                                           "Displays a list of breakpoints with "
                                           "their current status. Clicking on a "
                                           "breakpoint item with the RMB displays "
                                           "a popupmenu so you may manipulate the "
                                           "breakpoint. Double clicking will take you "
                                           "to the source in the editor window."));
    topLevel()->embedOutputView(breakpointWidget, i18n("&Breakpoints"));
    
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
    topLevel()->embedOutputView(framestackWidget, i18n("&Frame Stack"));
    
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
    topLevel()->embedOutputView(disassembleWidget, i18n("Disassemble"));
    
    VariableTree *variableTree = variableWidget->varTree();

    // variableTree -> framestackWidget
    connect( variableTree,     SIGNAL(selectFrame(int, int)),
             framestackWidget, SLOT(slotSelectFrame(int, int)));
    
    // breakpointWidget -> this
    connect( breakpointWidget, SIGNAL(refreshBPState(Breakpoint*)),
             this,             SLOT(slotRefreshBPState(Breakpoint*)));
    connect( breakpointWidget, SIGNAL(publishBPState(Breakpoint*)),
             this,             SLOT(slotRefreshBPState(Breakpoint*)));
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

    action = new KAction(i18n("Examine Core File"), "core", 0,
                         this, SLOT(slotExamineCore()),
                         actionCollection(), "debug_core");
    action->setStatusText( i18n("Loads a core file into the debugger") );
    action->setWhatsThis( i18n("Examine core file\n\n"
                               "This loads a core file, which is typically created "
                               "after the application has crashed e.g. with a "
                               "segmentation fault. The core file contains an "
                               "image of the program memory at the time it crashed, "
                               "allowing you a post-mortem analysis.") );

    action = new KAction(i18n("Attach to Process"), "connect_creating", 0,
                         this, SLOT(slotAttachProcess()),
                         actionCollection(), "debug_attach");
    action->setStatusText( i18n("Attaches the debugger to a running process") );

    action = new KAction(i18n("Sto&p"), "stop", 0,
                         this, SLOT(slotStop()),
                         actionCollection(), "debug_stop");
    action->setEnabled(false);
    action->setStatusText( i18n("Kills the executable and exits the debugger") );

    action = new KAction(i18n("Interrupt"), "player_pause", 0,
                         this, SLOT(slotPause()),
                         actionCollection(), "debug_pause");
    action->setEnabled(false);
    action->setStatusText( i18n("Interrupts the application") );
    
    action = new KAction(i18n("&Continue"), "dbgrun", 0,
                         this, SLOT(slotContinue()),
                         actionCollection(), "debug_cont");
    action->setEnabled(false);
    action->setStatusText( i18n("Continues the application execution") );
    action->setWhatsThis( i18n("Continue application execution\n\n"
                               "Continues the execution of your application in the "
                               "debugger. This only has affect when the application "
                               "has been halted by the debugger (i.e. a breakpoint has "
                               "been activated or the interrupt was pressed).") );

    action = new KAction(i18n("Run to &Cursor"), "dbgrunto", 0,
                         this, SLOT(slotRunToCursor()),
                         actionCollection(), "debug_runtocursor");
    action->setEnabled(false);
    action->setStatusText( i18n("Continues execution until the cursor position is reached") );

    action = new KAction(i18n("Step &Over"), "dbgnext", 0,
                         this, SLOT(slotStepOver()),
                         actionCollection(), "debug_stepover");
    action->setEnabled(false);
    action->setStatusText( i18n("Steps over the next line") );
    action->setWhatsThis( i18n("Step over\n\n"
                               "Executes one line of source in the current source file. "
                               "If the source line is a call to a function the whole "
                               "function is executed and the app will stop at the line "
                               "following the function call.") );

    action = new KAction(i18n("Step over Ins&truction"), "dbgnextinst", 0,
                         this, SLOT(slotStepOverInstruction()),
                         actionCollection(), "debug_stepoverinst");
    action->setEnabled(false);
    action->setStatusText( i18n("Steps over the next assembly instruction") );

    action = new KAction(i18n("Step &Into"), "dbgstep", 0,
                         this, SLOT(slotStepInto()),
                         actionCollection(), "debug_stepinto");
    action->setEnabled(false);
    action->setStatusText( i18n("Steps into the next statement") );
    action->setWhatsThis( i18n("Step into\n\n"
                               "Executes exactly one line of source. If the source line "
                               "is a call to a function then execution will stop after "
                               "the function has been entered.") );


    action = new KAction(i18n("Step into I&nstruction"), "dbgstepinst", 0,
                         this, SLOT(slotStepIntoInstruction()),
                         actionCollection(), "debug_stepintoinst");
    action->setEnabled(false);
    action->setStatusText( i18n("Steps into the next assembly instruction") );

    action = new KAction(i18n("Step O&ut"), "dbgstepout", 0,
                         this, SLOT(slotStepOut()),
                         actionCollection(), "debug_stepout");
    action->setEnabled(false);
    action->setStatusText( i18n("Steps out of the current function") );
    action->setWhatsThis( i18n("Step out of\n\n"
                               "Executes the application until the currently executing "
                               "function is completed. The debugger will then display "
                               "the line after the original call to that function. If "
                               "we are in the outermost frame (i.e. in main()), then "
                               "this operation has no affect.") );

    action = new KAction(i18n("Viewers"), "dbgmemview", 0,
                         this, SLOT(slotMemoryView()),
                         actionCollection(), "debug_memview");
    action->setEnabled(false);
    action->setStatusText( i18n("Various views into the application") );
    
    connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
             this, SLOT(projectConfigWidget(KDialogBase*)) );
    connect( core(), SIGNAL(toggledBreakpoint(const QString &, int)),
             breakpointWidget, SLOT(slotToggleBreakpoint(const QString &, int)) );
    connect( core(), SIGNAL(editedBreakpoint(const QString &, int)),
             breakpointWidget, SLOT(slotEditBreakpoint(const QString &, int)) );
    connect( core(), SIGNAL(toggledBreakpointEnabled(const QString &, int)),
             breakpointWidget, SLOT(slotToggleBreakpointEnabled(const QString &, int)) );

    connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
	     this, SLOT(fillContextMenu(QPopupMenu *, const Context *)));
}


DebuggerPart::~DebuggerPart()
{
    if (variableWidget)
        topLevel()->removeView(variableWidget);
    if (breakpointWidget)
        topLevel()->removeView(breakpointWidget);
    if (framestackWidget)
        topLevel()->removeView(framestackWidget);
    if (disassembleWidget)
        topLevel()->removeView(disassembleWidget);
    
    delete variableWidget;
    delete breakpointWidget;
    delete framestackWidget;
    delete disassembleWidget;
    delete controller;
    delete floatingToolBar;
    delete statusBarIndicator;

    GDBParser::destroy();
}


void DebuggerPart::fillContextMenu(QPopupMenu *popup, const Context *context)
{
  if (!context->hasType("editor"))
    return;

  m_editorContext = *((EditorContext*)(context));

  popup->insertSeparator();
  popup->insertItem(i18n("Toggle Breakpoint"), this, SLOT(toggleBreakpoint()));
}


void DebuggerPart::toggleBreakpoint()
{
  breakpointWidget->slotToggleBreakpoint(m_editorContext.url().path(), m_editorContext.line());
}


void DebuggerPart::projectConfigWidget(KDialogBase *dlg)
{
    QVBox *vbox = dlg->addVBoxPage(i18n("Debugger"));
    DebuggerConfigWidget *w = new DebuggerConfigWidget(*projectDom(), vbox, "debugger config widget");
    connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
}


void DebuggerPart::setupController()
{
    VariableTree *variableTree = variableWidget->varTree();

    controller = new GDBController(variableTree, framestackWidget, *projectDom());

    // variableTree -> controller
    connect( variableTree,     SIGNAL(expandItem(VarItem*)),
             controller,       SLOT(slotExpandItem(VarItem*)));
    connect( variableTree,     SIGNAL(expandUserItem(VarItem*, const QCString&)),
             controller,       SLOT(slotExpandUserItem(VarItem*, const QCString&)));
    connect( variableTree,     SIGNAL(setLocalViewState(bool)),
             controller,       SLOT(slotSetLocalViewState(bool)));

    // framestackWidget -> controller
    connect( framestackWidget, SIGNAL(selectFrame(int)),
             controller,       SLOT(slotSelectFrame(int)));

    // breakpointWidget -> controller
    connect( breakpointWidget, SIGNAL(clearAllBreakpoints()),
             controller,       SLOT(slotClearAllBreakpoints()));

    // disassembleWidget -> controller
    connect( disassembleWidget,SIGNAL(disassemble(const QString&, const QString&)),
             controller,       SLOT(slotDisassemble(const QString&, const QString&)));

    // controller -> breakpointWidget
    connect( controller,       SIGNAL(acceptPendingBPs()),
             breakpointWidget, SLOT(slotSetPendingBPs()));
    connect( controller,       SIGNAL(unableToSetBPNow(int)),
             breakpointWidget, SLOT(slotUnableToSetBPNow(int)));
    connect( controller,       SIGNAL(rawGDBBreakpointList (char*)),
             breakpointWidget, SLOT(slotParseGDBBrkptList(char*)));
    connect( controller,       SIGNAL(rawGDBBreakpointSet(char*, int)),
             breakpointWidget, SLOT(slotParseGDBBreakpointSet(char*, int)));
    connect( breakpointWidget, SIGNAL(publishBPState(Breakpoint*)),
             controller,       SLOT(slotBPState(Breakpoint*)));

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
             this,             SLOT(slotApplReceivedStdout(const char*)));
    connect( controller,       SIGNAL(ttyStderr(const char*)),
             this,             SLOT(slotApplReceivedStderr(const char*)));
}


void DebuggerPart::startDebugger()
{
    core()->running(this, true);

    KActionCollection *ac = actionCollection();
    ac->action("debug_stop")->setEnabled(true);
    ac->action("debug_pause")->setEnabled(true);
    ac->action("debug_cont")->setEnabled(true);
    ac->action("debug_runtocursor")->setEnabled(true);
    ac->action("debug_stepover")->setEnabled(true);
    ac->action("debug_stepoverinst")->setEnabled(true);
    ac->action("debug_stepinto")->setEnabled(true);
    ac->action("debug_stepintoinst")->setEnabled(true);
    ac->action("debug_stepout")->setEnabled(true);
    ac->action("debug_memview")->setEnabled(true);

    variableWidget->setEnabled(true);
    framestackWidget->setEnabled(true);
    disassembleWidget->setEnabled(true);
    
    bool enableFloatingToolBar = DomUtil::readBoolEntry(*projectDom(), "/kdevdebugger/general/floatingtoolbar");

    if (enableFloatingToolBar) {
        floatingToolBar = new DbgToolBar(this, topLevel()->main());
        floatingToolBar->show();
    }

    setupController();
    QString program;
    if (project())
        program = project()->projectDirectory() + "/" + project()->mainProgram();
    controller->slotStart(program);
    breakpointWidget->slotSetPendingBPs();
}


void DebuggerPart::slotRun()
{
    if (controller)
        slotStop();

    topLevel()->statusBar()->message(i18n("Debugging program"), 1000);
    
    startDebugger();
    controller->slotRun();
}


void DebuggerPart::slotExamineCore()
{
    if (controller)
        slotStop();

    topLevel()->statusBar()->message(i18n("Choose a core file to examine..."), 1000);

    QString dirName = project()? project()->projectDirectory() : QDir::homeDirPath();
    QString coreFile = KFileDialog::getOpenFileName(dirName);
    if (coreFile.isNull())
        return;

    topLevel()->statusBar()->message(i18n("Examining core file %1").arg(coreFile), 1000);

    startDebugger();
    controller->slotCoreFile(coreFile);
}


void DebuggerPart::slotAttachProcess()
{
    if (controller)
        slotStop();

    topLevel()->statusBar()->message(i18n("Choose a process to attach to..."), 1000);

    Dbg_PS_Dialog dlg;
    if (!dlg.exec() || !dlg.pidSelected())
        return;

    int pid = dlg.pidSelected();
    topLevel()->statusBar()->message(i18n("Attaching to process %1").arg(pid), 1000);
    
    startDebugger();
    controller->slotAttachTo(pid);
}


void DebuggerPart::slotStop()
{
    core()->running(this, false);

    KActionCollection *ac = actionCollection();
    ac->action("debug_stop")->setEnabled(false);
    ac->action("debug_pause")->setEnabled(false);
    ac->action("debug_cont")->setEnabled(false);
    ac->action("debug_runtocursor")->setEnabled(false);
    ac->action("debug_stepover")->setEnabled(false);
    ac->action("debug_stepoverinst")->setEnabled(false);
    ac->action("debug_stepinto")->setEnabled(false);
    ac->action("debug_stepintoinst")->setEnabled(false);
    ac->action("debug_stepout")->setEnabled(false);
    ac->action("debug_memview")->setEnabled(false);

    variableWidget->setEnabled(false);
    framestackWidget->setEnabled(false);
    disassembleWidget->setEnabled(false);

    breakpointWidget->reset();
    framestackWidget->clear();
    variableWidget->clear();
    disassembleWidget->clear();
    disassembleWidget->slotActivate(false);

    debugger()->clearExecutionPoint();
    delete floatingToolBar;
    floatingToolBar = 0;

}


void DebuggerPart::slotPause()
{
    controller->slotBreakInto();
}


void DebuggerPart::slotContinue()
{
    controller->slotRun();
}


void DebuggerPart::slotRunToCursor()
{
    QString fileName;
    int lineNum = 0;
    // FIXME: Find out current file name and line number
    controller->slotRunUntil(fileName, lineNum);
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


void DebuggerPart::slotRefreshBPState(Breakpoint *BP)
{
    if (BP->isActionDie())
        debugger()->setBreakpoint(BP->fileName(), BP->lineNum()-1,
                              -1, true, false);
    else 
        debugger()->setBreakpoint(BP->fileName(), BP->lineNum()-1,
                              1/*BP->id()*/, BP->isEnabled(), BP->isPending() );
}


void DebuggerPart::slotStatus(const QString &msg, int state)
{
    QString stateIndicator("P");    // default to "paused"
    
    if (state & s_appBusy) {
        stateIndicator = "A";
        debugger()->clearExecutionPoint();
    }
    
    if (state & (s_dbgNotStarted|s_appNotStarted))
        stateIndicator = " ";
    
    if (state & s_programExited) {
        stateIndicator = "E";
        debugger()->clearExecutionPoint();
    }
    
    // And now? :-)
    kdDebug(9012) << "Debugger state: " << stateIndicator << endl;
    statusBarIndicator->setText(stateIndicator);

    if (!msg.isEmpty())
        topLevel()->statusBar()->message(msg, 1000);
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


void DebuggerPart::slotApplReceivedStdout(const char *buf)
{
    appFrontend()->insertStdoutLine(QString::fromLatin1(buf));
}


void DebuggerPart::slotApplReceivedStderr(const char *buf)
{
    appFrontend()->insertStderrLine(QString::fromLatin1(buf));
}

#include "debuggerpart.moc"

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

#include "javadebuggerpart.h"

#include <qdir.h>
#include <qwhatsthis.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kaction.h>
#include <kstatusbar.h>

#include "kdevplugininfo.h"
#include "kdevcore.h"
#include "kdevproject.h"
#include "kdevmainwindow.h"
#include "kdevpartcontroller.h"
#include "kdevdebugger.h"
#include "debugger.h"
#include "kdevappfrontend.h"

#include "variablewidget.h"
#include "breakpointwidget.h"
#include "framestackwidget.h"
#include "disassemblewidget.h"
#include "jdbcontroller.h"
#include "breakpoint.h"
#include "dbgpsdlg.h"
#include "memviewdlg.h"


namespace JAVADebugger
{
static const KDevPluginInfo data("kdevjavadebugger");
K_EXPORT_COMPONENT_FACTORY( libkdevjavadebugger, JavaDebuggerFactory( data ) )

JavaDebuggerPart::JavaDebuggerPart(QObject *parent, const char *name, const QStringList &)
    : KDevPlugin(&data, parent, name ? name : "JavaDebuggerPart"),
      controller(0)
{
    setInstance(JavaDebuggerFactory::instance());

    setXMLFile("kdevjavadebugger.rc");

    m_debugger = new Debugger( partController() );
    
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
                                         "debugger. Click the right mouse button on items in "
                                         "this view to get a popup menu.\n"
                                         "To speed up stepping through your code "
                                         "leave the tree items closed and add the "
                                         "variable(s) to the watch section.\n"
                                         "To change a variable value in your "
                                         "running app use a watch variable (&eg; a=5)."));
    mainWindow()->embedSelectView(variableWidget, i18n("Watch"), i18n("debugger variable-view"));
    mainWindow()->setViewAvailable(variableWidget, false);

    breakpointWidget = new BreakpointWidget();
    breakpointWidget->setCaption(i18n("Breakpoint List"));
    QWhatsThis::add(breakpointWidget, i18n("Breakpoint list\n\n"
                                           "Displays a list of breakpoints with "
                                           "their current status. Clicking on a "
                                           "breakpoint item with the right mouse button displays "
                                           "a popup menu so you may manipulate the "
                                           "breakpoint. Double clicking will take you "
                                           "to the source in the editor window."));
    mainWindow()->embedOutputView(breakpointWidget, i18n("&Breakpoints"), i18n("debugger breakpoints"));

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
    mainWindow()->embedOutputView(framestackWidget, i18n("&Frame Stack"), i18n("debugger function call stack"));
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
    mainWindow()->embedOutputView(disassembleWidget, i18n("Disassemble"), i18n("debugger disassemble"));
    mainWindow()->setViewAvailable(disassembleWidget, false);

    VariableTree *variableTree = variableWidget->varTree();

    // variableTree -> framestackWidget
    connect( variableTree,     SIGNAL(selectFrame(int)),
             framestackWidget, SLOT(slotSelectFrame(int)));

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
                               "debugger. This only takes effect when the application "
                               "has been halted by the debugger (i.e. a breakpoint has "
                               "been activated or the interrupt was pressed).") );

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
                               "this operation has no effect.") );

    action = new KAction(i18n("Viewers"), "dbgmemview", 0,
                         this, SLOT(slotMemoryView()),
                         actionCollection(), "debug_memview");
    action->setEnabled(false);
    action->setStatusText( i18n("Various views into the application") );

    connect( core(), SIGNAL(toggledBreakpoint(const QString &, int)),
             breakpointWidget, SLOT(slotToggleBreakpoint(const QString &, int)) );
    connect( core(), SIGNAL(editedBreakpoint(const QString &, int)),
             breakpointWidget, SLOT(slotEditBreakpoint(const QString &, int)) );
    connect( core(), SIGNAL(toggledBreakpointEnabled(const QString &, int)),
             breakpointWidget, SLOT(slotToggleBreakpointEnabled(const QString &, int)) );
}


JavaDebuggerPart::~JavaDebuggerPart()
{
    mainWindow()->removeView(variableWidget);
    mainWindow()->removeView(breakpointWidget);
    mainWindow()->removeView(framestackWidget);
    mainWindow()->removeView(disassembleWidget);

    delete variableWidget;
    delete breakpointWidget;
    delete framestackWidget;
    delete disassembleWidget;
    delete controller;
    //    delete floatingToolBar;
}


void JavaDebuggerPart::setupController()
{
    VariableTree *variableTree = variableWidget->varTree();

    if (project()) {
		controller = new JDBController(variableTree, framestackWidget, project()->projectDirectory(), project()->mainProgram());
    } else {
		controller = new JDBController(variableTree, framestackWidget, "", "");
	}

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
    connect( controller,       SIGNAL(rawJDBBreakpointList (char*)),
             breakpointWidget, SLOT(slotParseJDBBrkptList(char*)));
    connect( controller,       SIGNAL(rawJDBBreakpointSet(char*, int)),
             breakpointWidget, SLOT(slotParseJDBBreakpointSet(char*, int)));
    connect( breakpointWidget, SIGNAL(publishBPState(Breakpoint*)),
             controller,       SLOT(slotBPState(Breakpoint*)));

    // controller -> disassembleWidget
    connect( controller,       SIGNAL(showStepInSource(const QString&, int, const QString&)),
             disassembleWidget,SLOT(slotShowStepInSource(const QString&, int, const QString&)));
    connect( controller,       SIGNAL(rawJDBDisassemble(char*)),
             disassembleWidget,SLOT(slotDisassemble(char*)));

    // controller -> this
    connect( controller,       SIGNAL(dbgStatus(const QString&, int)),
             this,             SLOT(slotStatus(const QString&, int)));
    connect( controller,       SIGNAL(showStepInSource(const QString&, int, const QString&)),
             this,             SLOT(slotShowStep(const QString&, int)));

    // Hmm, the application's output can be separated from the debugger's?
    // Didn't know that...
#if 0
    connect( controller,       SIGNAL(ttyStdout(const char*)),
             this,             SLOT(slotApplReceivedStdout(const char*)));
    connect( controller,       SIGNAL(ttyStderr(const char*)),
             this,             SLOT(slotApplReceivedStderr(const char*)));
#endif
}


void JavaDebuggerPart::startDebugger()
{
    core()->running(this, true);

    KActionCollection *ac = actionCollection();
    ac->action("debug_stop")->setEnabled(true);
    ac->action("debug_pause")->setEnabled(true);
    ac->action("debug_cont")->setEnabled(true);
    ac->action("debug_stepover")->setEnabled(true);
    ac->action("debug_stepinto")->setEnabled(true);
    ac->action("debug_stepintoinst")->setEnabled(true);
    ac->action("debug_stepout")->setEnabled(true);
    ac->action("debug_memview")->setEnabled(true);

    variableWidget->setEnabled(true);
    framestackWidget->setEnabled(true);
    disassembleWidget->setEnabled(true);

    mainWindow()->setViewAvailable(variableWidget, true);
    mainWindow()->setViewAvailable(framestackWidget, true);
    mainWindow()->setViewAvailable(disassembleWidget, true);

    // Floatinging tool bar can wait until later :-)
    //    if (enableFloatingToolBar) {
    //        floatingToolBar = new DbgToolBar(controller, this);
    //        floatingToolBar->show();
    //    }

    setupController();
    controller->slotStart("", "", "");
    breakpointWidget->slotSetPendingBPs();
}


void JavaDebuggerPart::slotRun()
{
    if (controller)
        slotStop();

    mainWindow()->statusBar()->message(i18n("Debugging program"));

    startDebugger();
    controller->slotRun();
}



void JavaDebuggerPart::slotStop()
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

    mainWindow()->setViewAvailable(variableWidget, false);
    mainWindow()->setViewAvailable(framestackWidget, false);
    mainWindow()->setViewAvailable(disassembleWidget, false);

    variableWidget->setEnabled(false);
    framestackWidget->setEnabled(false);
    disassembleWidget->setEnabled(false);

    breakpointWidget->reset();
    framestackWidget->clear();
    variableWidget->clear();
    disassembleWidget->clear();
    disassembleWidget->slotActivate(false);

    debugger()->clearExecutionPoint();
    //    delete floatingToolBar;
    //    floatingToolBar = 0;

}


void JavaDebuggerPart::slotPause()
{
    controller->slotBreakInto();
}


void JavaDebuggerPart::slotContinue()
{
    controller->slotRun();
}


void JavaDebuggerPart::slotStepOver()
{
    controller->slotStepOver();
}


void JavaDebuggerPart::slotStepIntoInstruction()
{
    controller->slotStepIntoIns();
}


void JavaDebuggerPart::slotStepInto()
{
    controller->slotStepInto();
}


void JavaDebuggerPart::slotStepOut()
{
    controller->slotStepOutOff();
}


void JavaDebuggerPart::slotMemoryView()
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

    connect( controller, SIGNAL(rawJDBMemoryDump(char*)),
             dlg,        SLOT(slotRawJDBMemoryView(char*)));
    connect( controller, SIGNAL(rawJDBDisassemble(char*)),
             dlg,        SLOT(slotRawJDBMemoryView(char*)));
    connect( controller, SIGNAL(rawJDBRegisters(char*)),
             dlg,        SLOT(slotRawJDBMemoryView(char*)));
    connect( controller, SIGNAL(rawJDBLibraries(char*)),
             dlg,        SLOT(slotRawJDBMemoryView(char*)));

    dlg->exec();
    delete dlg;
}


void JavaDebuggerPart::slotRefreshBPState(Breakpoint *BP)
{
    if (BP->isActionDie())
        debugger()->setBreakpoint(BP->fileName(), BP->lineNum()-1,
                              -1, true, false);
    else
        debugger()->setBreakpoint(BP->fileName(), BP->lineNum()-1,
                              1/*BP->id()*/, BP->isEnabled(), BP->isPending() );
}


void JavaDebuggerPart::slotStatus(const QString &msg, int state)
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

    if (!msg.isEmpty())
        mainWindow()->statusBar()->message(msg);
}


void JavaDebuggerPart::slotShowStep(const QString &fileName, int lineNum)
{
    // Debugger counts lines from 1
    debugger()->gotoExecutionPoint(KURL( fileName ), lineNum-1);
}


void JavaDebuggerPart::slotGotoSource(const QString &fileName, int lineNum)
{
    partController()->editDocument(KURL( fileName ), lineNum);
}

}

KDevDebugger * JAVADebugger::JavaDebuggerPart::debugger()
{
    return m_debugger;
}

KDevAppFrontend * JAVADebugger::JavaDebuggerPart::appFrontend( )
{
    return extension<KDevAppFrontend>("KDevelop/AppFrontend");
}

#include "javadebuggerpart.moc"

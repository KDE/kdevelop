#include "dbgmanager.h"

#include "dbgfactory.h"
#include "brkptmanager.h"

#include "kdevcomponent.h"
#include "KDevComponentManager.h"
#include "projectspace.h"
#include "kaction.h"
#include "kdebug.h"
#include "klocale.h"

DbgManager::DbgManager(QObject *parent, const char *name ) :
    KDevComponent(parent, name),
    m_BPManager(0)
{
    setInstance(DbgFactory::instance());
    setXMLFile("kdevelopdbgui.rc" );
}

DbgManager::~DbgManager()
{
    delete m_BPManager;
}

ProjectSpace* DbgManager::projectSpace(){
	return static_cast<ProjectSpace*>(componentManager()->component("ProjectSpace"));
}

void DbgManager::setupGUI()
{
    kdDebug(9001) << "Building GrepWidget" << endl;
    m_BPManager = new BreakpointManager(0, "Breakpoint Manager", 0);
    emit embedWidget(m_BPManager, OutputView, i18n("breakpoints"), i18n("breakpoint manager"));

    KAction *action;

    action = new KAction( i18n("&Start"), 0, this, SLOT( slotDebugStart() ), actionCollection(),"debug_start" );
    //action->setStatusText( i18n("") );
    //action->setWhatsThis( i18n("") );

  // Debug "Start (other)..." submenu
    action = new KAction( i18n("Examine core file"), "debugger", 0, this, SLOT( slotDebugExamineCore() ),
             actionCollection(),"debug_examine_core" );
  action = new KAction( i18n("Debug another executable"), "debugger", 0, this, SLOT( slotDebugNamedFile() ),
          actionCollection(), "debug_debug_other_exe");
  action = new KAction( i18n("Attach to process"), "debugger", 0, this, SLOT( slotDebugAttach() ),
          actionCollection(), "debug_attatch_to_process");
  action = new KAction( i18n("Debug with arguments"), "debugger", 0, this, SLOT( slotDebugExecuteWithArgs() ),
          actionCollection(), "debug_debug_with_args");
  // Separator
  action = new KAction( i18n("Run"), 0, this, SLOT( slotDebugRun() ), actionCollection(), "debug_run");
  action = new KAction( i18n("Run to cursor"), 0, this, SLOT( slotDebugRunToCursor() ), actionCollection(), "debug_run_to_cursor");
  action = new KAction( i18n("Step over"), 0, this, SLOT( slotDebugStepOver() ), actionCollection(), "debug_step_over");
  action = new KAction( i18n("Step over instr."), 0, this, SLOT( slotDebugStepOverInstr() ), actionCollection(), "debug_step_over_instr");
  action = new KAction( i18n("Step into"), 0, this, SLOT( slotDebugStepInto() ), actionCollection(), "debug_step_into");
  action = new KAction( i18n("Step into instr."), 0, this, SLOT( slotDebugStepIntoInstr() ), actionCollection(), "debug_step_into_instr");
  action = new KAction( i18n("Step out"), 0, this, SLOT( slotDebugStepOut() ), actionCollection(), "debug_step_out");
  // Separator
  action = new KAction( i18n("Interrupt"), 0, this, SLOT( slotDebugInterrupt() ), actionCollection(), "debug_interrupt");
  action = new KAction( i18n("Stop"), 0, this, SLOT( slotDebugStop() ), actionCollection(), "debug_stop");
}

void DbgManager::compilationAborted()
{}

void DbgManager::projectSpaceClosed()
{}

void DbgManager::projectSpaceOpened(ProjectSpace*)
{}

void DbgManager::slotDebugStart()
{}

void DbgManager::slotDebugExamineCore()
{}

void DbgManager::slotDebugNamedFile()
{}

void DbgManager::slotDebugAttach()
{}

void DbgManager::slotDebugExecuteWithArgs()
{}

void DbgManager::slotDebugRun()
{}

void DbgManager::slotDebugRunToCursor()
{}

void DbgManager::slotDebugStepOver()
{}

void DbgManager::slotDebugStepOverInstr()
{}

void DbgManager::slotDebugStepInto()
{}

void DbgManager::slotDebugStepIntoInstr()
{}

void DbgManager::slotDebugStepOut()
{}

void DbgManager::slotDebugInterrupt()
{}

void DbgManager::slotDebugStop()
{}

#include "dbgmanager.moc"

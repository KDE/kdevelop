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

#include <qwhatsthis.h>
#include <kdebug.h>
#include <klocale.h>
#include <kaction.h>

#include "kdevcore.h"
#include "debuggerfactory.h"
#include "debuggerpart.h"


DebuggerPart::DebuggerPart(KDevApi *api, QObject *parent, const char *name)
    : KDevPart(api, parent, name)
{
    setInstance(DebuggerFactory::instance());
    
    setXMLFile("kdevdebugger.rc");

    KAction *action;
    
    action = new KAction(i18n("&Continue"), "dbgrun", 0,
                         this, SLOT(slotContinue()),
                         actionCollection(), "debug_cont");
    action->setWhatsThis( i18n("Continue application execution\n\n"
                               "Continues the execution of your application in the "
                               "debugger. This only has affect when the application "
                               "has been halted by the debugger (i.e. a breakpoint has "
                               "been activated or the interrupt was pressed).") );

    action = new KAction(i18n("Run to &cursor"), "dbgrunto", 0,
                         this, SLOT(slotRunToCursor()),
                         actionCollection(), "debug_runtocursor");
    action->setWhatsThis( i18n("Step into\n\n"
                               "Executes exactly one line of source. If the source line "
                               "is a call to a function then execution will stop after "
                               "the function has been entered.") );

    action = new KAction(i18n("&Step over"), "dbgnext", 0,
                         this, SLOT(slotStepOver()),
                         actionCollection(), "debug_stepover");
    action->setWhatsThis( i18n("Step over\n\n"
                               "Executes one line of source in the current source file. "
                               "If the source line is a call to a function the whole "
                               "function is executed and the app will stop at the line "
                               "following the function call.") );

    action = new KAction(i18n("Step &over instruction"), "dbgnextinst", 0,
                         this, SLOT(slotStepOverInstruction()),
                         actionCollection(), "debug_stepoverinst");

    action = new KAction(i18n("Step &into"), "dbgstep", 0,
                         this, SLOT(slotStepInto()),
                         actionCollection(), "debug_stepinto");

    action = new KAction(i18n("Step into i&nstruction"), "dbgstepinst", 0,
                         this, SLOT(slotStepIntoInstruction()),
                         actionCollection(), "debug_stepintoinst");

    action = new KAction(i18n("Step &out"), "dbgstepout", 0,
                         this, SLOT(slotStepOut()),
                         actionCollection(), "debug_stepout");
    action->setWhatsThis( i18n("Step out of\n\n"
                               "Executes the application until the currently executing "
                               "function is completed. The debugger will then display "
                               "the line after the original call to that function. If "
                               "we are in the outermost frame (i.e. in main()), then "
                               "this operation has no affect.") );
}


DebuggerPart::~DebuggerPart()
{
}


void DebuggerPart::slotContinue()
{
}


void DebuggerPart::slotRunToCursor()
{
}


void DebuggerPart::slotStepOver()
{
}


void DebuggerPart::slotStepOverInstruction()
{
}


void DebuggerPart::slotStepIntoInstruction()
{
}


void DebuggerPart::slotStepInto()
{
}


void DebuggerPart::slotStepOut()
{
}

#include "debuggerpart.moc"

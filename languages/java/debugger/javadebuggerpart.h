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

#ifndef _DEBUGGERPART_H_
#define _DEBUGGERPART_H_

#include <qguardedptr.h>
#include "kdevplugin.h"

#include <kdevgenericfactory.h>

class KDevAppFrontend;
class KDevDebugger;

namespace JAVADebugger
{

class VariableWidget;
class BreakpointWidget;
class FramestackWidget;
class DisassembleWidget;
class Breakpoint;
class DbgController;
class DbgToolBar;


class JavaDebuggerPart : public KDevPlugin
{
    Q_OBJECT

public:
    JavaDebuggerPart( QObject *parent, const char *name, const QStringList & );
    ~JavaDebuggerPart();

private slots:
    void slotRun();
    void slotStop();
    void slotPause();
    void slotContinue();
    void slotStepOver();
    void slotStepIntoInstruction();
    void slotStepInto();
    void slotStepOut();
    void slotMemoryView();

    void slotRefreshBPState(Breakpoint *BP);
    void slotStatus(const QString &msg, int state);
    void slotShowStep(const QString &, int);
    void slotGotoSource(const QString &, int);

private:
    void startDebugger();
    void setupController();
    
    KDevAppFrontend *appFrontend();
    KDevDebugger *debugger();

    QGuardedPtr<VariableWidget> variableWidget;
    QGuardedPtr<BreakpointWidget> breakpointWidget;
    QGuardedPtr<FramestackWidget> framestackWidget;
    QGuardedPtr<DisassembleWidget> disassembleWidget;
    DbgController *controller;
    //    QGuardedPtr<DbgToolBar> floatingToolBar;
    
    KDevDebugger *m_debugger;
};

typedef KDevGenericFactory<JavaDebuggerPart> JavaDebuggerFactory;

}

#endif

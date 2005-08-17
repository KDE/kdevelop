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

#include <qpointer.h>
//Added by qt3to4:
#include <QLabel>
#include <Q3PopupMenu>
#include <Q3CString>
#include "kdevplugin.h"
#include "kdevcore.h"

#include "debuggerdcopinterface.h"

namespace KParts { class Part; }

class QLabel;
class Q3PopupMenu;
class KDialogBase;
class ProcessWidget;
class ProcessLineMaker;
class KDevAppFrontend;
class KDevDebugger;

namespace GDBDebugger
{

class GDBBreakpointWidget;
class FramestackWidget;
class DisassembleWidget;
class Breakpoint;
class DbgController;
class DbgToolBar;
class VariableWidget;
class GDBOutputWidget;

class DebuggerPart : public KDevPlugin, virtual public DebuggerDCOPInterface
{
    Q_OBJECT

public:
    DebuggerPart( QObject *parent, const char *name, const QStringList & );
    ~DebuggerPart();
    virtual void restorePartialProjectSession(const QDomElement* el);
    virtual void savePartialProjectSession(QDomElement* el);

k_dcop:
    virtual ASYNC slotDebugExternalProcess();

private slots:
    void setupDcop();
    void guiClientAdded(KXMLGUIClient*);
    void contextMenu(Q3PopupMenu *popup, const Context *context);
    void toggleBreakpoint();
    void contextWatch();
//    void projectOpened();
    void projectClosed();
    void projectConfigWidget(KDialogBase *dlg);
    void slotActivePartChanged(KParts::Part*);

    void slotRun();
    void slotExamineCore();
    void slotAttachProcess();
    void slotStopDebugger();
    void slotStop(KDevPlugin* which = 0);
    void slotPause();
    void slotRunToCursor();
    void slotStepOver();
    void slotStepOverInstruction();
    void slotStepIntoInstruction();
    void slotStepInto();
    void slotStepOut();
    void slotMemoryView();

    void slotRefreshBPState(const Breakpoint&);
    void slotStatus(const QString &msg, int state);
    void slotShowStep(const QString &fileName, int lineNum);
    void slotGotoSource(const QString &fileName, int lineNum);

    void slotDCOPApplicationRegistered(const Q3CString &appId);
    void slotCloseDrKonqi();

    //! There was an error running the debugger
    void errRunningDebugger( int errorCode );

private:
    KDevAppFrontend *appFrontend();
    KDevDebugger *debugger();
    
    bool attachProcess(int pid);
    bool startDebugger();
    void setupController();

    QPointer<VariableWidget> variableWidget;
    QPointer<GDBBreakpointWidget> gdbBreakpointWidget;
    QPointer<FramestackWidget> framestackWidget;
    QPointer<DisassembleWidget> disassembleWidget;
    QPointer<GDBOutputWidget> gdbOutputWidget;
    DbgController *controller;
    QPointer<QLabel> statusBarIndicator;
    QPointer<DbgToolBar> floatingToolBar;
    ProcessLineMaker* procLineMaker;
    ProcessLineMaker* gdbLineMaker;

    QString m_contextIdent;
    Q3CString m_drkonqi;
    
    KDevDebugger *m_debugger;
};

}

#endif

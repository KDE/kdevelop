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

#include <QPointer>
//Added by qt3to4:
#include <QByteArray>
#include <QLabel>
#include <Q3PopupMenu>
#include <kvbox.h>
#include "kdevplugin.h"
#include "kdevcore.h"

#include "gdbcontroller.h"
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
class GDBController;
class DbgToolBar;
class VariableWidget;
class GDBOutputWidget;
class ViewerWidget;

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
    virtual ASYNC slotDebugCommandLine(const QString& command);

private Q_SLOTS:
    void setupDcop();
    void guiClientAdded(KXMLGUIClient*);
    void contextMenu(Q3PopupMenu *popup, const Context *context);
    void toggleBreakpoint();
    void contextEvaluate();
    void contextWatch();
//    void projectOpened();
    void projectClosed();
    void projectConfigWidget(KDialogBase *dlg);
    void slotActivePartChanged(KParts::Part*);

    void slotRun();
    // Called to finish run operation in the case when we're
    // starting the debugger. Called either directly from
    // slotRun, if no rebuilding of project is needed, or
    // indirectly from project()->projectCompiled() after project
    // is rebuilt.
    void slotRun_part2();
    void slotRestart();
    void slotExamineCore();
    void slotAttachProcess();
    void slotStopDebugger();
    void slotStop(KDevPlugin* which = 0);
    void slotPause();
    void slotRunToCursor();
    void slotJumpToCursor();
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

    void slotDCOPApplicationRegistered(const QByteArray &appId);
    void slotCloseDrKonqi();

    // Hide or show the view that's the sender of this signal.
    void slotShowView(bool enabled);

    void slotDebuggerAbnormalExit();

    // Called when some file in the project was saved.
    // Sets 'needRebuild_' to true.
    void slotFileSaved();

    void slotProjectCompiled();

    void slotEvent(GDBController::event_t);

private:
    KDevAppFrontend *appFrontend();
    KDevDebugger *debugger();
    
    bool attachProcess(int pid);
    bool startDebugger();
    void setupController();
    bool haveModifiedFiles();

    QPointer<VariableWidget> variableWidget;
    QPointer<GDBBreakpointWidget> gdbBreakpointWidget;
    QPointer<FramestackWidget> framestackWidget;
    QPointer<DisassembleWidget> disassembleWidget;
    QPointer<GDBOutputWidget> gdbOutputWidget;
    QPointer<ViewerWidget> viewerWidget;
    GDBController *controller;
    QPointer<QLabel> statusBarIndicator;
    QPointer<DbgToolBar> floatingToolBar;
    ProcessLineMaker* procLineMaker;
    ProcessLineMaker* gdbLineMaker;

    QString m_contextIdent;
    QByteArray m_drkonqi;
    
    KDevDebugger *m_debugger;
    int previousDebuggerState_;
    // Set to true after each debugger restart
    // Currently used to auto-show variables view
    // on the first pause.
    bool justRestarted_;

    // Flag that specifies in project rebuild is necessary
    // before running the debugger. Set to 'true' in constructor
    // because we have no idea if project is 'dirty' or not
    // when it's opened, and then set to 'true' each time a file is
    // modified.    
    bool needRebuild_;

    // Set by 'startDebugger' and cleared by 'slotStopDebugger'.
    bool running_;

Q_SIGNALS:
    void buildProject();
};

}

#endif

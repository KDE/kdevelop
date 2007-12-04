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
#include <QByteArray>
#include <QLabel>
#include <QtCore/QVariant>

#include <KConfigGroup>
#include <KTextEditor/Cursor>

#include <iplugin.h>
#include <irunprovider.h>
#include <istatus.h>

#include "gdbcontroller.h"

class QLabel;
class QMenu;
class KDialog;
class ProcessWidget;

class KToolBar;
class KAction;

class BreakpointController;

namespace KDevelop {
class Context;
class ProcessLineMaker;
}

namespace GDBDebugger
{

class GDBBreakpointWidget;
class FramestackWidget;
class DisassembleWidget;
class Breakpoint;
class GDBController;
class VariableWidget;
class GDBOutputWidget;
class ViewerWidget;

class CppDebuggerPlugin : public KDevelop::IPlugin, public KDevelop::IRunProvider, public KDevelop::IStatus
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IRunProvider)
    Q_INTERFACES(KDevelop::IStatus)

public:
    CppDebuggerPlugin( QObject *parent, const QVariantList & = QVariantList() );
    ~CppDebuggerPlugin();

    BreakpointController* breakpoints() const;

    /**
     * Call this when something very interesting happens that the user
     * might be unaware of. It will make KDevelop's taskbar entry flash
     * if the application doesn't already have focus.
     * Typical use case: The debugger has stopped on a breakpoint.
     */
    void demandAttention() const;

    // BEGIN IRunProvider
    virtual QStringList instrumentorsProvided() const;
    virtual bool execute(const KDevelop::IRun& run, int serial);
    virtual void abort(int serial);

Q_SIGNALS:
    void finished(int serial);
    void output(int serial, const QString& line, KDevelop::IRunProvider::OutputTypes type);

public:
    // BEGIN IStatus
    virtual QString statusName() const;

Q_SIGNALS:
    void clearMessage();
    void showMessage(const QString & message, int timeout = 0);
    void hideProgress();
    void showProgress(int minimum, int maximum, int value);

    void raiseOutputViews();
    void raiseFramestackViews();
    void raiseVariableViews();
    void clearViews();

    void addWatchVariable(const QString& variable);
    void evaluateExpression(const QString& variable);

    void reset();

    void addMemoryView();

    void toggleBreakpoint(const KUrl& url, const KTextEditor::Cursor& cursor);

//k_dcop:
//    virtual ASYNC slotDebugExternalProcess();
//    virtual ASYNC slotDebugCommandLine(const QString& command);

private Q_SLOTS:
    void setupDcop();
    void guiClientAdded(KXMLGUIClient*);
    void contextMenu(QMenu *popup, const KDevelop::Context *context);
    void toggleBreakpoint();
    void contextEvaluate();
    void contextWatch();
//    void projectOpened();
    void projectClosed();

    void slotStartDebugger();
    void slotExamineCore();
    void slotAttachProcess();
    void slotStopDebugger();
    void slotRunToCursor();
    void slotJumpToCursor();

    void slotRefreshBPState(const Breakpoint&);

    void slotShowStep(const QString &fileName, int lineNum);
    void slotGotoSource(const QString &fileName, int lineNum);

    //void slotDCOPApplicationRegistered(const QByteArray &appId);
    void slotCloseDrKonqi();

    void slotDebuggerAbnormalExit();

    // Called when some file in the project was saved.
    // Sets 'needRebuild_' to true.
    void slotFileSaved();

    void slotProjectCompiled();

    void slotEvent(GDBController::event_t);

    void slotStateChanged(DBGStateFlags oldState, DBGStateFlags newState);

    void applicationStandardOutputLines(const QStringList& lines);
    void applicationStandardErrorLines(const QStringList& lines);

Q_SIGNALS:
    void startDebugger(const KDevelop::IRun & run, int serial);
    void stopDebugger();
    void attachTo(int pid);
    void coreFile(const QString& core);
    void runUntil(const KUrl& url, int line);
    void jumpTo(const KUrl& url, int line);

private:
    KConfigGroup config() const;

    void attachProcess(int pid);
    void setupController();
    void setupActions();

    GDBController *controller;
    //QPointer<QLabel> statusBarIndicator;
    QPointer<KToolBar> floatingToolBar;
    KDevelop::ProcessLineMaker* procLineMaker;
    KDevelop::ProcessLineMaker* gdbLineMaker;

    QString m_contextIdent;
    QByteArray m_drkonqi;

    //KDevDebugger *m_debugger;
    DBGStateFlags debuggerState_;
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

    class BreakpointListFactory* m_breakpointFactory;
    class VariableBreakpointListFactory* m_variableFactory;
    class FramestackViewFactory* m_framestackFactory;
    class DisassembleViewFactory* m_disassembleFactory;
    class GDBOutputViewFactory* m_outputFactory;
    class SpecialViewFactory* m_specialFactory;

    KConfigGroup m_config;
    BreakpointController* m_breakpointController;

    KAction* m_startDebugger;
    KAction* m_restartDebugger;
    KAction* m_stopDebugger;
    KAction* m_interruptDebugger;
    KAction* m_runToCursor;
    KAction* m_setToCursor;
    KAction* m_stepOver;
    KAction* m_stepIntoInstruction;
    KAction* m_stepInto;
    KAction* m_stepOverInstruction;
    KAction* m_stepOut;
    KAction* m_toggleBreakpoint;

    
Q_SIGNALS:
    void buildProject();
};

}

#endif

/*
 * GDB Debugger Support
 *
 * Copyright 1999-2001 John Birch <jbb@kdevelop.org>
 * Copyright 2001 by Bernd Gehrmann <bernd@kdevelop.org>
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

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
class QDBusInterface;
class KDialog;
class ProcessWidget;

class KToolBar;
class KAction;

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

    /**
     * Call this when something very interesting happens that the user
     * might be unaware of. It will make KDevelop's taskbar entry flash
     * if the application doesn't already have focus.
     * Typical use case: The debugger has stopped on a breakpoint.
     */
    void demandAttention() const;

    virtual KDevelop::ContextMenuExtension contextMenuExtension( KDevelop::Context* );

    //BEGIN IRunProvider
    virtual QStringList instrumentorsProvided() const;
    virtual QString translatedInstrumentor(const QString& instrumentor) const;
    virtual bool execute(const KDevelop::IRun& run, KJob* job);
    virtual void abort(KJob* job);

Q_SIGNALS:
    void finished(KJob* job);
    void output(KJob* job, const QString& line, KDevelop::IRunProvider::OutputTypes type);
    //END IRunProvider

public:
    //BEGIN IStatus
    virtual QString statusName() const;

Q_SIGNALS:
    void clearMessage();
    void showMessage(const QString & message, int timeout = 0);
    void hideProgress();
    void showProgress(int minimum, int maximum, int value);
    //END IStatus

    void raiseOutputViews();
    void raiseFramestackViews();
    void raiseVariableViews();
    void clearViews();

    void addWatchVariable(const QString& variable);
    void evaluateExpression(const QString& variable);

    void reset();

    void addMemoryView();

    void toggleBreakpoint(const KUrl& url, const KTextEditor::Cursor& cursor);

private Q_SLOTS:
    void setupDbus();
    void slotDebugExternalProcess();
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

    void slotGotoSource(const QString &fileName, int lineNum);

    void slotDbusApplicationRegistered(const QString &service);
    void slotCloseDrKonqi();

    void slotDebuggerAbnormalExit();

    void slotStateChanged(DBGStateFlags oldState, DBGStateFlags newState);

    void applicationStandardOutputLines(const QStringList& lines);
    void applicationStandardErrorLines(const QStringList& lines);

Q_SIGNALS:
    void startDebugger(const KDevelop::IRun & run, KJob* job);
    void stopDebugger();
    void attachTo(int pid);
    void coreFile(const QString& core);
    void runUntil(const KUrl& url, int line);
    void jumpTo(const KUrl& url, int line);

protected:
    virtual void initializeGuiState();

private:
    KConfigGroup config() const;

    void attachProcess(int pid);
    void setupController();
    void setupActions();

    GDBController *controller;
    QPointer<KToolBar> floatingToolBar;
    KDevelop::ProcessLineMaker* procLineMaker;
    KDevelop::ProcessLineMaker* gdbLineMaker;
    QDBusInterface* drkonqiInterface;

    QString m_contextIdent;
    QByteArray m_drkonqi;

    DBGStateFlags debuggerState_;
    // Set to true after each debugger restart
    // Currently used to auto-show variables view
    // on the first pause.
    bool justRestarted_;

    // Set by 'startDebugger' and cleared by 'slotStopDebugger'.
    bool running_;

    KConfigGroup m_config;

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
};

}

#endif

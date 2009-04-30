/*
 * GDB Debugger Support
 *
 * Copyright 1999-2001 John Birch <jbb@kdevelop.org>
 * Copyright 2001 by Bernd Gehrmann <bernd@kdevelop.org>
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2009 Niko Sams <niko.sams@gmail.com>
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


#ifndef GDB_DEBUGSESSION_H
#define GDB_DEBUGSESSION_H

#include <QPointer>

#include <KConfigGroup>

#include <debugger/interfaces/idebugsession.h>

#include "gdbglobal.h"

class KToolBar;

namespace KTextEditor {
class Cursor;
}
namespace KDevelop {
class ProcessLineMaker;
class ILaunchConfiguration;
}
namespace GDBDebugger {
class BreakpointController;
class GDBController;


class DebugSession : public KDevelop::IDebugSession
{
    Q_OBJECT
public:
    DebugSession(GDBController* controller);
    ~DebugSession();

    virtual DebuggerState state() const;
    virtual KDevelop::StackModel* stackModel() const;

    virtual bool restartAvaliable() const;

    GDBController* controller() { return m_controller; }

Q_SIGNALS:
    void applicationStandardOutputLines(const QStringList& lines);
    void applicationStandardErrorLines(const QStringList& lines);
    void showMessage(const QString&, int);
    void reset();
    void raiseOutputViews();
    void raiseFramestackViews();
    void raiseVariableViews();

public Q_SLOTS:
    bool startProgram(KDevelop::ILaunchConfiguration* run, KJob* job);
    virtual void startDebugger();
    virtual void restartDebugger();
    virtual void stopDebugger();
    virtual void interruptDebugger();
    virtual void run();
    virtual void runToCursor();
    virtual void jumpToCursor();
    virtual void stepOver();
    virtual void stepIntoInstruction();
    virtual void stepInto();
    virtual void stepOverInstruction();
    virtual void stepOut();

    void examineCoreFile(const KUrl& coreFile);
    void attachToProcess(int pid);

private Q_SLOTS:
    void slotDebuggerAbnormalExit();
    void gdbStateChanged(DBGStateFlags oldState, DBGStateFlags newState);
    void slotShowStepInSource(const QString& file, int line, const QString& address);

private:
    void setupController();
    void setSessionState(KDevelop::IDebugSession::DebuggerState state);

    GDBController *m_controller;
    KDevelop::ProcessLineMaker *m_procLineMaker;
    KDevelop::ProcessLineMaker *m_gdbLineMaker;
    DBGStateFlags m_gdbState;
    DebuggerState m_sessionState;
    bool justRestarted_;
    QPointer<KToolBar> floatingToolBar;
    KConfigGroup m_config;
};

}

#endif

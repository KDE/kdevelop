/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2009 Niko Sams <niko.sams@gmail.com>                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef KDEV_IDEBUGSESSION_H
#define KDEV_IDEBUGSESSION_H

#include <QtCore/QObject>
#include "../debuggerexport.h"

class KUrl;

namespace KDevelop {

class IVariableController;
class IBreakpointController;
class Breakpoint;
class StackModel;

class KDEVPLATFORMDEBUGGER_EXPORT IDebugSession : public QObject
{
    Q_OBJECT
    Q_ENUMS(DebuggerState)
public:
    IDebugSession();
    virtual ~IDebugSession();

    enum DebuggerState {
        NotStartedState,
        StartingState,
        ActiveState,
        PausedState,
        StoppingState,
        StoppedState,
        EndedState
    };

public:
    /**
     * Current state of the debug session
     */
    virtual DebuggerState state() const = 0;

    virtual StackModel* stackModel() const = 0;

    /**
     * Should return if restart is currently available
     */
    virtual bool restartAvaliable() const = 0;

    /**
     * Returns if the debugee is currently running. This includes paused.
     */
    bool isRunning() const;
    
    IBreakpointController *breakpointController();
    IVariableController *variableController();

public Q_SLOTS:
    virtual void restartDebugger() = 0;
    virtual void stopDebugger() = 0;
    virtual void interruptDebugger() = 0;
    virtual void run() = 0;
    virtual void runToCursor() = 0;
    virtual void jumpToCursor() = 0;
    virtual void stepOver() = 0;
    virtual void stepIntoInstruction() = 0;
    virtual void stepInto() = 0;
    virtual void stepOverInstruction() = 0;
    virtual void stepOut() = 0;

Q_SIGNALS:
    void stateChanged(KDevelop::IDebugSession::DebuggerState state);
    void showStepInSource(const KUrl& file, int line);
    void clearExecutionPoint();
    void finished();

    void raiseFramestackViews();

protected:

    IBreakpointController *m_breakpointController;
    IVariableController *m_variableController;
};

}

#endif

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
class KJob;

namespace KDevelop {
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
        StoppedState
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
     * Execution-Job that started the debug session.
     *
     * Can be 0 if it is eg a JIT debug session.
     */
    KJob* job() const;

    /**
     * Returns if the debugee is currently running. This includes paused.
     */
    bool isRunning() const;

public Q_SLOTS:
    virtual void startDebugger() = 0;
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
    virtual void toggleBreakpoint() = 0;

Q_SIGNALS:
    void stateChanged(KDevelop::IDebugSession::DebuggerState state);
    void showStepInSource(const KUrl& file, int line);
    void clearExecutionPoint();
    void finished();

protected:
    KJob *m_job;
};

}

#endif

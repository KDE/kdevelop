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

#ifndef KDEVPLATFORM_IDEBUGSESSION_H
#define KDEVPLATFORM_IDEBUGSESSION_H

#include <debugger/debuggerexport.h>

#include <QtCore/QObject>
#include <QtCore/QAbstractItemModel>
#include <QUrl>

namespace KDevelop {

class IVariableController;
class IBreakpointController;
class IFrameStackModel;
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

    enum event_t { program_state_changed = 1, 
                   program_exited, 
                   debugger_exited,
                   // Emitted when the thread or frame that is selected in UI
                   // changes.
                   thread_or_frame_changed, 
                   debugger_busy, 
                   debugger_ready,
                   // Raised when debugger believe that program start running.
                   // Can be used to hide current line indicator.
                   // Don't count on this being raise in all cases where
                   // program is running.
                   program_running,
                   // Raise when the debugger is in touch with the program,
                   // and should have access to its debug symbols. The program
                   // is not necessary running yet, or might already exited,
                   // or be otherwise dead.
                   connected_to_program
    };

public:
    /**
     * Current state of the debug session
     */
    virtual DebuggerState state() const = 0;

    /**
     * Should return if restart is currently available
     */
    virtual bool restartAvaliable() const = 0;

    /**
     * Returns if the debugee is currently running. This includes paused.
     */
    bool isRunning() const;
    
    /**
     * Returns the local Url for a source file used in the current debug session.
     *
     * The default implementation just returns the url and is sufficient for
     * local debuggers. Remote debuggers can implement a path mapping mechanism.
     */
    virtual QPair<QUrl, int> convertToLocalUrl(const QPair<QUrl, int> &remoteUrl) const;

    /**
     * Returns the remote Url for a source file used in the current debug session.
     *
     * The default implementation just returns the url and is sufficient for
     * local debuggers. Remote debuggers can implement a path mapping mechanism.
     */
    virtual QPair<QUrl, int> convertToRemoteUrl(const QPair<QUrl, int> &localUrl) const;

    /**
     * @return the breakpoint controller of this session
     *
     * @note Implementations must ensure that a breakpoint controller always exists (even if it
     * is a dummy stub implementation that does nothing), and that it does not change during
     * the lifetime of a session.
     */
    virtual IBreakpointController* breakpointController() const = 0;

    /**
     * @return the variable controller of this session
     *
     * @note Implementations must ensure that a variable controller always exists (even if it
     * is a dummy stub implementation that does nothing), and that it does not change during
     * the lifetime of a session.
     */
    virtual IVariableController* variableController() const = 0;

    /**
     * @return the frame stack model of this session
     *
     * @note Implementations must ensure that a frame stack model always exists (even if it
     * is a dummy stub implementation that does nothing), and that it does not change during
     * the lifetime of a session.
     */
    virtual IFrameStackModel* frameStackModel() const = 0;

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
    void showStepInSource(const QUrl& file, int line, const QString &addr);
    void showStepInDisassemble(const QString &addr);
    void clearExecutionPoint();
    void finished();

    void raiseFramestackViews();

    /** This signal is emitted whenever the given event in a program
        happens. See DESIGN.txt for expected handled of each event.

        NOTE: this signal should never be emitted directly. Instead,
        use raiseEvent.
    */
    void event(IDebugSession::event_t e);

public:
    using QObject::event; // prevent hiding of base method.

    QUrl currentUrl() const;
    int currentLine() const;
    QString currentAddr() const;
    
protected:

    // Clear the position before running code
    void clearCurrentPosition();
    /// Sets new position and emits showStepInSource or showStepInDisassemble (if source file is unavailable) signal
    void setCurrentPosition(const QUrl& url, int line, const QString& addr);

    /** Raises the specified event. Should be used instead of
        emitting 'event' directly, since this method can perform
        additional book-keeping for events.
        FIXME: it might make sense to automatically route
        events to all debugger components, as opposed to requiring
        that they connect to any signal.
    */
    virtual void raiseEvent(event_t e);
    friend class FrameStackModel;
    
private Q_SLOTS:
    void slotStateChanged(KDevelop::IDebugSession::DebuggerState state);

private: //TODO use d-pointer
    // Current position in debugged program, gets set when the state changes
    QUrl m_url;
    int m_line;
    QString m_addr;
};

}

#endif

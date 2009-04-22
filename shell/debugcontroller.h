/* This file is part of KDevelop
 *
 * Copyright 1999-2001 John Birch <jbb@kdevelop.org>
 * Copyright 2001 by Bernd Gehrmann <bernd@kdevelop.org>
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2009 Niko Sams <niko.sams@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef KDEV_DEBUGCONTROLLER_H
#define KDEV_DEBUGCONTROLLER_H

#include <QtCore/QVariantList>
#include <QtCore/QPointer>
#include <KDE/KXMLGUIClient>
#include <KDE/KTextEditor/MarkInterface>

#include "../interfaces/idebugcontroller.h"
#include "../debugger/interfaces/idebugsession.h"

class KAction;
namespace KParts {
    class Part;
}
namespace KDevelop {

class DebugController : public IDebugController, public KXMLGUIClient
{
    Q_OBJECT
public:
    DebugController(QObject *parent = 0);
    void initialize();

    virtual void addSession(IDebugSession* session);
    IDebugSession* currentSession();

    virtual ContextMenuExtension contextMenuExtension( Context* context );

private Q_SLOTS:
    void startDebugger();
    void restartDebugger();
    void stopDebugger();
    void interruptDebugger();
    void run();
    void runToCursor();
    void jumpToCursor();
    void stepOver();
    void stepIntoInstruction();
    void stepInto();
    void stepOverInstruction();
    void stepOut();
    void toggleBreakpoint();

    void debuggerStateChanged(KDevelop::IDebugSession::DebuggerState state);
    void showStepInSource(const KUrl &file, int line);
    void clearExecutionPoint();
    
    void partAdded(KParts::Part* part);


private:
    void setupActions();
    void updateDebuggerState(KDevelop::IDebugSession::DebuggerState state, KDevelop::IDebugSession* session);

    enum MarkType {
        BookmarkMark           = KTextEditor::MarkInterface::markType01,
        BreakpointMark         = KTextEditor::MarkInterface::markType02,
        ActiveBreakpointMark   = KTextEditor::MarkInterface::markType03,
        ReachedBreakpointMark  = KTextEditor::MarkInterface::markType04,
        DisabledBreakpointMark = KTextEditor::MarkInterface::markType05,
        ExecutionPointMark     = KTextEditor::MarkInterface::markType06
    };
    static const QPixmap* inactiveBreakpointPixmap();
    static const QPixmap* activeBreakpointPixmap();
    static const QPixmap* reachedBreakpointPixmap();
    static const QPixmap* disabledBreakpointPixmap();
    static const QPixmap* executionPointPixmap();

    KAction* m_startDebugger;
    KAction* m_restartDebugger;
    KAction* m_stopDebugger;
    KAction* m_interruptDebugger;
    KAction* m_runToCursor;
    KAction* m_jumpToCursor;
    KAction* m_stepOver;
    KAction* m_stepIntoInstruction;
    KAction* m_stepInto;
    KAction* m_stepOverInstruction;
    KAction* m_stepOut;
    KAction* m_toggleBreakpoint;

    QPointer<IDebugSession> m_currentSession;
};

}

#endif

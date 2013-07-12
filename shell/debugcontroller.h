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

#ifndef KDEVPLATFORM_DEBUGCONTROLLER_H
#define KDEVPLATFORM_DEBUGCONTROLLER_H

#include <QtCore/QVariantList>
#include <QWeakPointer>
#include <KDE/KXMLGUIClient>
#include <KDE/KTextEditor/MarkInterface>

#include "../interfaces/idebugcontroller.h"
#include "../debugger/interfaces/idebugsession.h"

class KAction;
namespace KParts {
    class Part;
}
namespace KDevelop {

class Context;
class ContextMenuExtension;

class DebugController : public IDebugController, public KXMLGUIClient
{
    Q_OBJECT
public:
    DebugController(QObject *parent = 0);
    ~DebugController();
    void initialize();
    void cleanup();

    /// Must be called by debugger plugin that needs debugger actions and toolviews.
    virtual void initializeUi();

    virtual void addSession(IDebugSession* session);
    virtual IDebugSession* currentSession();

    ContextMenuExtension contextMenuExtension( Context* context );

    virtual BreakpointModel* breakpointModel();
    virtual VariableCollection* variableCollection();

private Q_SLOTS:
    //void restartDebugger();
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

Q_SIGNALS:
    void raiseFramestackViews();

private:
    void setupActions();
    void updateDebuggerState(KDevelop::IDebugSession::DebuggerState state, KDevelop::IDebugSession* session);

    static const QPixmap* executionPointPixmap();

    KAction* m_continueDebugger;
    //KAction* m_restartDebugger;
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

    QWeakPointer<IDebugSession> m_currentSession;
    BreakpointModel *m_breakpointModel;
    VariableCollection *m_variableCollection;    
    
    bool m_uiInitialized;
};

}

#endif

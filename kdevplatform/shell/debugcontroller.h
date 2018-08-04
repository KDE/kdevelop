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

#include <QPointer>

#include <KXMLGUIClient>

#include "../interfaces/idebugcontroller.h"
#include "../debugger/interfaces/idebugsession.h"

class QAction;

namespace Sublime {
    class Area;
}

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
    explicit DebugController(QObject *parent = nullptr);
    ~DebugController() override;
    void initialize();
    void cleanup();

    /// Must be called by debugger plugin that needs debugger actions and tool views.
    void initializeUi() override;

    void addSession(IDebugSession* session) override;
    IDebugSession* currentSession() override;

    ContextMenuExtension contextMenuExtension(Context* context, QWidget* parent);

    BreakpointModel* breakpointModel() override;
    VariableCollection* variableCollection() override;

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
    void showCurrentLine();

    void debuggerStateChanged(KDevelop::IDebugSession::DebuggerState state);
    void showStepInSource(const QUrl &file, int line);
    void clearExecutionPoint();

    void partAdded(KParts::Part* part);
    void areaChanged(Sublime::Area* newArea);

Q_SIGNALS:
    void raiseFramestackViews();

private:
    void setupActions();
    void updateDebuggerState(KDevelop::IDebugSession::DebuggerState state, KDevelop::IDebugSession* session);
    void setContinueStartsDebug(bool startsDebug);

    static const QPixmap* executionPointPixmap();

    QAction* m_continueDebugger = nullptr;
    //QAction* m_restartDebugger;
    QAction* m_stopDebugger = nullptr;
    QAction* m_interruptDebugger = nullptr;
    QAction* m_runToCursor = nullptr;
    QAction* m_jumpToCursor = nullptr;
    QAction* m_stepOver = nullptr;
    QAction* m_stepIntoInstruction = nullptr;
    QAction* m_stepInto = nullptr;
    QAction* m_stepOverInstruction = nullptr;
    QAction* m_stepOut = nullptr;
    QAction* m_toggleBreakpoint = nullptr;
    QAction* m_showCurrentLine = nullptr;

    QPointer<IDebugSession> m_currentSession;
    BreakpointModel *m_breakpointModel;
    VariableCollection *m_variableCollection;

    bool m_uiInitialized = false;
};

}

#endif

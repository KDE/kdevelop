/*
 * This file is part of KDevelop
 *
 * Copyright 2016  Aetf <aetf@unlimitedcodeworks.xyz>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TESTDEBUGSESSION_H
#define TESTDEBUGSESSION_H

#include "testsexport.h"

#include <debugger/interfaces/idebugsession.h>

#include <memory>

using std::unique_ptr;

namespace KDevelop {
class IBreakpointController;
class IVariableController;
class IFrameStackModel;

/**
 * \class TestDebugSession testdebugsession.h
 *
 * This is an IDebugSession implementation that should be used only inside
 * unit tests.
 *
 * The initial debug state is NotStartedState, one can use normal slots to
 * change the state, for example calling runToCursor() changes to PausedState.
 * See corresponding implementation for what state a specific slot changes to.
 *
 * This class also allows one to replace IBreakpointController, IVariableController,
 * or IFrameStackModel implementation with custom implementation to make it possible
 * to write tests more easily.
 *
 * The usage is as follows:
 * \code
 * AutoTestShell::init();
 * TestCore::initialize(Core::NoUi);
 * TestDebugSession* session = new TestDebugSession();
 * //replace variable controller or similar
 * session->setVariableController(new MyCustomVariableController(session));
 * ... //test code
 * delete session;
 * TestCore::shutdown();
 * \endcode
 *
 * @note It is important to set custom controllers right after the creation of the
 * debug session, and not change them inside the test code, as required by
 * IDebugSession that controllers do not change during the lifetime of a session
 */
class KDEVPLATFORMTESTS_EXPORT TestDebugSession
    : public IDebugSession
{
    Q_OBJECT

public:
    TestDebugSession();
    ~TestDebugSession() override;

    void setBreakpointController(IBreakpointController* breakpointController);
    void setVariableController(IVariableController* variableController);
    void setFrameStackModel(IFrameStackModel* frameStackModel);

    DebuggerState state() const override;

    bool restartAvaliable() const override;
    IBreakpointController* breakpointController() const override;
    IVariableController* variableController() const override;
    IFrameStackModel* frameStackModel() const override;

public Q_SLOTS:
    void restartDebugger() override;
    void stopDebugger() override;
    void interruptDebugger() override;
    void run() override;
    void runToCursor() override;
    void jumpToCursor() override;
    void stepOver() override;
    void stepIntoInstruction() override;
    void stepInto() override;
    void stepOverInstruction() override;
    void stepOut() override;

private:
    IBreakpointController* m_breakpointController = nullptr;
    IVariableController* m_variableController = nullptr;
    IFrameStackModel* m_frameStackModel = nullptr;
    DebuggerState m_sessionState = NotStartedState;
};
} // end of namespace KDevelop
#endif // TESTDEBUGSESSION_H

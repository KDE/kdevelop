/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
    void killDebuggerNow() override;
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

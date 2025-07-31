/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef LLDBTEST_H
#define LLDBTEST_H

#include "tests/debuggertestbase.h"

namespace KDevelop {
class Variable;
}

namespace KDevMI { namespace LLDB {

class LldbTest : public DebuggerTestBase
{
    Q_OBJECT
protected:
    [[nodiscard]] MIDebugSession* createTestDebugSession() override;

private Q_SLOTS:
    void testBreakOnStart();
    void testChangeLocationBreakpoint();
    void testDeleteBreakpoint();
    void testPendingBreakpoint();
    void testUpdateBreakpoint();
    void testIgnoreHitsBreakpoint();
    void testConditionBreakpoint();
    void testBreakOnWriteBreakpoint();
    void testBreakOnWriteWithConditionBreakpoint();
    void testBreakOnReadBreakpoint();
    void testBreakOnReadBreakpoint2();
    void testBreakOnAccessBreakpoint();
    void testInsertBreakpointWhileRunning();
    void testInsertBreakpointWhileRunningMultiple();
    void testInsertBreakpointFunctionName();
    void testManualBreakpoint();
    void testPickupManuallyInsertedBreakpoint();
    void testPickupManuallyInsertedBreakpointOnlyOnce();
    void testBreakpointWithSpaceInPath();
    void testBreakpointDisabledOnStart();
    void testMultipleLocationsBreakpoint();
    void testMultipleBreakpoint();
    void testRegularExpressionBreakpoint();

    void testCatchpoint();

    void testAttach();
    void testRemoteDebugging();

    void testVariablesLocals();
    void testVariablesWatchesQuotes();
    void testVariablesWatchesTwoSessions();
    void testVariablesStartSecondSession();
    void testVariablesSwitchFrame();
    void testVariablesQuicklySwitchFrame();
    void testSwitchFrameLldbConsole();

    void testSegfaultDebugee();

    void testCommandOrderFastStepping();

    void testRunLldbScript();

    void testBug301287();

    void testSpecialPath();

private:
    // convenient access methods
    KDevelop::Variable *watchVariableAt(int i);
    QModelIndex localVariableIndexAt(int i, int col = 0);

    [[nodiscard]] bool isLldb() const override;
    void startInitTestCase() override;
    void finishInit() override;
};

} // end of namespace LLDB
} // end of namespace KDevMI

#endif // LLDBTEST_H

/*
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>
    SPDX-FileCopyrightText: 2013 Vlas Puhov <vlas.puhov@mail.ru>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef GDBTEST_H
#define GDBTEST_H

#include "tests/debuggertestbase.h"

namespace KDevMI {
namespace GDB {

class GdbTest : public DebuggerTestBase
{
    Q_OBJECT
protected:
    [[nodiscard]] MIDebugSession* createTestDebugSession() override;

private:
    void testStdOut();

    void testBreakpoint();
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
    void testShowStepInSource();
    void testStack();
    void testStackFetchMore();
    void testStackSwitchThread();
private Q_SLOTS:
    void testAttach();
    void testManualAttach();
    void testCoreFile();
    void testVariablesLocals();
    void testVariablesWatchesQuotes();
    void testVariablesWatchesTwoSessions();
    void testVariablesStopDebugger();
    void testVariablesStartSecondSession();
    void testVariablesSwitchFrame();
    void testVariablesQuicklySwitchFrame();
    void testSegfaultDebugee();
    void testSwitchFrameGdbConsole();
    void testCommandOrderFastStepping();
    void testPickupManuallyInsertedBreakpoint();
    void testPickupManuallyInsertedBreakpointOnlyOnce();
    void testPickupCatchThrowOnlyOnce();
    void testRunGdbScript();
    void testRemoteDebug();
    void testRemoteDebugInsertBreakpoint();
    void testRemoteDebugInsertBreakpointPickupOnlyOnce();
    void testBreakpointWithSpaceInPath();
    void testBreakpointDisabledOnStart();
    void testCatchpoint();
    void testThreadAndFrameInfo();
    void parseBug304730();
    void testMultipleLocationsBreakpoint();
    void testBug301287();
    void testMultipleBreakpoint();
    void testRegularExpressionBreakpoint();
    void testPathWithSpace();

private:
    [[nodiscard]] bool isLldb() const override;
    void startInitTestCase() override;
    void finishInit() override;
};

} // end of namespace GDB
} // end of namespace KDevMI

#endif // GDBTEST_H

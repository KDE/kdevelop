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

private Q_SLOTS:
    void testUpdateBreakpoint();
    void testManualBreakpoint();
    void testAttach();
    void testManualAttach();
    void testVariablesLocals();
    void testVariablesWatchesQuotes();
    void testVariablesWatchesTwoSessions();
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

/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef LLDBTEST_H
#define LLDBTEST_H

#include <QObject>

class IExecutePlugin;

namespace KDevelop {
class TestCore;
class Variable;
class VariableCollection;
}

namespace KDevMI { namespace LLDB {

class LldbTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void init();
    void cleanup();

    void testStdout();
    void testEnvironmentSet();

    void testUnsupportedUrlExpressionBreakpoints();
    void testBreakpoint();
    void testBreakOnStart();
    void testDisableBreakpoint();
    void testChangeLocationBreakpoint();
    void testDeleteBreakpoint();
    void testPendingBreakpoint();
    void testBreakpointsOnNoOpLines();
    void testUpdateBreakpoint();
    void testIgnoreHitsBreakpoint();
    void testConditionBreakpoint();
    void testBreakOnWriteBreakpoint();
    void testBreakOnWriteWithConditionBreakpoint();
    void testBreakOnReadBreakpoint();
    void testBreakOnReadBreakpoint2();
    void testBreakOnAccessBreakpoint();
    void testBreakpointErrors();
    void testInsertBreakpointWhileRunning();
    void testInsertBreakpointWhileRunningMultiple();
    void testInsertBreakpointFunctionName();
    void testManualBreakpoint();
    void testInsertAndRemoveBreakpointWhileRunning();
    void testPickupManuallyInsertedBreakpoint();
    void testPickupManuallyInsertedBreakpointOnlyOnce();
    void testBreakpointWithSpaceInPath();
    void testBreakpointDisabledOnStart();
    void testMultipleLocationsBreakpoint();
    void testMultipleBreakpoint();
    void testRegularExpressionBreakpoint();
    void testChangeBreakpointWhileRunning();

    void testCatchpoint();

    void testShowStepInSource();

    void testStack();
    void testStackFetchMore();
    void testStackSwitchThread();

    void testAttach();
    void testRemoteDebugging();

    void testCoreFile();

    void testVariablesLocals();
    void testVariablesLocalsStruct();
    void testVariablesWatches();
    void testVariablesWatchesQuotes();
    void testVariablesWatchesTwoSessions();
    void testVariablesStopDebugger();
    void testVariablesStartSecondSession();
    void testVariablesSwitchFrame();
    void testVariablesQuicklySwitchFrame();
    void testSwitchFrameLldbConsole();

    void testSegfaultDebugee();

    void testCommandOrderFastStepping();

    void testRunLldbScript();

    void testBug301287();

    void testDebugInExternalTerminal();

    void testSpecialPath();

    void testEnvironmentCd();

private:
    // convenient access methods
    KDevelop::VariableCollection *variableCollection();
    KDevelop::Variable *watchVariableAt(int i);
    QModelIndex localVariableIndexAt(int i, int col = 0);

private:
    KDevelop::TestCore *m_core;
    IExecutePlugin *m_iface;
};

} // end of namespace LLDB
} // end of namespace KDevMI

#endif // LLDBTEST_H

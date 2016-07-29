/*
 * Unit tests for LLDB debugger plugin
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

#ifndef LLDBTEST_H
#define LLDBTEST_H

#include <QObject>

class IExecutePlugin;

namespace KDevelop {
class BreakpointModel;
class TestCore;
class Variable;
class VariableCollection;
}

namespace KDevMI { namespace LLDB {

class LldbTest : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();

    void init();
    void cleanup();

    void testStdout();
    void testEnvironmentSet();

    void testBreakpoint();
    void testBreakOnStart();
    void testDisableBreakpoint();
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
    void testStackDeactivateAndActive();
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

private:
    // convenient access methods
    KDevelop::BreakpointModel *breakpoints();

    KDevelop::VariableCollection *variableCollection();
    KDevelop::Variable *watchVariableAt(int i);
    KDevelop::Variable *localVariableAt(int i);

private:
    KDevelop::TestCore *m_core;
    IExecutePlugin *m_iface;

    QString m_debugeeFileName;
};

} // end of namespace LLDB
} // end of namespace KDevMI

#endif // LLDBTEST_H

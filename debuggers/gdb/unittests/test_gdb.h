/*
   Copyright 2009 Niko Sams <niko.sams@gmail.com>
   Copyright 2013 Vlas Puhov <vlas.puhov@mail.ru>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef GDBTEST_H
#define GDBTEST_H

#include <QtCore/QObject>
#include <debugsession.h>

namespace KDevelop {
class TestCore;
}
namespace GDBDebugger {

class GdbTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();

    void testStdOut();
    void testBreakpoint();
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
    void testShowStepInSource();
    void testStack();
    void testStackFetchMore();
    void testStackDeactivateAndActive();
    void testStackSwitchThread();
    void testAttach();
    void testManualAttach();
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
    void testSegfaultDebugee();
    void testSwitchFrameGdbConsole();
    void testInsertAndRemoveBreakpointWhileRunning();
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
    void testChangeBreakpointWhileRunning();
    void testDebugInExternalTerminal();
    void testPathWithSpace();

private:
    void waitForState(GDBDebugger::DebugSession *session,
                      KDevelop::IDebugSession::DebuggerState state,
                      const char *file, int line,
                      bool waitForIdle = false);
    IExecutePlugin* m_iface;
};

}

#endif // GDBTEST_H

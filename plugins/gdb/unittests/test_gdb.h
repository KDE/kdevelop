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

    [[nodiscard]] const char* configScriptEntryKey() const override;
    [[nodiscard]] const char* runScriptEntryKey() const override;

private Q_SLOTS:
    void testUpdateBreakpoint();
    void testManualBreakpoint();
    void testAttach();
    void testManualAttach();
    void testVariablesLocals();
    void testVariablesWatchesQuotes();
    void testVariablesWatchesTwoSessions();

    void testPickupCatchThrowOnlyOnce();

    void testRemoteDebug_data();
    void testRemoteDebug();

    void testBreakpointDisabledOnStart();

    void parseBug304730();

private:
    [[nodiscard]] bool isLldb() const override;
    void startInitTestCase() override;
    void finishInit() override;
};

} // end of namespace GDB
} // end of namespace KDevMI

#endif // GDBTEST_H

/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef LLDBTEST_H
#define LLDBTEST_H

#include "tests/debuggertestbase.h"

namespace KDevMI { namespace LLDB {

class LldbTest : public DebuggerTestBase
{
    Q_OBJECT
protected:
    [[nodiscard]] MIDebugSession* createTestDebugSession() override;

    [[nodiscard]] const char* configScriptEntryKey() const override;
    [[nodiscard]] const char* runScriptEntryKey() const override;

private Q_SLOTS:
    void testBreakOnStart();
    void testUpdateBreakpoint();
    void testManualBreakpoint();
    void testBreakpointDisabledOnStart();

    void testAttach();
    void testRemoteDebugging();

    void testVariablesLocals();
    void testVariablesWatchesQuotes();

private:
    [[nodiscard]] bool isLldb() const override;
    void startInitTestCase() override;
};

} // end of namespace LLDB
} // end of namespace KDevMI

#endif // LLDBTEST_H

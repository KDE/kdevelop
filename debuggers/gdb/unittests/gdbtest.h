/*
   Copyright 2009 Niko Sams <niko.sams@gmail.com>

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
class GdbTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init();
    void cleanup();

    void testStdOut();
    void testBreakpoint();
    void testDisableBreakpoint();
    void testChangeLocationBreakpoint();
    void testDeleteBreakpoint();
    void testPendingBreakpoint();
    void testUpdateBreakpoint();
    void testShowStepInSource();
    void testStack();
private:
    KDevelop::TestCore* m_core;
    void waitForState(const GDBDebugger::DebugSession& session, KDevelop::IDebugSession::DebuggerState state);
};

#endif // GDBTEST_H

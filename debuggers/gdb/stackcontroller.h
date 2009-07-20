/*
 * GDB Debugger Support
 *
 * Copyright 1999 John Birch <jbb@kdevelop.org>
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2009 Niko Sams <niko.sams@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef GDBDEBUGGER_STACKCONTROLLER_H
#define GDBDEBUGGER_STACKCONTROLLER_H

#include <debugger/interfaces/istackcontroller.h>

namespace GDBMI {
class ResultRecord;
}
namespace GDBDebugger {

class DebugSession;

class StackController : public KDevelop::IStackController
{
public:
    StackController(DebugSession *session);

    virtual void fetchThreads();
    virtual void fetchFrames(int threadNumber, int from, int to);

private:
    DebugSession *session();

    void handleThreadInfo(const GDBMI::ResultRecord& r);
    void handleFrameList(const GDBMI::ResultRecord& r);

};

}

#endif // GDBDEBUGGER_STACKCONTROLLER_H

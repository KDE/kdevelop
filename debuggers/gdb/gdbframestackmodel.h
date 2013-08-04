/*
 * GDB-specific implementation of thread and frame model.
 * 
 * Copyright 2009 Vladimir Prus <ghost@cs.msu.su>
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

#ifndef KDEVELOP_GDB_FRAMESTACKMODEL_H
#define KDEVELOP_GDB_FRAMESTACKMODEL_H

#include <debugger/framestack/framestackmodel.h>

#include "debugsession.h"
using namespace GDBDebugger;

namespace GDBMI { struct ResultRecord; }

namespace KDevelop {
    
    class GdbFrameStackModel : public FrameStackModel
    {
    public:
        GdbFrameStackModel(DebugSession* session) : FrameStackModel(session) {}
        
    public:
        DebugSession* session() { return static_cast<DebugSession *>(FrameStackModel::session()); }    
        
    protected: // FrameStackModel overrides
        void fetchThreads();
        void fetchFrames(int threadNumber, int from, int to);
        
    private:        
        void handleThreadInfo(const GDBMI::ResultRecord& r);
    };
}

#endif

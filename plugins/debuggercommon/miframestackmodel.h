/*
 * Implementation of thread and frame model that are common to debuggers using MI.
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

#ifndef KDEVELOP_MI_FRAMESTACKMODEL_H
#define KDEVELOP_MI_FRAMESTACKMODEL_H

#include <debugger/framestack/framestackmodel.h>

namespace KDevMI {

namespace MI {
struct ResultRecord;
}

class MIDebugSession;

class MIFrameStackModel : public KDevelop::FrameStackModel
{
    Q_OBJECT

public:
    explicit MIFrameStackModel( MIDebugSession* session);

    MIDebugSession* session();

protected: // FrameStackModel overrides
    void fetchThreads() override;
    void fetchFrames(int threadNumber, int from, int to) override;

private:
    void handleThreadInfo(const MI::ResultRecord& r);
};

} // end of namespace KDevMI

#endif

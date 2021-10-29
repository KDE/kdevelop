/*
    SPDX-FileCopyrightText: 2009 Vladimir Prus <ghost@cs.msu.su>

    SPDX-License-Identifier: GPL-2.0-or-later
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

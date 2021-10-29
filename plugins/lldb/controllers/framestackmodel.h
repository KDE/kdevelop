/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef LLDB_FRAMESTACKMODEL_H
#define LLDB_FRAMESTACKMODEL_H

#include "miframestackmodel.h"

namespace KDevMI {

namespace MI {
struct AsyncRecord;
}

namespace LLDB {

class DebugSession;
class LldbFrameStackModel : public MIFrameStackModel
{
    Q_OBJECT
public:
    explicit LldbFrameStackModel(DebugSession* session);

    DebugSession* session();

protected:
    void fetchThreads() override;

private Q_SLOTS:
    void inferiorStopped(const MI::AsyncRecord& r);

private:
    void handleThreadInfo(const MI::ResultRecord& r);

    int stoppedAtThread;
};

} // end of namespace LLDB
} // end of namespace KDevMI

#endif // LLDB_FRAMESTACKMODEL_H

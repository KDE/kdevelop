/*
 * LLDB-specific implementation of frame stack model
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

#include "framestackmodel.h"

#include "debuglog.h"
#include "debugsession.h"
#include "mi/micommand.h"

#include <KLocalizedString>

namespace {

QString getFunctionOrAddress(const KDevMI::MI::Value &frame)
{
    if (frame.hasField("func"))
        return frame["func"].literal();
    else
        return frame["addr"].literal();
}

}

using namespace KDevMI::LLDB;
using namespace KDevMI::MI;

LldbFrameStackModel::LldbFrameStackModel(DebugSession *session)
    : MIFrameStackModel(session)
    , stoppedAtThread(-1)
{
    connect(session, &DebugSession::inferiorStopped, this, &LldbFrameStackModel::inferiorStopped);
}

DebugSession* LldbFrameStackModel::session()
{
    return static_cast<DebugSession *>(FrameStackModel::session());
}

void LldbFrameStackModel::inferiorStopped(const MI::AsyncRecord& r)
{
    if (session()->debuggerStateIsOn(s_shuttingDown)) return;

    if (r.hasField("thread-id")) {
        stoppedAtThread = r["thread-id"].toInt();
    }
}

void LldbFrameStackModel::fetchThreads()
{
    // TODO: preliminary test shows there might be a bug in lldb-mi
    // that's causing std::logic_error when executing -thread-info with
    // more than one threads. Find a workaround for this (and report bug
    // if it truely is).
    session()->addCommand(ThreadInfo, "", this, &LldbFrameStackModel::handleThreadInfo);
}

void LldbFrameStackModel::handleThreadInfo(const ResultRecord& r)
{
    const Value& threads = r["threads"];

    QList<FrameStackModel::ThreadItem> threadsList;
    for (int gidx = 0; gidx != threads.size(); ++gidx) {
        FrameStackModel::ThreadItem i;
        const Value & threadMI = threads[gidx];
        i.nr = threadMI["id"].toInt();
        if (threadMI["state"].literal() == "stopped") {
            // lldb-mi returns multiple frame entry for each thread
            // so can't directly use threadMI["frame"]
            auto &th = dynamic_cast<const TupleValue&>(threadMI);
            Value *topFrame = nullptr;
            for (auto res : th.results) {
                if (res->variable == "frame") {
                    if (!topFrame || (*res->value)["level"].toInt() < (*topFrame)["level"].toInt()) {
                        topFrame = res->value;
                    }
                }
            }
            i.name = getFunctionOrAddress(*topFrame);
        } else {
            i.name = i18n("(running)");
        }
        threadsList << i;
    }
    setThreads(threadsList);
    if (r.hasField("current-thread-id")) {
        int currentThreadId = r["current-thread-id"].toInt();

        setCurrentThread(currentThreadId);

        if (session()->hasCrashed()) {
            setCrashedThreadIndex(currentThreadId);
        }
    }
    // lldb-mi doesn't have current-thread-id field. Use the thread-id field when inferiorStopped
    if (stoppedAtThread != -1) {
        setCurrentThread(stoppedAtThread);
    }
    stoppedAtThread = -1;
}

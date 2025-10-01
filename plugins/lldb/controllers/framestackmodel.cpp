/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "framestackmodel.h"

#include "debuglog.h"
#include "debugsession.h"
#include "mi/micommand.h"

#include <KLocalizedString>

namespace {

QString getFunctionOrAddress(const KDevMI::MI::Value &frame)
{
    if (frame.hasField(QStringLiteral("func")))
        return frame[QStringLiteral("func")].literal();
    else
        return frame[QStringLiteral("addr")].literal();
}

}

using namespace KDevMI::LLDB;
using namespace KDevMI::MI;
using namespace KDevMI;

LldbFrameStackModel::LldbFrameStackModel(DebugSession *session)
    : MIFrameStackModel(session)
{
}

DebugSession* LldbFrameStackModel::session()
{
    return static_cast<DebugSession *>(FrameStackModel::session());
}

void LldbFrameStackModel::fetchThreads()
{
    // TODO: preliminary test shows there might be a bug in lldb-mi
    // that's causing std::logic_error when executing -thread-info with
    // more than one threads. Find a workaround for this (and report bug
    // if it truly is).
    session()->addCommand(ThreadInfo, QString(), this, &LldbFrameStackModel::handleThreadInfo);
}

void LldbFrameStackModel::handleThreadInfo(const ResultRecord& r)
{
    const Value& threads = r[QStringLiteral("threads")];

    if (threads.kind == Value::StringLiteral && threads.literal() == QLatin1String{"[]"}) {
        // LLDB-MI replies with `threads="[]"` if the session state is not paused.
        // This occurs if the exec-continue command is queued and sent just before the thread-info command.
        // The quotes around [] must be a LLDB-MI bug because the type
        // of `threads` must be `list`, not `const` (aka string literal).
        qCDebug(DEBUGGERLLDB)
            << "debugger replied with `threads=\"[]\"`, setting zero threads and -1 as the current thread";
        setThreads({});
        setCurrentThread(-1);
        return;
    }

    QVector<FrameStackModel::ThreadItem> threadsList;
    threadsList.reserve(threads.size());
    for (int gidx = 0; gidx != threads.size(); ++gidx) {
        FrameStackModel::ThreadItem i;
        const Value & threadMI = threads[gidx];
        i.nr = threadMI[QStringLiteral("id")].toInt();
        if (threadMI[QStringLiteral("state")].literal() == QLatin1String("stopped")) {
            // lldb-mi returns multiple frame entry for each thread
            // so can't directly use threadMI["frame"]
            auto &th = static_cast<const TupleValue&>(threadMI);
            Value *topFrame = nullptr;
            for (auto res : th.results) {
                if (res->variable == QLatin1String("frame")) {
                    if (!topFrame || (*res->value)[QStringLiteral("level")].toInt() < (*topFrame)[QStringLiteral("level")].toInt()) {
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
    if (r.hasField(QStringLiteral("current-thread-id"))) {
        int currentThreadId = r[QStringLiteral("current-thread-id")].toInt();

        setCurrentThread(currentThreadId);

        if (session()->hasCrashed()) {
            setCrashedThreadIndex(currentThreadId);
        }
    }
}

#include "moc_framestackmodel.cpp"

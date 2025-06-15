/*
    SPDX-FileCopyrightText: 2009 Vladimir Prus <ghost@cs.msu.su>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "miframestackmodel.h"

#include "midebugsession.h"
#include "mi/micommand.h"

#include <debuglog.h>

#include <KLocalizedString>

#include <algorithm>

using namespace KDevelop;
using namespace KDevMI;
using namespace KDevMI::MI;

QString getFunctionOrAddress(const Value &frame)
{
    if (frame.hasField(QStringLiteral("func")))
        return frame[QStringLiteral("func")].literal();
    else
        return frame[QStringLiteral("addr")].literal();
}

QPair<QString, int> getSource(const Value &frame)
{
    QPair<QString, int> ret(QString(), -1);
    if (frame.hasField(QStringLiteral("fullname")))
        ret=qMakePair(frame[QStringLiteral("fullname")].literal(), frame[QStringLiteral("line")].toInt()-1);
    else if (frame.hasField(QStringLiteral("file")))
        ret=qMakePair(frame[QStringLiteral("file")].literal(), frame[QStringLiteral("line")].toInt()-1);
    else if (frame.hasField(QStringLiteral("from")))
        ret.first=frame[QStringLiteral("from")].literal();

    return ret;
}

MIFrameStackModel::MIFrameStackModel(MIDebugSession * session)
    : FrameStackModel(session)
{
}

MIDebugSession * MIFrameStackModel::session()
{
    return static_cast<MIDebugSession *>(FrameStackModel::session());
}

void MIFrameStackModel::fetchThreads()
{
    session()->addCommand(ThreadInfo, QString(), this, &MIFrameStackModel::handleThreadInfo);
}

void MIFrameStackModel::handleThreadInfo(const ResultRecord& r)
{
    const Value& threads = r[QStringLiteral("threads")];

    QVector<FrameStackModel::ThreadItem> threadsList;
    threadsList.reserve(threads.size());
    for (int i = 0; i!= threads.size(); ++i) {
        const auto &threadMI = threads[i];
        FrameStackModel::ThreadItem threadItem;
        threadItem.nr = threadMI[QStringLiteral("id")].toInt();
        if (threadMI[QStringLiteral("state")].literal() == QLatin1String("stopped")) {
            threadItem.name = getFunctionOrAddress(threadMI[QStringLiteral("frame")]);
        } else {
            threadItem.name = i18n("(running)");
        }
        threadsList << threadItem;
    }
    // Sort the list by id, some old version of GDB
    // reports them in backward order. We want UI to
    // show thread IDs in the natural order.
    std::sort(threadsList.begin(), threadsList.end(),
              [](const FrameStackModel::ThreadItem &a, const FrameStackModel::ThreadItem &b){
                  return a.nr < b.nr;
              });

    setThreads(threadsList);
    if (r.hasField(QStringLiteral("current-thread-id"))) {
        int currentThreadId = r[QStringLiteral("current-thread-id")].toInt();

        setCurrentThread(currentThreadId);

        if (session()->hasCrashed()) {
            setCrashedThreadIndex(currentThreadId);
        }
    }
}

struct FrameListHandler : public MICommandHandler
{
    FrameListHandler(MIFrameStackModel* model, int thread, int to)
        : model(model), m_thread(thread) , m_to(to) {}

    void handle(const ResultRecord &r) override
    {
        static const auto levelField = QStringLiteral("level");
        const Value& stack = r[QStringLiteral("stack")];
        const auto& firstFrame = stack[0];

        if (stack.size() == 1
            && !firstFrame.hasField(levelField)
            // If the hasField() call above does not throw an exception, firstFrame is guaranteed to be a TupleValue.
            && static_cast<const TupleValue&>(firstFrame).results_by_name.empty()) {
            // LLDB-MI replies with `stack=[{}]` if the session state is not paused.
            // This occurs if the exec-continue command is queued and sent just before the stack-list-frames command.
            qCDebug(DEBUGGERCOMMON) << "debugger replied with `stack=[{}]`, setting zero frames and no more frames";
            model->setFrames(m_thread, {});
            model->setHasMoreFrames(m_thread, false);
            return;
        }

        const auto first = firstFrame[levelField].toInt();
        QVector<KDevelop::FrameStackModel::FrameItem> frames;
        frames.reserve(stack.size());
        for (int i = 0; i< stack.size(); ++i) {
            const Value& frame = stack[i];
            KDevelop::FrameStackModel::FrameItem f;
            f.nr = frame[levelField].toInt();
            f.name = getFunctionOrAddress(frame);
            QPair<QString, int> loc = getSource(frame);
            f.file = QUrl::fromLocalFile(loc.first).adjusted(QUrl::NormalizePathSegments);
            f.line = loc.second;
            frames << f;
        }
        bool hasMore = false;
        if (!frames.isEmpty()) {
            if (frames.last().nr == m_to+1) {
                frames.removeLast();
                hasMore = true;
            }
        }
        if (first == 0) {
            model->setFrames(m_thread, frames);
        } else {
            model->insertFrames(m_thread, frames);
        }
        model->setHasMoreFrames(m_thread, hasMore);
    }
private:
    MIFrameStackModel* model;
    int m_thread;
    int m_to;
};

void MIFrameStackModel::fetchFrames(int threadNumber, int from, int to)
{
    //to+1 so we know if there are more
    QString arg = QStringLiteral("%1 %2").arg(from).arg(to+1);
    auto c = session()->createCommand(StackListFrames, arg);
    c->setHandler(new FrameListHandler(this, threadNumber, to));
    c->setThread(threadNumber);
    session()->addCommand(std::move(c));
}

#include "moc_miframestackmodel.cpp"

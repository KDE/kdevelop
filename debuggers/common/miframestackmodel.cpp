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

#include "miframestackmodel.h"

#include "midebugsession.h"
#include "mi/micommand.h"

#include <KLocalizedString>

using namespace KDevelop;
using namespace KDevMI;
using namespace KDevMI::MI;

QString getFunctionOrAddress(const Value &frame)
{
    if (frame.hasField("func"))
        return frame["func"].literal();
    else
        return frame["addr"].literal();
}

QPair<QString, int> getSource(const Value &frame)
{
    QPair<QString, int> ret(QString(), -1);
    if (frame.hasField("fullname"))
        ret=qMakePair(frame["fullname"].literal(), frame["line"].toInt()-1);
    else if (frame.hasField("file"))
        ret=qMakePair(frame["file"].literal(), frame["line"].toInt()-1);
    else if (frame.hasField("from"))
        ret.first=frame["from"].literal();

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
    // TODO: preliminary test shows there might be a bug in lldb-mi
    // that's causing std::logic_error when executing -thread-info with
    // more than one threads. Find a workaround for this (and report bug
    // if it truely is).
    session()->addCommand(ThreadInfo, "", this, &MIFrameStackModel::handleThreadInfo);
}

void MIFrameStackModel::handleThreadInfo(const ResultRecord& r)
{
    const Value& threads = r["threads"];

    // Traverse GDB threads in backward order -- since GDB
    // reports them in backward order. We want UI to
    // show thread IDs in the natural order.
    // FIXME: at least GDB 7.11 is reporting in the right order,
    // consider sort the list afterwards.

    QList<KDevelop::FrameStackModel::ThreadItem> threadsList;
    int gidx = threads.size()-1;
    for (; gidx >= 0; --gidx) {
        KDevelop::FrameStackModel::ThreadItem i;
        const Value & threadMI = threads[gidx];
        i.nr = threadMI["id"].toInt();
        if (threadMI["state"].literal() == "stopped") {
            i.name = getFunctionOrAddress(threads[gidx]["frame"]);
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
}

struct FrameListHandler : public MICommandHandler
{
    FrameListHandler(MIFrameStackModel* model, int thread, int to)
        : model(model), m_thread(thread) , m_to(to) {}

    void handle(const ResultRecord &r) override
    {
        const Value& stack = r["stack"];
        int first = stack[0]["level"].toInt();
        QList<KDevelop::FrameStackModel::FrameItem> frames;
        for (int i = 0; i< stack.size(); ++i) {
            const Value& frame = stack[i];
            KDevelop::FrameStackModel::FrameItem f;
            f.nr = frame["level"].toInt();
            f.name = getFunctionOrAddress(frame);
            QPair<QString, int> loc = getSource(frame);
            f.file = QUrl::fromLocalFile(loc.first);
            f.line = loc.second;
            frames << f;
        }
        bool hasMore = false;
        if (!frames.isEmpty()) {
            if (frames.last().nr == m_to+1) {
                frames.takeLast();
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
    QString arg = QString("%1 %2").arg(from).arg(to+1);
    MICommand *c = session()->createCommand(StackListFrames, arg);
    c->setHandler(new FrameListHandler(this, threadNumber, to));
    c->setThread(threadNumber);
    session()->addCommand(c);
}

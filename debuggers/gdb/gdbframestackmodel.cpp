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

#include "gdbframestackmodel.h"
#include "gdbcommand.h"

#include <KLocalizedString>

#include <algorithm>

using namespace KDevelop;

QString getFunctionOrAddress(const GDBMI::Value &frame)
{
    if (frame.hasField("func"))
        return frame["func"].literal();
    else
        return frame["addr"].literal();
}

QPair<QString, int> getSource(const GDBMI::Value &frame)
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

void GdbFrameStackModel::fetchThreads()
{
    session()->addCommand(
        new GDBCommand(GDBMI::ThreadInfo, "",
                    this,
                    &GdbFrameStackModel::handleThreadInfo));    
}

void GdbFrameStackModel::handleThreadInfo(const GDBMI::ResultRecord& r)
{
    const GDBMI::Value& threads = r["threads"];

    QList<FrameStackModel::ThreadItem> threadsList;
    for (int i = 0; i!= threads.size(); ++i) {
        const auto &threadMI = threads[i];
        FrameStackModel::ThreadItem threadItem;
        threadItem.nr = threadMI["id"].toInt();
        if (threadMI["state"].literal() == "stopped") {
            threadItem.name = getFunctionOrAddress(threadMI["frame"]);
        } else {
            i18n("(running)");
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
    if (r.hasField("current-thread-id")) {
        int currentThreadId = r["current-thread-id"].toInt();

        setCurrentThread(currentThreadId);

        if (session()->hasCrashed()) {
            setCrashedThreadIndex(currentThreadId);
        }
    }
}

struct FrameListHandler : public GDBCommandHandler
{
    FrameListHandler(GdbFrameStackModel* model, int thread, int to)
        : model(model), m_thread(thread) , m_to(to) {}

    void handle(const GDBMI::ResultRecord &r) override
    {
        const GDBMI::Value& stack = r["stack"];
        int first = stack[0]["level"].toInt();
        QList<KDevelop::FrameStackModel::FrameItem> frames;
        for (int i = 0; i< stack.size(); ++i) {
            const GDBMI::Value& frame = stack[i];
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
    GdbFrameStackModel* model;
    int m_thread;
    int m_to;
};

void GdbFrameStackModel::fetchFrames(int threadNumber, int from, int to)
{
    //to+1 so we know if there are more
    QString arg = QString("%1 %2").arg(from).arg(to+1);
    GDBCommand *c = new GDBCommand(GDBMI::StackListFrames, arg,
                                   new FrameListHandler(this, threadNumber, to));
    c->setThread(threadNumber);
    session()->addCommand(c);
}

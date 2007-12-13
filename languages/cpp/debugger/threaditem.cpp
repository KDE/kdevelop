/*
 * GDB Debugger Support
 *
 * Copyright 1999 John Birch <jbb@kdevelop.org>
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
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

#include "threaditem.h"

#include <klocale.h>
#include <kdebug.h>
#include <kglobalsettings.h>

#include "gdbparser.h"
#include "gdbcommand.h"
#include "gdbcontroller.h"
#include "framestackitem.h"
#include "stackmanager.h"

using namespace GDBMI;
using namespace GDBDebugger;

// Guestimate for a reasonable value ;)
const int g_lookaheadCount = 20;

ThreadItem::ThreadItem(StackManager* stack, int thread)
    : QObject(stack)
    , m_thread(thread)
    , m_lastConfirmedFrame(0)
    , m_moreFramesAvailable(true)
{
}

ThreadItem::~ThreadItem()
{
}

int ThreadItem::thread() const
{
    return m_thread;
}

const QList< FrameStackItem * > & ThreadItem::frames() const
{
    return m_frames;
}

bool ThreadItem::moreFramesAvailable() const
{
    return m_moreFramesAvailable;
}

void ThreadItem::fetchMoreFrames()
{
    GDBCommand* stackInfoDepth = new GDBCommand(StackInfoDepth, m_lastConfirmedFrame + g_lookaheadCount);
    stackInfoDepth->setHandler(this, &ThreadItem::handleStackDepth);
    stackInfoDepth->setThread(thread());
    stackManager()->controller()->addCommand(stackInfoDepth);
}

StackManager* ThreadItem::stackManager() const
{
    return static_cast<StackManager*>(const_cast<QObject*>(parent()));
}

void ThreadItem::handleStackDepth(const GDBMI::ResultRecord& r)
{
    int lastActuallyRequestedFrame = m_lastConfirmedFrame + g_lookaheadCount;

    m_lastConfirmedFrame = r["depth"].literal().toInt();

    m_moreFramesAvailable = m_lastConfirmedFrame >= lastActuallyRequestedFrame;

    if (m_frames.count() <= m_lastConfirmedFrame) {
        stackManager()->prepareInsertFrames(this, m_frames.count(), m_lastConfirmedFrame);

        for (int i = m_frames.count(); i <= m_lastConfirmedFrame; ++i)
            createFrame(i);

        stackManager()->completeInsertFrames();
    }
}

void ThreadItem::refreshFrames(int from, int to)
{
    // Check that it's not already in progress
    for (int i = from; i <= to; ++i)
        if (!m_frames[i]->isRefreshRequested())
            goto dirty;

    return;

    dirty:

    // For now just update the whole block, even if some are not dirty
    for (int i = from; i <= to && i < m_frames.count(); ++i)
        m_frames[i]->setRefreshRequested();

    GDBCommand* listFrameCommand = new GDBCommand(StackListFrames, QString("%1 %2").arg(from).arg(to));
    listFrameCommand->setHandler(this, &ThreadItem::parseGDBBacktraceList);
    listFrameCommand->setThread(thread());
    stackManager()->controller()->addCommand(listFrameCommand);
}

void ThreadItem::parseGDBBacktraceList(const GDBMI::ResultRecord& r)
{
    if (!r.hasField("stack"))
        return;

    const GDBMI::Value& frames = r["stack"];

    if (frames.empty())
        return;

    Q_ASSERT(dynamic_cast<const GDBMI::ListValue*>(&frames));

    for (int i = 0, e = frames.size(); i != e; ++i)
    {
        FrameStackItem* frame = createFrame(frames[i]["level"].literal().toInt());
        frame->parseFrame(frames[i]);
    }
}

void ThreadItem::parseThread(const GDBMI::ResultRecord& r)
{
    Q_ASSERT(m_thread == r["new-thread-id"].literal().toInt());

    // We're given the first frame too, parse it

    // First frame doesn't have a model index (shares the thread index), don't have to notify creation
    FrameStackItem* frame = createFrame(0);
    frame->parseFrame(r["frame"]);
}

FrameStackItem* ThreadItem::createFrame(int frame)
{
    if (frame < m_frames.count())
        return m_frames.at(frame);

    FrameStackItem* item = new FrameStackItem(this, frame);
    m_frames.append(item);
    return item;
}

void GDBDebugger::ThreadItem::setDirty()
{
    m_moreFramesAvailable = true;
}

#include "threaditem.moc"

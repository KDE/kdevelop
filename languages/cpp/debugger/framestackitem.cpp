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

#include "framestackitem.h"

#include <KLocale>

#include "threaditem.h"
#include "stackmanager.h"

using namespace GDBMI;
using namespace GDBDebugger;

FrameStackItem::FrameStackItem(ThreadItem* thread, int frame)
    : QObject(thread)
    , m_frame(frame)
    , m_dirty(true)
    , m_refreshRequested(false)
    , m_level(-1)
    , m_function(i18n("<Loading...>"))
    , m_sourceLine(-1)
{
}

ThreadItem* FrameStackItem::thread() const
{
    return static_cast<ThreadItem*>(const_cast<QObject*>(parent()));
}

int FrameStackItem::frame() const
{
    return m_frame;
}

void FrameStackItem::parseFrame(const GDBMI::Value& frame)
{
    m_dirty = false;
    m_refreshRequested = false;

    m_level = frame["level"].literal().toInt();

    if (frame.hasField("func"))
        m_function = frame["func"].literal();

    if (frame.hasField("address"))
        m_address = frame["address"].literal().toULongLong();

    if (frame.hasField("file"))
    {
        m_source = frame["file"].literal();

        if (frame.hasField("line"))
        {
            m_sourceLine = frame["line"].literal().toInt();
        }
    }
    else if (frame.hasField("from"))
    {
        m_source = frame["from"].literal();
    }

    stackManager()->dataChanged(this);
}

QString GDBDebugger::FrameStackItem::function() const
{
    return m_function;
}

QString GDBDebugger::FrameStackItem::sourceString() const
{
    if (m_sourceLine != -1)
        return QString("%1:%2").arg(m_source).arg(m_sourceLine);

    return m_source;
}

int GDBDebugger::FrameStackItem::sourceLine() const
{
    return m_sourceLine;
}

StackManager* FrameStackItem::stackManager() const
{
    return thread()->stackManager();
}

bool GDBDebugger::FrameStackItem::isDirty() const
{
    return m_dirty;
}

bool GDBDebugger::FrameStackItem::isRefreshRequested() const
{
    return m_refreshRequested;
}

void GDBDebugger::FrameStackItem::setRefreshRequested()
{
    m_refreshRequested = true;
}

void FrameStackItem::refresh()
{
    if (m_refreshRequested)
        return;

    // Refresh in 5 frame blocks
    int from = frame() - (frame() % 5);
    int to = from + 4;
    thread()->refreshFrames(from, to);

    Q_ASSERT(m_refreshRequested);
}

#include "framestackitem.moc"

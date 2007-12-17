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

#ifndef THREADITEM_H
#define THREADITEM_H

#include <QObject>

#include "mi/miparser.h"

namespace GDBDebugger
{

class StackManager;
class FrameStackItem;

class ThreadItem : public QObject
{
    Q_OBJECT

    friend class StackManager;

public:
    ThreadItem(StackManager* stack, int thread);
    virtual ~ThreadItem();

    StackManager* stackManager() const;
    int thread() const;

    const QList<FrameStackItem*>& frames() const;

    /**
     * Returns true if more frames than currently loaded are available.
     */
    bool moreFramesAvailable() const;

    void setDirty();

    /**
     * Check to see if more frames are available from GDB, and create them if so.
     */
    void fetchMoreFrames();

    /**
     * Request a refresh of the frames \a from -> \a to
     */
    void refreshFrames(int from, int to);

protected:
    void parseThread(const GDBMI::ResultRecord&);
    void parseGDBBacktraceList(const GDBMI::ResultRecord&);
    void handleStackDepth(const GDBMI::ResultRecord& r);

private:
    FrameStackItem* createFrame(int frame);

    StackManager* m_stack;
    int m_thread;
    QString m_savedFunc;
    QString m_savedSource;

    QList<FrameStackItem*> m_frames;

    int m_lastConfirmedFrame;
    bool m_moreFramesAvailable;
};

}

#endif // THREADITEM_H

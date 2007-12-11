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

#ifndef FRAMESTACKITEM_H
#define FRAMESTACKITEM_H

#include <QObject>

#include "mi/gdbmi.h"

namespace GDBDebugger
{

class StackManager;
class ThreadItem;

class FrameStackItem : public QObject
{
    Q_OBJECT

public:
    FrameStackItem(ThreadItem* thread, int frame);

    StackManager* stackManager() const;

    ThreadItem* thread() const;
    int frame() const;

    bool isDirty() const;
    bool isRefreshRequested() const;
    void setRefreshRequested();
    void refresh();

    QString function() const;
    QString sourceString() const;
    int sourceLine() const;

    void parseFrame(const GDBMI::Value& frame);

private:
    // Which frame
    ThreadItem* m_thread;
    int m_frame;
    bool m_dirty :1;
    bool m_refreshRequested :1;

    // Details
    int m_level;
    qulonglong m_address;
    QString m_function;
    QString m_source;
    int m_sourceLine;
};

}

#endif // FRAMESTACKITEM_H

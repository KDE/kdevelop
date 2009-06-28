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

#ifndef STACKMANAGER_H
#define STACKMANAGER_H

#include <QAbstractItemModel>

#include "mi/miparser.h"
#include "gdbglobal.h"
#include <debugger/interfaces/stackitem.h>
#include <debugger/interfaces/stackmodel.h>

namespace KDevelop {
class TreeModel;
}

namespace GDBDebugger
{
class DebugSession;

class Thread : public KDevelop::ThreadItem
{
    Q_OBJECT
public:
    Thread(KDevelop::StackModel* model, DebugSession *session,
           const GDBMI::Value& thread);

    int id() const { return id_; }

    virtual void clicked();
    void updateSelf(const GDBMI::Value& thread, bool initial = false);

    void fetchMoreChildren();
    void fetchMoreChildren_1(bool clear);
    void handleFrameList(const GDBMI::ResultRecord& r);

    DebugSession* session_;
    int id_;

    static const int step = 20;
};

class Frame : public KDevelop::FrameItem
{
    Q_OBJECT
public:
    Frame(KDevelop::FramesModel* model, Thread* parent, const GDBMI::Value& frame);
    void updateSelf(const GDBMI::Value& frame);

    Thread* thread() { return static_cast<Thread*>(parentItem); }

    void fetchMoreChildren() {}

private:
    int id_;
};

class StackManager : public KDevelop::StackModel
{
    Q_OBJECT

public:

    StackManager(DebugSession* session);
    virtual ~StackManager();

    virtual void update();

Q_SIGNALS:
    void selectThread(const QModelIndex& index);   

public:
    // FIXME: there should be some other way for model
    // to make a given thread selected.
    void selectThreadReally(const QModelIndex& index)
    { emit selectThread(index); }

public Q_SLOTS:
    void slotEvent(event_t e);

private:
    DebugSession* session_;
    class DebugUniverse *universe_;
};

}


#endif // STACKMANAGER_H

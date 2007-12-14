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

#include "stackmanager.h"

#include <klocale.h>
#include <kdebug.h>
#include <kglobalsettings.h>

#include "gdbparser.h"
#include "gdbcommand.h"
#include "gdbcontroller.h"
#include "threaditem.h"
#include "framestackitem.h"

//#include "modeltest.h"

using namespace GDBMI;
using namespace GDBDebugger;

StackManager::StackManager(GDBController* controller)
    : QAbstractItemModel(controller)
    , m_ignoreOneFetch(false)
{
    //new ModelTest(this);
    
    // FIXME: maybe, all debugger components should derive from
    // a base class that does this connect.
    connect(controller, SIGNAL(event(event_t)),
            this,       SLOT(slotEvent(event_t)));
}

StackManager::~StackManager()
{
}

void StackManager::clear()
{
    qDeleteAll(m_threads);
    m_threads.clear();
    reset();
}

void StackManager::slotEvent(event_t e)
{
    switch(e)
    {
        case program_state_changed:

            kDebug(9012) << "Clearning framestack";
            clear();

            controller()->addCommand(
                new GDBCommand(ThreadListIds, "",
                               this, &StackManager::handleThreadList));

            break;

         case thread_or_frame_changed:
            break;

        case program_exited:
        case debugger_exited: 
            clear();
            break;

        case debugger_busy:
        case debugger_ready:
        case shared_library_loaded:
        case program_running:
        case connected_to_program:
            break;
    }
}

void StackManager::handleThreadList(const GDBMI::ResultRecord& r)
{
    // Gdb reply is: 
    //  ^done,thread-ids={thread-id="3",thread-id="2",thread-id="1"},
    // which syntactically is a tuple, but one has to access it
    // by index anyway.
    const GDBMI::TupleValue& ids = 
        dynamic_cast<const GDBMI::TupleValue&>(r["thread-ids"]);

    if (ids.results.size())
    {
        // Need to iterate over all threads to figure out where each one stands.
        // Note that this sequence of command will be executed in strict
        // sequences, so no other view can add its command in between and
        // get state for a wrong thread.

        // Really threaded program.
        for(int i = 0, e = ids.results.size(); i != e; ++i)
        {
            int threadId = ids.results[i]->value->literal().toInt();

            ThreadItem* thread = createThread(threadId);
            thread->setDirty();

            controller()->addCommand(
                new GDBCommand(ThreadSelect, threadId,
                               thread, &ThreadItem::parseThread));
        }
    }
}

ThreadItem* StackManager::createThread(int threadId)
{
    foreach (ThreadItem* thread, m_threads)
        if (thread->thread() == threadId)
            return thread;

    ThreadItem* thread = new ThreadItem(this, threadId);

    int index = 0;
    for (; index < m_threads.count(); ++index) {
        if (m_threads.at(index)->thread() > threadId)
            goto found;
    }

    index = m_threads.count();

    found:

    beginInsertRows(QModelIndex(), index, index);

    m_threads.insert(index, thread);

    endInsertRows();

    return thread;
}

GDBController* StackManager::controller() const
{
    return static_cast<GDBController*>(const_cast<QObject*>(QObject::parent()));
}

int StackManager::columnCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);
    return ColumnLast + 1;
}

int StackManager::rowCount(const QModelIndex & parent) const
{
    if (!parent.isValid())
        return m_threads.count();

    if (!parent.internalPointer()) {
        if (parent.row() < 0 || parent.row() >= m_threads.count() || parent.column() != 0)
            return 0;

        return m_threads.at(parent.row())->frames().count();
    }

    return 0;
}

QModelIndex StackManager::index(int row, int column, const QModelIndex & parent) const
{
    if (row < 0 || column < 0 || column > ColumnLast)
        return QModelIndex();

    if (!parent.isValid()) {
        if (row >= m_threads.count())
            return QModelIndex();

        return createIndex(row, column, 0);
    }

    if (parent.column() != 0)
        return QModelIndex();

    if (parent.internalPointer())
        return QModelIndex();

    if (parent.row() >= m_threads.count())
        return QModelIndex();

    ThreadItem* thread = m_threads.at(parent.row());
    if (row < thread->frames().count());
        return createIndex(row, column, thread);

    return QModelIndex();
}

QModelIndex StackManager::indexForThread(ThreadItem * thread, int column) const
{
    Q_ASSERT(m_threads.contains(thread));
    return createIndex(m_threads.indexOf(thread), column, 0);
}

QModelIndex StackManager::indexForFrame(FrameStackItem * frame, int column) const
{
    return createIndex(frame->frame(), column, frame->thread());
}

QModelIndex StackManager::parent(const QModelIndex & index) const
{
    if (!index.isValid())
        return QModelIndex();

    if (!index.internalPointer())
        return QModelIndex();

    ThreadItem* thread = static_cast<ThreadItem*>(index.internalPointer());
    int row = m_threads.indexOf(thread);
    Q_ASSERT(row != -1);
    if (row == -1)
        return QModelIndex();

    return createIndex(row, 0, 0);
}

QVariant StackManager::data(const QModelIndex & index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.internalPointer()) {
        ThreadItem* thread = static_cast<ThreadItem*>(index.internalPointer());
        Q_ASSERT(thread->frames().count() > index.row());
        FrameStackItem* frame = thread->frames().at(index.row());

        // Refresh if dirty, it will tell us when to emit dataChanged
        if (frame->isDirty())
            frame->refresh();

        /*if (index.row() == thread->frames().count() - 1) {
            //kDebug() << index.row() << role;
            //if (thread->moreFramesAvailable() && index.row() < 40)
                thread->fetchMoreFrames();
        }*/

        switch (role) {
            case Qt::DisplayRole:
                switch (index.column()) {
                    case 0:
                        return frame->frame();
                    case 1:
                        return frame->function();
                    case 2:
                        return frame->sourceString();
                }
        }

        return QVariant();
    }

    Q_ASSERT(m_threads.count() > index.row());
    ThreadItem* thread = m_threads.at(index.row());
    switch (role) {
        case Qt::DisplayRole:
            switch (index.column()) {
                case 0:
                    return i18n("Thread %1", thread->thread());
                case 1:
                    if (!thread->frames().isEmpty())
                        return thread->frames().first()->function();
                    break;
                case 2:
                    if (!thread->frames().isEmpty())
                        return thread->frames().first()->sourceString();
            }
    }

    return QVariant();
}

QVariant StackManager::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation);

    switch (role) {
        case Qt::DisplayRole:
            switch (section) {
                case 0:
                    return i18n("ID");
                case 1:
                    return i18n("Function");
                case 2:
                    return i18n("Source");
            }
    }

    return QVariant();
}

bool StackManager::canFetchMore(const QModelIndex & parent) const
{
    kDebug() << parent;
    if (!parent.isValid())
        return false;

    if (parent.internalPointer())
        return false;

    if (m_ignoreOneFetch) {
        m_ignoreOneFetch = false;
        return false;
    }

    Q_ASSERT(parent.row() < m_threads.count());
    ThreadItem* thread = m_threads.at(parent.row());
    if (thread->moreFramesAvailable())
        return true;

    return false;
}

void StackManager::fetchMore(const QModelIndex & parent)
{
    if (!parent.isValid() || parent.internalPointer() || parent.row() >= m_threads.count())
        return;

    ThreadItem* thread = m_threads.at(parent.row());
    thread->fetchMoreFrames();
}

void StackManager::prepareInsertFrames(ThreadItem* thread, int index, int endIndex)
{
    beginInsertRows(indexForThread(thread), index, endIndex);
}

void StackManager::completeInsertFrames()
{
    m_ignoreOneFetch = true;
    endInsertRows();
}

void StackManager::dataChanged(FrameStackItem * frame)
{
    emit QAbstractItemModel::dataChanged(indexForFrame(frame, ColumnContext), indexForFrame(frame, ColumnLast));
}

QObject * StackManager::objectForIndex(const QModelIndex & index) const
{
    if (!index.isValid())
        return 0;

    if (index.internalPointer()) {
        ThreadItem* thread = static_cast<ThreadItem*>(index.internalPointer());
        Q_ASSERT(index.row() < thread->frames().count());
        return thread->frames().at(index.row());
    }

    Q_ASSERT(m_threads.count() > index.row());
    return m_threads.at(index.row());
}

ThreadItem* GDBDebugger::StackManager::threadForIndex(const QModelIndex & index) const
{
    if (!index.isValid())
        return 0;

    if (index.internalPointer())
        return static_cast<ThreadItem*>(index.internalPointer());

    Q_ASSERT(m_threads.count() > index.row());
    return m_threads.at(index.row());
}

#include "stackmanager.moc"

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

namespace GDBDebugger
{

class ThreadItem;
class FrameStackItem;
class GDBController;
class TreeItem;
class TreeModel;

class StackManager : public QObject
{
    Q_OBJECT

    friend class ThreadItem;
    friend class FrameStackItem;

public:
#if 0
    enum StackColumns {
        ColumnID,
        ColumnContext,
        ColumnSource,
        ColumnLast = ColumnSource
    };
#endif

    StackManager(GDBController* controller);
    virtual ~StackManager();

    void setAutoUpdate(bool);

    GDBController* controller() const;

    TreeModel *model();

private:

    void updateThreads();

#if 0
    // Item model reimplementations
    virtual bool canFetchMore ( const QModelIndex & parent ) const;
    virtual int columnCount( const QModelIndex & parent = QModelIndex() ) const;
    virtual QVariant data( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    virtual void fetchMore ( const QModelIndex & parent );
    //virtual bool hasChildren( const QModelIndex & parent = QModelIndex() ) const;
    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    virtual QModelIndex index( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
    virtual QModelIndex parent( const QModelIndex & index ) const;
    virtual int rowCount( const QModelIndex & parent = QModelIndex() ) const;

    QObject* objectForIndex(const QModelIndex& index) const;
    ThreadItem* threadForIndex(const QModelIndex& index) const;

protected:
    void prepareInsertFrames(ThreadItem* thread, int startIndex, int endIndex);
    void completeInsertFrames();
    void dataChanged(FrameStackItem* frame);

private:
    void handleThreadList(const GDBMI::ResultRecord&);
#endif


public Q_SLOTS:
    void slotEvent(event_t e);

#if 0
private:
    void clear();
    ThreadItem* createThread(int threadId);

    QModelIndex indexForThread(ThreadItem* thread, int column = 0) const;
    QModelIndex indexForFrame(FrameStackItem* frame, int column = 0) const;

private:
    QList<ThreadItem*> m_threads;
    mutable bool m_ignoreOneFetch;
#endif
private:
    GDBController* controller_;
    bool autoUpdate_;
    TreeModel* model_;
    class DebugUniverse *universe_;
};

}

#endif // STACKMANAGER_H

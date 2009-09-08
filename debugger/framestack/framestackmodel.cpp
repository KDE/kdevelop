/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2009 Niko Sams <niko.sams@gmail.com>                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "framestackmodel.h"

#include <KDebug>
#include <KLocalizedString>
#include <KMimeType>
#include <KIcon>

#include "../../interfaces/icore.h"
#include "../../interfaces/idebugcontroller.h"
#include "../../interfaces/iprojectcontroller.h"
#include "../interfaces/isession.h"
#include "../../shell/debugcontroller.h"

namespace KDevelop {

FrameStackModel::FrameStackModel(IDebugSession *session)
    : IFrameStackModel(session), m_activeThread(-1)
{
   connect(session, SIGNAL(stateChanged(KDevelop::IDebugSession::DebuggerState)),
           SLOT(stateChanged(KDevelop::IDebugSession::DebuggerState)));
}

FrameStackModel::~FrameStackModel()
{
}

void FrameStackModel::setThreads(const QList<ThreadItem> &threads)
{
    kDebug() << threads.count();

    if (!m_threads.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, m_threads.count()-1);
        m_threads.clear();
        endRemoveRows();
    }

    bool activeThreadIsValid = false;
    int currentThreadNr = -1;
    foreach (const ThreadItem &i, threads) {
        if (m_activeThread == i.nr) {
            activeThreadIsValid = true;
            break;
        }
        if (i.isCurrent) currentThreadNr = i.nr;
    }
    if (!activeThreadIsValid) {
        setActiveThread(currentThreadNr);
    }

    if (!threads.isEmpty()) {
        beginInsertRows(QModelIndex(), 0, threads.count()-1);
        m_threads = threads;
        endInsertRows();
    }
}

QModelIndex FrameStackModel::indexForThreadNumber(int threadNumber)
{
    int i=0;
    foreach (const ThreadItem &t, m_threads) {
        if (t.nr == threadNumber) {
            return index(i, 0);
        }
        i++;
    }
    return QModelIndex();
}

void FrameStackModel::setFrames(int threadNumber, QList<FrameItem> frames)
{
    QModelIndex threadIndex = indexForThreadNumber(threadNumber);
    Q_ASSERT(threadIndex.isValid());

    if (!m_frames[threadNumber].isEmpty()) {
        beginRemoveRows(threadIndex, 0, m_frames[threadNumber].count()-1);
        m_frames[threadNumber].clear();
        endRemoveRows();
    }

    beginInsertRows(threadIndex, 0, frames.count()-1);
    m_frames[threadNumber] = frames;
    endInsertRows();
}

void FrameStackModel::insertFrames(int threadNumber, const QList<FrameItem> &frames)
{
    QModelIndex threadIndex = indexForThreadNumber(threadNumber);
    Q_ASSERT(threadIndex.isValid());

    beginInsertRows(threadIndex, m_frames[threadNumber].count()-1,
                    m_frames[threadNumber].count()+frames.count()-2);
    m_frames[threadNumber] << frames;
    endInsertRows();
}

void FrameStackModel::setHasMoreFrames(int threadNumber, bool hasMoreFrames)
{
    m_hasMoreFrames[threadNumber] = hasMoreFrames;
}

FrameStackModel::FrameItem FrameStackModel::frame(const QModelIndex& index)
{
    Q_ASSERT(index.internalId());
    Q_ASSERT(m_threads.count() >= index.internalId());
    const ThreadItem &thread = m_threads.at(index.internalId()-1);
    return m_frames[thread.nr].at(index.row());
}

QVariant FrameStackModel::data(const QModelIndex& index, int role) const
{
    if (!index.internalId()) {
        //thread
        if (m_threads.count() <= index.row()) return QVariant();
        const ThreadItem &thread = m_threads.at(index.row());
        if (index.column() == 0) {
            if (role == Qt::DisplayRole) {
                return i18nc("#thread-id at function-name or address", "#%1 at %2", thread.nr, thread.name);
            }
        }
    } else {
        //frame
        if (m_threads.count() < index.internalId()) return QVariant();
        const ThreadItem &thread = m_threads.at(index.internalId()-1);
        if (m_frames[thread.nr].count() <= index.row()) return QVariant();
        const FrameItem &frame = m_frames[thread.nr].at(index.row());
        if (index.column() == 0) {
            if (role == Qt::DisplayRole) {
                return QVariant(QString::number(frame.nr));
            }
        } else if (index.column() == 1) {
            if (role == Qt::DisplayRole) {
                return QVariant(frame.name);
            }
        } else if (index.column() == 2) {
            if (role == Qt::DisplayRole) {
                QString ret = ICore::self()->projectController()
                    ->prettyFileName(frame.file, IProjectController::FormatPlain);
                if (frame.line != -1) {
                    ret += ":" + QString::number(frame.line + 1);
                }
                return ret;
            } else if (role == Qt::DecorationRole) {
                KMimeType::Ptr p = KMimeType::findByUrl(frame.file);
                return KIcon(p->iconName());
            }
        }
    }
    return QVariant();
}

int FrameStackModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 3;
}

int FrameStackModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid()) {
        return m_threads.count();
    } else if (!parent.internalId() && parent.column() == 0) {
        if (parent.row() < m_threads.count()) {
            return m_frames[m_threads.at(parent.row()).nr].count();
        }
    }
    return 0;
}

QModelIndex FrameStackModel::parent(const QModelIndex& child) const
{
    if (!child.internalId()) {
        return QModelIndex();
    } else {
        return index(child.internalId()-1, 0);
    }
}

QModelIndex FrameStackModel::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid()) {
        Q_ASSERT(!parent.internalId());
        Q_ASSERT(parent.column() == 0);
        Q_ASSERT(parent.row() < m_threads.count());
        return createIndex(row, column, parent.row()+1);
    } else {
        return createIndex(row, column, 0);
    }
}


QVariant FrameStackModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0) {
            return i18n("Depth");
        } else if (section == 1) {
            return i18n("Function");
        } else if (section == 2) {
            return i18n("Source");
        }
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

void FrameStackModel::setActiveThread(int threadNumber)
{
    kDebug() << threadNumber;
    if (m_activeThread != threadNumber && threadNumber != -1) {
        fetchFrames(threadNumber, 0, 20);
    }
    m_activeThread = threadNumber;
}

void FrameStackModel::setActiveThread(const QModelIndex& index)
{
    Q_ASSERT(index.isValid());
    Q_ASSERT(!index.internalId());
    Q_ASSERT(index.column() == 0);
    setActiveThread(m_threads[index.row()].nr);
}


int FrameStackModel::activeThread() const
{
    return m_activeThread;
}

QModelIndex FrameStackModel::activeThreadIndex() const
{
    int i = 0;
    foreach (const ThreadItem &t, m_threads) {
        if (t.nr == activeThread()) {
            return index(i, 0);
        }
        ++i;
    }
    return QModelIndex();
}

void FrameStackModel::update()
{
    fetchThreads();
    if (m_activeThread != -1) {
        fetchFrames(m_activeThread, 0, 20);
    }
}

void FrameStackModel::stateChanged(IDebugSession::DebuggerState state)
{
    kDebug() << state;

    if (state == IDebugSession::EndedState && !ICore::self()->debugController()->currentSession()) {
        setThreads(QList<FrameStackModel::ThreadItem>());
    }

    //ignore if not current session
    if (sender() != ICore::self()->debugController()->currentSession()) return;

    if (state == IDebugSession::PausedState 
        || state == IDebugSession::StoppedState) 
    {
        update();
    }
}

// FIXME: it should be possible to fetch more frames for
// an arbitrary thread, without making it current.
void FrameStackModel::fetchMoreFrames()
{    
    if (m_activeThread != -1 && m_hasMoreFrames[m_activeThread]) {
        fetchFrames(m_activeThread,
                    m_frames[m_activeThread].count(),
                    m_frames[m_activeThread].count()-1+20);
    }
}

}

#include "framestackmodel.moc"

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

namespace KDevelop {

FrameStackModel::FrameStackModel(IDebugSession *session)
    : IFrameStackModel(session)
    , m_currentThread(-1)
    , m_currentFrame(-1)
    , m_subsequentFrameFetchOperations(0)
    , m_updateCurrentFrameOnNextFetch(false)
{
    connect(session, SIGNAL(stateChanged(KDevelop::IDebugSession::DebuggerState)), SLOT(stateChanged(KDevelop::IDebugSession::DebuggerState)));
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

    if (!frames.isEmpty()) {
        beginInsertRows(threadIndex, 0, frames.count()-1);
        m_frames[threadNumber] = frames;
        endInsertRows();
    }

    //if first frame doesn't contain debug ifnormation (no line numbers) set
    //currentPosition to the first frame with debug information
    if (m_currentThread == threadNumber && m_updateCurrentFrameOnNextFetch) {
        m_currentFrame = 0;
        foreach (const FrameItem &frame, frames) {
            if (!frame.file.isEmpty() && frame.line != -1) {
                if (session()->currentUrl() != frame.file || session()->currentLine() != frame.line) {
                    session()->setCurrentPosition(frame.file, frame.line, QString());
                }
                m_currentFrame = frame.nr;
                break;
            }
        }
        m_updateCurrentFrameOnNextFetch = false;
    }

    session()->raiseEvent(IDebugSession::thread_or_frame_changed);

    // FIXME: Ugly hack. Apparently, when rows are added, the selection
    // in the view is cleared. Emit this so that some frame is still
    // selected.
    emit currentFrameChanged(m_currentFrame);
}

void FrameStackModel::insertFrames(int threadNumber, const QList<FrameItem> &frames)
{
    QModelIndex threadIndex = indexForThreadNumber(threadNumber);
    Q_ASSERT(threadIndex.isValid());

    beginInsertRows(threadIndex, m_frames[threadNumber].count()-1,
                    m_frames[threadNumber].count()+frames.count()-1);
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
                    ret += ':' + QString::number(frame.line + 1);
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

void FrameStackModel::setCurrentThread(int threadNumber)
{
    kDebug() << threadNumber;
    if (m_currentThread != threadNumber && threadNumber != -1) {
        // FIXME: this logic means that if we switch to thread 3 and
        // then to thread 2 and then to thread 3, we'll request frames
        // for thread 3 again, even if the program was not run in between
        // and therefore frames could not have changed.
        m_currentFrame = 0; //set before fetchFrames else --frame argument would be wrong
        m_updateCurrentFrameOnNextFetch = true;
        fetchFrames(threadNumber, 0, 20);
    }
    if (threadNumber != m_currentThread) {
        m_currentFrame = 0;
        kDebug() << "currentFrame" << m_currentFrame;
        m_currentThread = threadNumber;
        emit currentFrameChanged(m_currentFrame);
    }
    kDebug() << "currentThread: " << m_currentThread << "currentFrame: " << m_currentFrame;
    emit currentThreadChanged(threadNumber);
    session()->raiseEvent(IDebugSession::thread_or_frame_changed);
}

void FrameStackModel::setCurrentThread(const QModelIndex& index)
{
    Q_ASSERT(index.isValid());
    Q_ASSERT(!index.internalId());
    Q_ASSERT(index.column() == 0);
    setCurrentThread(m_threads[index.row()].nr);
}


int FrameStackModel::currentThread() const
{
    return m_currentThread;
}

QModelIndex FrameStackModel::currentThreadIndex() const
{
    int i = 0;
    foreach (const ThreadItem &t, m_threads) {
        if (t.nr == currentThread()) {
            return index(i, 0);
        }
        ++i;
    }
    return QModelIndex();
}

int FrameStackModel::currentFrame() const
{
    return m_currentFrame;
}

QModelIndex FrameStackModel::currentFrameIndex() const
{
    QModelIndex idx = currentThreadIndex();
    return idx.child(m_currentFrame, 0);
}

void FrameStackModel::setCurrentFrame(int frame)
{
    kDebug() << frame;
    if (frame != m_currentFrame)
    {
        m_currentFrame = frame;
        session()->raiseEvent(IDebugSession::thread_or_frame_changed);
        emit currentFrameChanged(frame);
    }
}

void FrameStackModel::update()
{
    m_subsequentFrameFetchOperations = 0;
    fetchThreads();
    if (m_currentThread != -1) {
        fetchFrames(m_currentThread, 0, 20);
    }
}

void FrameStackModel::handleEvent(IDebugSession::event_t event)
{
    switch (event)
    {
    case IDebugSession::program_state_changed:
        update();
        break;

    default:
        break;
    }
}

void FrameStackModel::stateChanged(IDebugSession::DebuggerState state)
{
    if (state == IDebugSession::PausedState) {
        setCurrentFrame(-1);
        m_updateCurrentFrameOnNextFetch = true;
    } else if (state == IDebugSession::EndedState || state == IDebugSession::NotStartedState) {
        setThreads(QList<FrameStackModel::ThreadItem>());
    }
}

// FIXME: it should be possible to fetch more frames for
// an arbitrary thread, without making it current.
void FrameStackModel::fetchMoreFrames()
{
    m_subsequentFrameFetchOperations += 1;
    const int fetch = 20 * m_subsequentFrameFetchOperations * m_subsequentFrameFetchOperations;
    if (m_currentThread != -1 && m_hasMoreFrames[m_currentThread]) {
        setHasMoreFrames(m_currentThread, false);
        fetchFrames(m_currentThread,
                    m_frames[m_currentThread].count(),
                    m_frames[m_currentThread].count()-1+fetch);
    }
}

}

#include "framestackmodel.moc"

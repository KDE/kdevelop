/*
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "framestackmodel.h"

#include <QFileInfo>
#include <QIcon>
#include <QMimeType>
#include <QMimeDatabase>

#include <KLocalizedString>
#include <KColorScheme>

#include "../../interfaces/icore.h"
#include "../../interfaces/iprojectcontroller.h"
#include "../interfaces/isession.h"
#include <debug.h>

namespace KDevelop {

class FrameStackModelPrivate
{
public:
    explicit FrameStackModelPrivate(FrameStackModel* q) : q(q) {}

    void update();

    QModelIndex indexForThreadNumber(int threadNumber);

    FrameStackModel* q;

    int m_currentThread = -1;
    int m_currentFrame = -1;

    int m_crashedThreadIndex = -1;

    // used to count how often a user has scrolled down and more frames needed to be fetched;
    // this way, the number of frames fetched in each chunk can be increased if the user wants
    // to scroll far
    int m_subsequentFrameFetchOperations = 0;
    bool m_updateCurrentFrameOnNextFetch = false;

    QVector<FrameStackModel::ThreadItem> m_threads;
    QHash<int, QVector<FrameStackModel::FrameItem>> m_frames;
    QHash<int, bool> m_hasMoreFrames;

    // Caches
    mutable QHash<QString, bool> m_fileExistsCache;
};

FrameStackModel::FrameStackModel(IDebugSession *session)
    : IFrameStackModel(session)
    , d_ptr(new FrameStackModelPrivate(this))
{
    connect(session, &IDebugSession::stateChanged, this, &FrameStackModel::stateChanged);
}

FrameStackModel::~FrameStackModel()
{
}

void FrameStackModel::setThreads(const QVector<ThreadItem>& threads)
{
    Q_D(FrameStackModel);

    qCDebug(DEBUGGER) << threads.count();

    if (!d->m_threads.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, d->m_threads.count()-1);
        d->m_threads.clear();
        endRemoveRows();
    }

    if (!threads.isEmpty()) {
        beginInsertRows(QModelIndex(), 0, threads.count()-1);
        d->m_threads = threads;
        endInsertRows();
    }
}

QModelIndex FrameStackModelPrivate::indexForThreadNumber(int threadNumber)
{
    int i=0;
    for (const auto& t : std::as_const(m_threads)) {
        if (t.nr == threadNumber) {
            return q->index(i, 0);
        }
        i++;
    }
    return QModelIndex();
}

void FrameStackModel::setFrames(int threadNumber, const QVector<FrameItem>& frames)
{
    Q_D(FrameStackModel);

    QModelIndex threadIndex = d->indexForThreadNumber(threadNumber);
    Q_ASSERT(threadIndex.isValid());

    auto& threadFrames = d->m_frames[threadNumber];
    if (!threadFrames.empty()) {
        beginRemoveRows(threadIndex, 0, threadFrames.size() - 1);
        threadFrames.clear();
        endRemoveRows();
    }

    if (!frames.isEmpty()) {
        beginInsertRows(threadIndex, 0, frames.count()-1);
        threadFrames = frames;
        endInsertRows();
    }

    if (d->m_currentThread == threadNumber && d->m_updateCurrentFrameOnNextFetch) {
        d->m_currentFrame = 0;
        d->m_updateCurrentFrameOnNextFetch = false;
    }

    d->m_fileExistsCache.clear();

    session()->raiseEvent(IDebugSession::thread_or_frame_changed);

    // FIXME: Ugly hack. Apparently, when rows are added, the selection
    // in the view is cleared. Emit this so that some frame is still
    // selected.
    emit currentFrameChanged(d->m_currentFrame);
}

void FrameStackModel::insertFrames(int threadNumber, const QVector<FrameItem>& frames)
{
    Q_D(FrameStackModel);

    QModelIndex threadIndex = d->indexForThreadNumber(threadNumber);
    Q_ASSERT(threadIndex.isValid());

    auto& threadFrames = d->m_frames[threadNumber];
    beginInsertRows(threadIndex, threadFrames.size() - 1, threadFrames.size() + frames.size() - 1);
    threadFrames << frames;
    endInsertRows();
}

void FrameStackModel::setHasMoreFrames(int threadNumber, bool hasMoreFrames)
{
    Q_D(FrameStackModel);

    d->m_hasMoreFrames[threadNumber] = hasMoreFrames;
}

FrameStackModel::FrameItem FrameStackModel::frame(const QModelIndex& index)
{
    Q_D(FrameStackModel);

    Q_ASSERT(index.internalId());
    Q_ASSERT(static_cast<quintptr>(d->m_threads.count()) >= index.internalId());
    const ThreadItem &thread = d->m_threads.at(index.internalId()-1);
    return d->m_frames[thread.nr].at(index.row());
}

QVector<FrameStackModel::FrameItem> FrameStackModel::frames(int threadNumber) const
{
    Q_D(const FrameStackModel);

    return d->m_frames.value(threadNumber);
}

QVariant FrameStackModel::data(const QModelIndex& index, int role) const
{
    Q_D(const FrameStackModel);

    if (!index.internalId()) {
        //thread
        if (d->m_threads.count() <= index.row()) return QVariant();
        const ThreadItem &thread = d->m_threads.at(index.row());
        if (index.column() == 0) {
            if (role == Qt::DisplayRole) {
                if (thread.nr == d->m_crashedThreadIndex) {
                    return i18nc("#thread-id at function-name or address", "#%1 at %2 (crashed)", thread.nr, thread.name);
                } else {
                    return i18nc("#thread-id at function-name or address", "#%1 at %2", thread.nr, thread.name);
                }
            } else if (role == Qt::ForegroundRole) {
                if (thread.nr == d->m_crashedThreadIndex) {
                    KColorScheme scheme(QPalette::Active);
                    return scheme.foreground(KColorScheme::NegativeText).color();
                }
            }
        }
    } else {
        //frame
        if (static_cast<quintptr>(d->m_threads.count()) < index.internalId()) return QVariant();
        const ThreadItem &thread = d->m_threads.at(index.internalId()-1);
        const auto& threadFrames = d->m_frames[thread.nr];
        if (index.row() >= threadFrames.size())
            return QVariant();
        const FrameItem& frame = threadFrames.at(index.row());
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
                    ret += QLatin1Char(':') + QString::number(frame.line + 1);
                }
                return ret;
            } else if (role == Qt::DecorationRole) {
                QMimeType mime = QMimeDatabase().mimeTypeForUrl(frame.file);
                return QIcon::fromTheme(mime.iconName());
            } else if (role == Qt::ForegroundRole) {
                const auto fileName = frame.file.toLocalFile();
                auto cacheIt = d->m_fileExistsCache.find(fileName);
                if (cacheIt == d->m_fileExistsCache.end()) {
                    cacheIt = d->m_fileExistsCache.insert(fileName, QFileInfo::exists(fileName));
                }
                const bool fileExists = cacheIt.value();
                if (!fileExists) {
                    KColorScheme scheme(QPalette::Active);
                    return scheme.foreground(KColorScheme::InactiveText).color();
                }
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
    Q_D(const FrameStackModel);

    if (!parent.isValid()) {
        return d->m_threads.count();
    } else if (!parent.internalId() && parent.column() == 0) {
        if (parent.row() < d->m_threads.count()) {
            return d->m_frames[d->m_threads.at(parent.row()).nr].count();
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
    Q_D(const FrameStackModel);

    if (parent.isValid()) {
        Q_ASSERT(!parent.internalId());
        Q_ASSERT(parent.column() == 0);
        Q_ASSERT(parent.row() < d->m_threads.count());
        return createIndex(row, column, parent.row()+1);
    } else {
        return createIndex(row, column);
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
    Q_D(FrameStackModel);

    qCDebug(DEBUGGER) << threadNumber;
    if (d->m_currentThread != threadNumber && threadNumber != -1) {
        // FIXME: this logic means that if we switch to thread 3 and
        // then to thread 2 and then to thread 3, we'll request frames
        // for thread 3 again, even if the program was not run in between
        // and therefore frames could not have changed.
        d->m_currentFrame = 0; //set before fetchFrames else --frame argument would be wrong
        d->m_updateCurrentFrameOnNextFetch = true;
        fetchFrames(threadNumber, 0, 20);
    }
    if (threadNumber != d->m_currentThread) {
        d->m_currentFrame = 0;
        d->m_currentThread = threadNumber;
        emit currentFrameChanged(d->m_currentFrame);
    }
    qCDebug(DEBUGGER) << "currentThread: " << d->m_currentThread << "currentFrame: " << d->m_currentFrame;
    emit currentThreadChanged(threadNumber);
    session()->raiseEvent(IDebugSession::thread_or_frame_changed);
}

void FrameStackModel::setCurrentThread(const QModelIndex& index)
{
    Q_D(const FrameStackModel);

    Q_ASSERT(index.isValid());
    Q_ASSERT(!index.internalId());
    Q_ASSERT(index.column() == 0);
    setCurrentThread(d->m_threads[index.row()].nr);
}

void FrameStackModel::setCrashedThreadIndex(int index)
{
    Q_D(FrameStackModel);

    d->m_crashedThreadIndex = index;
}

int FrameStackModel::crashedThreadIndex() const
{
    Q_D(const FrameStackModel);

    return d->m_crashedThreadIndex;
}

int FrameStackModel::currentThread() const
{
    Q_D(const FrameStackModel);

    return d->m_currentThread;
}

QModelIndex FrameStackModel::currentThreadIndex() const
{
    Q_D(const FrameStackModel);

    int i = 0;
    for (const ThreadItem& t : std::as_const(d->m_threads)) {
        if (t.nr == currentThread()) {
            return index(i, 0);
        }
        ++i;
    }
    return QModelIndex();
}

int FrameStackModel::currentFrame() const
{
    Q_D(const FrameStackModel);

    return d->m_currentFrame;
}

QModelIndex FrameStackModel::currentFrameIndex() const
{
    Q_D(const FrameStackModel);

    return index(d->m_currentFrame, 0, currentThreadIndex());
}

void FrameStackModel::setCurrentFrame(int frame)
{
    Q_D(FrameStackModel);

    qCDebug(DEBUGGER) << frame;
    if (frame != d->m_currentFrame)
    {
        d->m_currentFrame = frame;
        session()->raiseEvent(IDebugSession::thread_or_frame_changed);
        emit currentFrameChanged(frame);
    }
}

void FrameStackModelPrivate::update()
{
    m_subsequentFrameFetchOperations = 0;
    q->fetchThreads();
    if (m_currentThread != -1) {
        q->fetchFrames(m_currentThread, 0, 20);
    }
}

void FrameStackModel::handleEvent(IDebugSession::event_t event)
{
    Q_D(FrameStackModel);

    switch (event)
    {
    case IDebugSession::program_state_changed:
        d->update();
        break;

    default:
        break;
    }
}

void FrameStackModel::stateChanged(IDebugSession::DebuggerState state)
{
    Q_D(FrameStackModel);

    if (state == IDebugSession::PausedState) {
        setCurrentFrame(-1);
        d->m_updateCurrentFrameOnNextFetch = true;
    } else if (state == IDebugSession::EndedState || state == IDebugSession::NotStartedState) {
        setThreads(QVector<FrameStackModel::ThreadItem>());
    }
}

// FIXME: it should be possible to fetch more frames for
// an arbitrary thread, without making it current.
void FrameStackModel::fetchMoreFrames()
{
    Q_D(FrameStackModel);

    d->m_subsequentFrameFetchOperations += 1;
    const int fetch = 20 * d->m_subsequentFrameFetchOperations * d->m_subsequentFrameFetchOperations;
    if (d->m_currentThread != -1 && d->m_hasMoreFrames[d->m_currentThread]) {
        setHasMoreFrames(d->m_currentThread, false);
        const int frameCount = d->m_frames[d->m_currentThread].size();
        fetchFrames(d->m_currentThread, frameCount, frameCount - 1 + fetch);
    }
}

}

#include "moc_framestackmodel.cpp"

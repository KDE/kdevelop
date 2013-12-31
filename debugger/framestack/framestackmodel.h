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

#ifndef KDEVPLATFORM_FRAMESTACKMODEL_H
#define KDEVPLATFORM_FRAMESTACKMODEL_H

#include <QtCore/QModelIndex>
#include <QtCore/QHash>

#include <KDE/KUrl>

#include "../debuggerexport.h"
#include "../interfaces/idebugsession.h"
#include "../interfaces/iframestackmodel.h"

namespace KDevelop {

class DebugController;

/** FIXME: This class needs rework, since at present it is not true model.
    Client cannot just obtain frames by grabbing a thread and listing
    children. It should first setCurrentThread beforehand, and it is the
    method that will actually fetch threads. Therefore, if this model
    is submitted to plain QTreeView, it won't work at all.

    Ideally, this should hold current thread and current frame numbers,
    and only fetch the list of threads, and list of frames inside thread
    when asked for by the view.
*/
class KDEVPLATFORMDEBUGGER_EXPORT FrameStackModel : public IFrameStackModel
{
    Q_OBJECT
public:
    FrameStackModel(IDebugSession* session);
    virtual ~FrameStackModel();
    
    struct ThreadItem {
        int nr;
        QString name;
    };
    void setThreads(const QList<ThreadItem> &threads);
    /**
     * Update frames for thread @p threadNumber
     *
     * @note The currentFrame property will be set to the first frame
     *   containing debug information
     */
    void setFrames(int threadNumber, QList<FrameItem> frames);
    void insertFrames(int threadNumber, const QList<FrameItem> &frames);
    void setHasMoreFrames(int threadNumber, bool hasMoreFrames);
    FrameItem frame(const QModelIndex &index);
    QList<FrameItem> frames(int threadNumber) const;

    //ItemModel implementation
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex& child) const;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    void setCurrentThread(int threadNumber);
    void setCurrentThread(const QModelIndex &index);
    int currentThread() const;
    QModelIndex currentThreadIndex() const;

    int currentFrame() const;
    QModelIndex currentFrameIndex() const;
    void setCurrentFrame(int frame);
    
    void fetchMoreFrames();

private Q_SLOTS:
    void stateChanged(KDevelop::IDebugSession::DebuggerState state);

private:
    virtual void handleEvent(IDebugSession::event_t event);

    void update();
    QModelIndex indexForThreadNumber(int threadNumber);

    int m_currentThread;
    int m_currentFrame;
    // used to count how often a user has scrolled down and more frames needed to be fetched;
    // this way, the number of frames fetched in each chunk can be increased if the user wants
    // to scroll far
    int m_subsequentFrameFetchOperations;
    bool m_updateCurrentFrameOnNextFetch;

    QList<ThreadItem> m_threads;
    QHash<int, QList<FrameItem> > m_frames;
    QHash<int, bool> m_hasMoreFrames;
};

}

#endif

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

#include <QModelIndex>

#include <debugger/debuggerexport.h>
#include <debugger/interfaces/idebugsession.h>
#include <debugger/interfaces/iframestackmodel.h>

namespace KDevelop {
class FrameStackModelPrivate;

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
    explicit FrameStackModel(IDebugSession* session);
    ~FrameStackModel() override;
    
    struct ThreadItem {
        int nr;
        QString name;
    };

    void setThreads(const QVector<ThreadItem>& threads);
    /**
     * Update frames for thread @p threadNumber
     *
     * @note The currentFrame property will be set to the first frame
     *   containing debug information
     */
    void setFrames(int threadNumber, const QVector<FrameItem>& frames);
    void insertFrames(int threadNumber, const QVector<FrameItem>& frames);
    void setHasMoreFrames(int threadNumber, bool hasMoreFrames);
    FrameItem frame(const QModelIndex &index) override;
    QVector<FrameItem> frames(int threadNumber) const;

    //ItemModel implementation
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void setCurrentThread(int threadNumber) override;
    void setCurrentThread(const QModelIndex &index) override;
    int currentThread() const override;
    QModelIndex currentThreadIndex() const override;

    int currentFrame() const override;
    QModelIndex currentFrameIndex() const override;
    void setCurrentFrame(int frame) override;
    
    void fetchMoreFrames() override;

    void setCrashedThreadIndex(int index);
    int crashedThreadIndex() const;

private Q_SLOTS:
    void stateChanged(KDevelop::IDebugSession::DebuggerState state);

private:
    void handleEvent(IDebugSession::event_t event) override;

private:
    const QScopedPointer<class FrameStackModelPrivate> d_ptr;
    Q_DECLARE_PRIVATE(FrameStackModel)
};

}

Q_DECLARE_TYPEINFO(KDevelop::FrameStackModel::ThreadItem, Q_MOVABLE_TYPE);

#endif

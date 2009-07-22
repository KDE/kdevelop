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

#ifndef KDEV_FRAMESTACKMODEL_H
#define KDEV_FRAMESTACKMODEL_H

#include <QtCore/QModelIndex>
#include <QtCore/QHash>

#include <KUrl>

#include "../debuggerexport.h"
#include "../interfaces/idebugsession.h"

namespace KDevelop {

class DebugController;

class KDEVPLATFORMDEBUGGER_EXPORT FrameStackModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    FrameStackModel(DebugController* parent);
    virtual ~FrameStackModel();

    struct FrameItem {
        int nr;
        QString name;
        KUrl file;
        int line;
    };
    struct ThreadItem {
        int nr;
        QString name;
        bool isCurrent; ///< if the thread is the current one and selected by default
    };
    void setThreads(const QList<ThreadItem> &threads);
    void setFrames(int threadNumber, QList<FrameItem> frames);
    void insertFrames(int threadNumber, const QList<FrameItem> &frames);
    void setHasMoreFrames(int threadNumber, bool hasMoreFrames);
    FrameItem frame(const QModelIndex &index);

    //ItemModel implementation
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex& child) const;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;


    void setAutoUpdate(bool autoUpdate);
    void setActiveThread(int threadNumber);
    void setActiveThread(const QModelIndex &index);
    int activeThread() const;
    QModelIndex activeThreadIndex() const;

public Q_SLOTS:
    void fetchMoreFrames();

private Q_SLOTS:
    void stateChanged(KDevelop::IDebugSession::DebuggerState state);
    void sessionAdded(KDevelop::IDebugSession* session);

private:
    void update();
    QModelIndex indexForThreadNumber(int threadNumber);

    bool m_autoUpdate;
    int m_activeThread;

    QList<ThreadItem> m_threads;
    QHash<int, QList<FrameItem> > m_frames;
    QHash<int, bool> m_hasMoreFrames;
};

}

#endif

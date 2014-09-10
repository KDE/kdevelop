/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2009 Vladimir Prus  <ghost@cs.msu.su>                       *
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

#ifndef KDEVPLATFORM_IFRAMESTACKMODEL_H
#define KDEVPLATFORM_IFRAMESTACKMODEL_H

#include "idebugsession.h"

#include <QUrl>

#include <QtCore/QAbstractItemModel>
#include <QtCore/QString>

namespace KDevelop {
    
    class KDEVPLATFORMDEBUGGER_EXPORT IFrameStackModel : public QAbstractItemModel
    {
        Q_OBJECT    
    public:
        
        /** Stack frame */
        struct FrameItem {
            int nr;
            QString name;
            QUrl file;
            /* If -1, it means that file is not necessary a source file,
               but possibly a solib name.  */
            int line;
        };
        
        IFrameStackModel(IDebugSession *session) 
        : QAbstractItemModel(session), m_session(session) {}
        
        IDebugSession* session() const { return m_session; }
        
        /** Sets the current thread to the specified number,
           and sets the current frame to 0.  
           Note that nothing prevents us from introducing 
           setCurrentThreadAndFrame, but for all the cases when we
           switch to a different thread we want frame 0.  */
        virtual void setCurrentThread(int threadNumber) = 0;
        virtual void setCurrentThread(const QModelIndex &index) = 0;
        virtual int currentThread() const = 0;
        virtual QModelIndex currentThreadIndex() const = 0;

        /* Return the frame we wish to operate on.  This is always
           in context of the current thread.  This may be -1 if
           no frame is selected. This should only the be the case
           if the thread has no stack as such -- e.g. because it's
           running, or because it's exited.  */
        virtual int currentFrame() const = 0;
        virtual QModelIndex currentFrameIndex() const = 0;
        virtual void setCurrentFrame(int frame) = 0;
        
        virtual FrameItem frame(const QModelIndex &index) = 0;
        
        virtual void fetchThreads() = 0;
        virtual void fetchFrames(int threadNumber, int from, int to) = 0;
        virtual void fetchMoreFrames() = 0;

    Q_SIGNALS:                            
        /* FIXME: It might make for a more conscise interface if those
           two were removed, and the clients react to thread_or_frame_changed
           event and compare the current thread/frame in the framestack model
           with the one they are displaying.  */
        void currentThreadChanged(int thread);
        void currentFrameChanged(int frame);
        
    private:

        virtual void handleEvent(IDebugSession::event_t event) = 0;
        friend class IDebugSession;

        IDebugSession *m_session;
    };
}

#endif

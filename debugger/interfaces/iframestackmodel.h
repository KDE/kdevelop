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

#ifndef KDEVELOP_IFRAMESTACKMODEL_H
#define KDEVELOP_IFRAMESTACKMODEL_H

#include "idebugsession.h"

#include <KDE/KUrl>

#include <QtCore/QAbstractItemModel>
#include <QtCore/QString>

namespace KDevelop {
    
    class KDEVPLATFORMDEBUGGER_EXPORT IFrameStackModel : public QAbstractItemModel
    {
    public:
        
        /** Stack frame */
        struct FrameItem {
            int nr;
            QString name;
            KUrl file;
            /* If -1, it means that file is not necessary a source file,
               but possibly a solib name.  */
            int line;
        };
        
        IFrameStackModel(IDebugSession *session) 
        : QAbstractItemModel(session), m_session(session) {}
        
        IDebugSession* session() const { return m_session; }

        /** Called whenever a stop is detected. threadNumber is
            the thread that has directly caused the stop, via
            breakpoint or other event. It may be -1 if no
            specific thread is involved.

            Implementation can use this to make the stop thread
            active if appropriate.

            This method may not be called by clients.  */
        virtual void actOnStop(int threadNumber) = 0;
        
        virtual void setActiveThread(int threadNumber) = 0;
        virtual void setActiveThread(const QModelIndex &index) = 0;
        virtual int activeThread() const = 0;
        virtual QModelIndex activeThreadIndex() const = 0;
        
        virtual FrameItem frame(const QModelIndex &index) = 0;
        
        virtual void fetchThreads() = 0;
        virtual void fetchFrames(int threadNumber, int from, int to) = 0;
        
    public Q_SLOTS:
        // FIXME: why is this slot?
        virtual void fetchMoreFrames() = 0;
        
    private:
        IDebugSession *m_session;
    };
}

#endif

/*
 * This file is part of KDevelop
 *
 * Copyright 1999 John Birch <jbb@kdevelop.org>
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2009 Niko Sams <niko.sams@gmail.com> 
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

#ifndef KDEVELOP_FRAMESTACKWIDGET_H
#define KDEVELOP_FRAMESTACKWIDGET_H

#include <QSplitter>
#include "../../debugger/interfaces/idebugsession.h"

class QModelIndex;
class QListView;

namespace KDevelop {
class IDebugSession;
class DebugController;
class AsyncTreeView;

class FramestackWidget : public QSplitter
{
    Q_OBJECT
    public:
        FramestackWidget( DebugController* controller, QWidget *parent=0 );
        virtual ~FramestackWidget();
    Q_SIGNALS:
        void requestRaise();
    protected:
        virtual void showEvent(QShowEvent* );
        virtual void hideEvent(QHideEvent* );
    private Q_SLOTS:
        void sessionAdded(KDevelop::IDebugSession* session);
        void setThreadShown(const QModelIndex& idx);
        void checkFetchMoreFrames();
        void assignSomeThread();
        void stateChanged(KDevelop::IDebugSession::DebuggerState state);
    private:
        DebugController *m_controller;
        AsyncTreeView *mFrames;
        QListView *mThreads;
        QWidget* mThreadsWidget;
};

}

#endif // KDEVELOP_FRAMESTACKWIDGET_H

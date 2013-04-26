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

#ifndef KDEVPLATFORM_FRAMESTACKWIDGET_H
#define KDEVPLATFORM_FRAMESTACKWIDGET_H

#include <QSplitter>

#include "../debuggerexport.h"
#include <debugger/interfaces/idebugsession.h>

class QMenu;
class QTreeView;
class QModelIndex;
class QListView;

namespace KDevelop {
class IDebugController;

class KDEVPLATFORMDEBUGGER_EXPORT FramestackWidget : public QSplitter
{
    Q_OBJECT
    public:
        FramestackWidget( IDebugController* controller, QWidget *parent=0 );
        virtual ~FramestackWidget();
    Q_SIGNALS:
        void requestRaise();
    protected:
        virtual void showEvent(QShowEvent* );
        virtual void hideEvent(QHideEvent* );
    private Q_SLOTS:
        void currentSessionChanged(KDevelop::IDebugSession* session);

        void setThreadShown(const QModelIndex& idx);
        void checkFetchMoreFrames();
        void currentThreadChanged(int);
        void currentFrameChanged(int);
        void frameClicked(const QModelIndex& idx);
        void frameContextMenuRequested(const QPoint &pos);
        void copySelection();
        void selectAll();
        void sessionStateChanged(KDevelop::IDebugSession::DebuggerState state);

    private:
        IDebugSession *m_session;
        QListView *m_threads;
        QWidget *m_threadsWidget;
        QTreeView *m_frames;
        QMenu *m_framesContextMenu;
};

}

#endif // KDEVPLATFORM_FRAMESTACKWIDGET_H

/*
    SPDX-FileCopyrightText: 1999 John Birch <jbb@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_FRAMESTACKWIDGET_H
#define KDEVPLATFORM_FRAMESTACKWIDGET_H

#include <debugger/debuggerexport.h>
#include <debugger/interfaces/idebugsession.h>
#include <util/autoorientedsplitter.h>

class QMenu;
class QTreeView;
class QModelIndex;
class QListView;

namespace KDevelop {
class IDebugController;

class KDEVPLATFORMDEBUGGER_EXPORT FramestackWidget : public AutoOrientedSplitter
{
    Q_OBJECT
    public:
        explicit FramestackWidget( IDebugController* controller, QWidget *parent=nullptr );
        ~FramestackWidget() override;

    private Q_SLOTS:
        void currentSessionChanged(KDevelop::IDebugSession* session);

        void setThreadShown(const QModelIndex& current);
        void checkFetchMoreFrames();
        void currentThreadChanged(int);
        void currentFrameChanged(int);
        void frameSelectionChanged(const QModelIndex& current);
        void frameContextMenuRequested(const QPoint &pos);
        void copySelection();
        void selectAll();
        void sessionStateChanged(KDevelop::IDebugSession::DebuggerState state);

    private:
        IDebugSession *m_session;
        QListView *m_threadsListView;
        QWidget *m_threadsWidget;
        QTreeView *m_framesTreeView;
        QMenu *m_framesContextMenu;
};

}

#endif // KDEVPLATFORM_FRAMESTACKWIDGET_H

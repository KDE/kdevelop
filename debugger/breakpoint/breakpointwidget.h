/*
 * This file is part of KDevelop
 *
 * Copyright 2008 Vladimir Prus <ghost@cs.msu.su>
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

#ifndef KDEVELOP_BREAKPOINTWIDGET_H
#define KDEVELOP_BREAKPOINTWIDGET_H

#include <QtGui/QWidget>
#include "../debuggerexport.h"

class QModelIndex;
class QItemSelection;
class QTableView;
class QMenu;

namespace KDevelop {
class DebugController;
class Breakpoint;
class BreakpointDetails;

class KDEVPLATFORMDEBUGGER_EXPORT BreakpointWidget : public QWidget
{
    Q_OBJECT
public:
    BreakpointWidget(DebugController *controller, QWidget *parent);
 
protected:
    void contextMenuEvent(QContextMenuEvent* event);
    void showEvent(QShowEvent * event);

private:
    void setupPopupMenu();

    void edit(KDevelop::Breakpoint *n);

private Q_SLOTS:
#if 0
    void slotContextMenuSelect( QAction* action );
#endif
    void slotAddBlankBreakpoint();
    void slotAddBlankWatchpoint();
    void slotAddBlankReadWatchpoint();
    void slotAddBlankAccessWatchpoint();
    void slotRemoveBreakpoint();
    void slotUpdateBreakpointDetail();
    void breakpointError(KDevelop::Breakpoint *b, const QString& msg, int column);
    void breakpointHit(KDevelop::Breakpoint *b);

private:
    QTableView* table_;
    BreakpointDetails* details_;
    QMenu* popup_;
    bool firstShow_;
    DebugController *m_debugController;
};

}

#endif // KDEVELOP_BREAKPOINTWIDGET_H

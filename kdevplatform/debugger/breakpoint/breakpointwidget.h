/*
 * This file is part of KDevelop
 *
 * Copyright 2008 Vladimir Prus <ghost@cs.msu.su>
 * Copyright 2013 Vlas Puhov <vlas.puhov@mail.ru>
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

#ifndef KDEVPLATFORM_BREAKPOINTWIDGET_H
#define KDEVPLATFORM_BREAKPOINTWIDGET_H

#include <util/autoorientedsplitter.h>

#include <debugger/debuggerexport.h>

class QModelIndex;

namespace KDevelop {
class IDebugController;
class Breakpoint;
class BreakpointWidgetPrivate;

class KDEVPLATFORMDEBUGGER_EXPORT BreakpointWidget : public AutoOrientedSplitter
{
    Q_OBJECT
public:
    BreakpointWidget(IDebugController *controller, QWidget *parent);
    ~BreakpointWidget() override;
 
protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
    void showEvent(QShowEvent * event) override;

private:
    void setupPopupMenu();

    void edit(KDevelop::Breakpoint *n);

private Q_SLOTS:
    void slotAddBlankBreakpoint();
    void slotAddBlankWatchpoint();
    void slotAddBlankReadWatchpoint();
    void slotAddBlankAccessWatchpoint();
    void slotRemoveBreakpoint();
    void slotUpdateBreakpointDetail();
    void slotDataInserted(int column, const QVariant& value);
    void slotOpenFile(const QModelIndex& breakpointIdx);
    void breakpointError(int row, const QString& msg);
    void breakpointHit(int row);
    void slotDisableAllBreakpoints();
    void slotEnableAllBreakpoints();
    void slotRemoveAllBreakpoints();
    void slotPopupMenuAboutToShow();
    
private:
    const QScopedPointer<class BreakpointWidgetPrivate> d_ptr;
    Q_DECLARE_PRIVATE(BreakpointWidget)
};

}

#endif // KDEVPLATFORM_BREAKPOINTWIDGET_H

/*
    SPDX-FileCopyrightText: 2008 Vladimir Prus <ghost@cs.msu.su>
    SPDX-FileCopyrightText: 2013 Vlas Puhov <vlas.puhov@mail.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
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

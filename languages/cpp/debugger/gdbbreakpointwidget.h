/*
 * GDB Debugger Support
 *
 * Copyright 2003 John Birch <jbb@kdevelop.org>
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
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

#ifndef _GDBBreakpointWidget_H_
#define _GDBBreakpointWidget_H_

#include <QPointer>
#include <QLabel>
#include <QTableView>

#include <KTextEditor/Cursor>

#include "mi/gdbmi.h"
#include "gdbcontroller.h"

class QToolButton;
class QLabel;
class QMenu;

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

namespace KDevelop { class IDocument; }

namespace GDBDebugger
{
class Breakpoint;
class GDBTable;
class GDBController;
class CppDebuggerPlugin;

class GDBBreakpointWidget : public QTableView
{
    Q_OBJECT

public:
    GDBBreakpointWidget( CppDebuggerPlugin* plugin, GDBController* controller,
                         QWidget* parent=0 );
    virtual ~GDBBreakpointWidget();

    BreakpointController* breakpoints() const;

Q_SIGNALS:
    void requestRaise();

public Q_SLOTS:
    void reset();

    // Connected to from the editor widget:
    void slotToggleBreakpoint(const KUrl &url, const KTextEditor::Cursor& cursor);
    void slotToggleBreakpoint(const QString &filename, int lineNum);
    void slotToggleBreakpointEnabled(const QString &fileName, int lineNum);

    // Connected to from the variable widget:
    void slotToggleWatchpoint(const QString &varName);

    void slotBreakpointHit(int id);


protected:
    virtual void contextMenuEvent(QContextMenuEvent* event);

private Q_SLOTS:
    void slotRemoveBreakpoint();
    void slotRemoveAllBreakpoints();
    void slotEditBreakpoint(const QString &fileName, int lineNum);
    void slotEditBreakpoint();
    void slotAddBlankBreakpoint();
    void slotAddBlankWatchpoint();
    void slotAddBlankReadWatchpoint();
    void slotContextMenuSelect( QAction* action );

    void slotWatchpointHit(int id,
                           const QString& oldValue,
                           const QString& newValue);

    void slotMaybeEditTracing(const QModelIndex& index);

Q_SIGNALS:
    void gotoSourcePosition(const QString &fileName, int lineNum);
    // Emitted when output from yet another passed tracepoint is available.
    void tracingOutput(const QByteArray&);

private:
    void setActive();

    void handleTracingPrintf(const QStringList& s);

    void editBreakpoint(Breakpoint* bp);

private:
    CppDebuggerPlugin* plugin_;
    GDBController*  controller_;

    QMenu*   m_ctxMenu;
    QPointer<Breakpoint> m_ctxMenuBreakpoint;
    QAction* m_breakpointShow;
    QAction* m_breakpointEdit;
    QAction* m_breakpointDisable;
    QAction* m_breakpointDelete;
    QAction* m_breakpointDisableAll;
    QAction* m_breakpointEnableAll;
    QAction* m_breakpointDeleteAll;
};


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

}

#endif

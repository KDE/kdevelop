/* This file is part of the KDE project
   Copyright (C) 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2002 John Firebaugh <jfirebaugh@kde.org>
   Copyright (C) 2007 Hamish Rodda <rodda@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef BREAKPOINTCONTROLLER_H
#define BREAKPOINTCONTROLLER_H

#include <QAbstractItemModel>
#include <QSet>

#include <KUrl>
#include <KConfig>
#include <KConfigGroup>
#include <KParts/Part>
#include <KTextEditor/Cursor>

#include "mi/gdbmi.h"
#include "gdbglobal.h"

#include "ibreakpointcontroller.h"

namespace KDevelop { class IDocument; }

namespace GDBDebugger
{

class GDBController;
class Breakpoint;
class FilePosBreakpoint;
class Watchpoint;
class Breakpoints;

/**
* Handles signals from the editor that relate to breakpoints and the execution
* point of the debugger.
* We may change, add or remove breakpoints in this class.
*/
class BreakpointController : public KDevelop::IBreakpointController
{
    Q_OBJECT

public:
    BreakpointController(GDBController* parent);
    GDBController* controller() const;

    FilePosBreakpoint* findBreakpoint(const QString& file, int line) const;
    Watchpoint* findWatchpoint(const QString& variableName) const;
    Watchpoint* findWatchpointByAddress(quint64 address) const;
    Breakpoint* findBreakpointById(int id) const;

    Breakpoint* addBreakpoint(Breakpoint *bp);
    void removeBreakpoint(Breakpoint *bp);

    QList<Breakpoint*> breakpoints() const { return m_breakpoints; }

    void removeAllBreakpoints();

public slots:
    void slotEvent(event_t);
    void slotBreakpointModified(Breakpoint* b);
    void slotBreakpointEnabledChanged(Breakpoint* b);
    void slotUpdateBreakpointMarks(KParts::Part* part);

    void slotToggleBreakpoint(const KUrl &url, const KTextEditor::Cursor& cursor);
    void slotToggleBreakpoint(const QString &filename, int lineNum);

private:
    void adjustMark(Breakpoint* bp, bool add);
    void handleBreakpointList(const GDBMI::ResultRecord& r);
    QList<Breakpoint*> m_breakpoints;
};

}

#endif

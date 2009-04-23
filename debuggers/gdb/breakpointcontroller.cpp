/* This file is part of the KDE project
   Copyright (C) 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2002 John Firebaugh <jfirebaugh@kde.org>
   Copyright (C) 2006, 2008 Vladimir Prus <ghost@cs.msu.su>
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

#include "breakpointcontroller.h"

#include "breakpoint.h"
#include "breakpoints.h"
#include "gdbcontroller.h"
#include "gdbcommand.h"

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/idocument.h>
#include <ktexteditor/markinterface.h>
#include <ktexteditor/document.h>
#include <KParts/PartManager>
#include <KDebug>

#include <debugger/util/treeitem.h>
#include <debugger/util/treemodel.h>

// #include "modeltest.h"

using namespace GDBDebugger;
using namespace GDBMI;
using namespace KTextEditor;

static int m_activeFlag = 0;

BreakpointController::BreakpointController(GDBController* parent)
    : IBreakpointController(parent, new Breakpoints(this, parent))
{
    connect(KDevelop::ICore::self()->partController(),
            SIGNAL(partAdded(KParts::Part*)),
            this,
            SLOT(slotUpdateBreakpointMarks(KParts::Part*)));

    // FIXME: maybe, all debugger components should derive from
    // a base class that does this connect.
    connect(parent,     SIGNAL(event(event_t)),
            this,       SLOT(slotEvent(event_t)));
}

GDBController * BreakpointController::controller() const
{
    return static_cast<GDBController*>(const_cast<QObject*>(QObject::parent()));
}

void BreakpointController::slotToggleBreakpoint(const KUrl& url, const KTextEditor::Cursor& cursor)
{
    slotToggleBreakpoint(url.toLocalFile(), cursor.line() + 1);
}

void BreakpointController::slotToggleBreakpoint(const QString &fileName, int lineNum)
{
    // FIXME: implement.
}

void BreakpointController::slotEvent(event_t e)
{
    switch(e)
    {
    case program_state_changed:
        {
            breakpointsItem()->update();
        }

    case connected_to_program:
        {
            kDebug(9012) << "connected to program";
            breakpointsItem()->sendMaybe();
            break;
        }
    case program_exited:
        {
        }
    case debugger_exited:
        breakpointsItem()->markOut();
        break;
    default:
        ;
    }
}

/*
void BreakpointController::slotUpdateBreakpointMarks(KParts::Part* part)
{
    if (KTextEditor::Document* doc = dynamic_cast<KTextEditor::Document*>(part))
    {
        if( !dynamic_cast<MarkInterface*>(doc))
            return;

        // When a file is loaded then we need to tell the editor (display window)
        // which lines contain a breakpoint.
        foreach (Breakpoint* breakpoint, breakpoints())
            adjustMark(breakpoint, true);
    }
}*/


#include "breakpointcontroller.moc"

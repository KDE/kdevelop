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

#include "breakpoints.h"
#include "gdbcontroller.h"
#include "breakpoint.h"
#include "gdbcommand.h"
#include <debugger/util/treemodel.h>
#include "mi/gdbmi.h"

#include <KConfigGroup>
#include <KGlobal>
#include <KSharedConfig>

namespace GDBDebugger
{

Breakpoints::Breakpoints(KDevelop::IBreakpointController *model, GDBController *controller)
    : IBreakpoints(model), controller_(controller)
{
}

void Breakpoints::update()
{
    controller_->addCommand(
        new GDBCommand(GDBMI::BreakList,
                       "",
                       this,
                       &Breakpoints::handleBreakpointList));        
}

void Breakpoints::createHelperBreakpoint()
{
    Breakpoint* n = new Breakpoint(model(), this, controller_);
    appendChild(n);
}

KDevelop::IBreakpoint* Breakpoints::addCodeBreakpoint()
{
    Breakpoint* n = new Breakpoint(model(), this, controller_,
                                         Breakpoint::CodeBreakpoint);
    insertChild(childItems.size()-1, n);
    return n;
}

KDevelop::IBreakpoint*
Breakpoints::addCodeBreakpoint(const QString& location)
{
    Breakpoint* n = new Breakpoint(model(), this, controller_,
                                         Breakpoint::CodeBreakpoint);
    insertChild(childItems.size()-1, n);
    n->setColumn(KDevelop::IBreakpoint::location_column, location);
    return n;
}

KDevelop::IBreakpoint* Breakpoints::addWatchpoint()
{
    Breakpoint* n = new Breakpoint(model(), this, controller_,
                                         Breakpoint::WriteBreakpoint);
    insertChild(childItems.size()-1, n);
    return n;
}

KDevelop::IBreakpoint* Breakpoints::addWatchpoint(const QString& expression)
{
    Breakpoint* n = new Breakpoint(model(), this, controller_,
                                         Breakpoint::WriteBreakpoint);
    insertChild(childItems.size()-1, n);
    n->setLocation(expression);
    return n;
}

KDevelop::IBreakpoint* Breakpoints::addReadWatchpoint()
{
    Breakpoint* n = new Breakpoint(model(), this, controller_,
                                         Breakpoint::ReadBreakpoint);
    insertChild(childItems.size()-1, n);

    return n;
}

void Breakpoints::handleBreakpointList(const GDBMI::ResultRecord &r)
{
    const GDBMI::Value& blist = r["BreakpointTable"]["body"];
    
    /* Remove breakpoints that are gone in GDB.  In future, we might
       want to inform the user that this happened. */
    QSet<int> present_in_gdb;
    for (int i = 0, e = blist.size(); i != e; ++i)
    {
        present_in_gdb.insert(blist[i]["number"].toInt());
    }
    
    for (int i = 0; i < childItems.size(); ++i)
    {
        Breakpoint *b = static_cast<Breakpoint*>(child(i));
        if (b->id() != -1 && !present_in_gdb.contains(b->id()))
            removeChild(i);
    }
    
    for(int i = 0, e = blist.size(); i != e; ++i)
    {
        const GDBMI::Value& mi_b = blist[i];            
        int id = mi_b["number"].toInt();
        
        Breakpoint *b = dynamic_cast<Breakpoint*>(breakpointById(id));
        if (!b)
        {
            Breakpoint::BreakpointKind kind = Breakpoint::CodeBreakpoint;
            QString type = mi_b["type"].literal();
            if (type == "watchpoint" || type == "hw watchpoint")
                kind = Breakpoint::WriteBreakpoint;
            else if (type == "read watchpoint")
                kind = Breakpoint::ReadBreakpoint;
            else if (type == "acc watchpoint")
                kind = Breakpoint::AccessBreakpoint;

            b = new Breakpoint (model(), this, controller_, kind);
            appendChild(b);
        }
        
        b->update(mi_b);
    }        
}

void Breakpoints::sendToGDB()
{
    for (int i = 0; i < childItems.size(); ++i)
    {
        Breakpoint *b = dynamic_cast<Breakpoint *>(child(i));
        Q_ASSERT(b);
        b->sendMaybe();
    }
}

void Breakpoints::load()
{
    KConfigGroup breakpoints = KGlobal::config()->group("breakpoints");
    int count = breakpoints.readEntry("number", 0);
    QVector<Breakpoint*> loaded;
    for (int i = 0; i < count; ++i)
    {
        Breakpoint *b = new Breakpoint(
            model(), this, controller_,
            breakpoints.group(QString::number(i)));
        loaded.push_back(b);
    }

    foreach (Breakpoint *b, loaded)
        appendChild(b, true);
}

}

#include "breakpoints.moc"

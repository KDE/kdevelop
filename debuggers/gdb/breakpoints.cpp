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
#include "newbreakpoint.h"
#include "gdbcommand.h"
#include "util/treemodel.h"
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
    NewBreakpoint* n = new NewBreakpoint(model(), this, controller_);
    appendChild(n);
}

KDevelop::INewBreakpoint* Breakpoints::addCodeBreakpoint()
{
    NewBreakpoint* n = new NewBreakpoint(model(), this, controller_,
                                         NewBreakpoint::code_breakpoint);
    insertChild(childItems.size()-1, n);
    return n;
}

KDevelop::INewBreakpoint* 
Breakpoints::addCodeBreakpoint(const QString& location)
{
    NewBreakpoint* n = new NewBreakpoint(model(), this, controller_,
                                         NewBreakpoint::code_breakpoint);
    insertChild(childItems.size()-1, n);
    n->setColumn(KDevelop::INewBreakpoint::location_column, location);
    return n;
}

KDevelop::INewBreakpoint* Breakpoints::addWatchpoint()
{
    NewBreakpoint* n = new NewBreakpoint(model(), this, controller_,
                                         NewBreakpoint::write_breakpoint);
    insertChild(childItems.size()-1, n);
    return n;
}

KDevelop::INewBreakpoint* Breakpoints::addWatchpoint(const QString& expression)
{
    NewBreakpoint* n = new NewBreakpoint(model(), this, controller_,
                                         NewBreakpoint::write_breakpoint);
    insertChild(childItems.size()-1, n);
    n->setLocation(expression);
    return n;
}

KDevelop::INewBreakpoint* Breakpoints::addReadWatchpoint()
{
    NewBreakpoint* n = new NewBreakpoint(model(), this, controller_,
                                         NewBreakpoint::read_breakpoint);
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
        NewBreakpoint *b = static_cast<NewBreakpoint*>(child(i));
        if (b->id() != -1 && !present_in_gdb.contains(b->id()))
            removeChild(i);
    }
    
    for(int i = 0, e = blist.size(); i != e; ++i)
    {
        const GDBMI::Value& mi_b = blist[i];            
        int id = mi_b["number"].toInt();
        
        NewBreakpoint *b = dynamic_cast<NewBreakpoint*>(breakpointById(id));
        if (!b)
        {
            NewBreakpoint::kind_t kind = NewBreakpoint::code_breakpoint;
            QString type = mi_b["type"].literal();
            if (type == "watchpoint" || type == "hw watchpoint")
                kind = NewBreakpoint::write_breakpoint;
            else if (type == "read watchpoint")
                kind = NewBreakpoint::read_breakpoint;
            else if (type == "acc watchpoint")
                kind = NewBreakpoint::access_breakpoint;

            b = new NewBreakpoint (model(), this, controller_, kind);
            appendChild(b);
        }
        
        b->update(mi_b);
    }        
}

void Breakpoints::sendToGDB()
{
    for (int i = 0; i < childItems.size(); ++i)
    {
        NewBreakpoint *b = dynamic_cast<NewBreakpoint *>(child(i));
        Q_ASSERT(b);
        b->sendMaybe();
    }
}

void Breakpoints::load()
{
    KConfigGroup breakpoints = KGlobal::config()->group("breakpoints");
    int count = breakpoints.readEntry("number", 0);
    QVector<NewBreakpoint*> loaded;
    for (int i = 0; i < count; ++i)
    {
        NewBreakpoint *b = new NewBreakpoint(
            model(), this, controller_,
            breakpoints.group(QString::number(i)));
        loaded.push_back(b);
    }

    foreach (NewBreakpoint *b, loaded)
        appendChild(b, true);
}

}

#include "breakpoints.moc"
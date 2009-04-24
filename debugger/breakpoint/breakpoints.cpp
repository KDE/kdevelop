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
   along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/>.
*/

#include "breakpoints.h"
#include "breakpoint.h"

#include <KConfigGroup>
#include <KGlobal>
#include <KSharedConfig>

using namespace KDevelop;

Breakpoints::Breakpoints(BreakpointModel *model)
 : TreeItem(model)
{}

BreakpointModel* Breakpoints::model()
{
    return static_cast<BreakpointModel*>(model_);
}

void Breakpoints::markOut()
{
    for (int i = 0; i < childItems.size(); ++i)
    {
        Breakpoint *b = dynamic_cast<Breakpoint *>(child(i));
        Q_ASSERT(b);
        b->markOut();
    }
}

void Breakpoints::save()
{
    KConfigGroup breakpoints = KGlobal::config()->group("breakpoints");
    // Note that the last item is always "click to create" item, which
    // we don't want to save.
    breakpoints.writeEntry("number", childItems.size()-1);
    for (int i = 0; i < childItems.size()-1; ++i)
    {
        Breakpoint *b = dynamic_cast<Breakpoint *>(child(i));
        KConfigGroup g = breakpoints.group(QString::number(i));
        b->save(g);
    }
}

Breakpoint *Breakpoints::breakpointById(int id)
{
    for (int i = 0; i < childItems.size(); ++i)
    {
        Breakpoint *b = static_cast<Breakpoint *>(child(i));
        if (b->id() == id)
            return b;
    }
    return NULL;
}

void Breakpoints::remove(const QModelIndex &index)
{
    Breakpoint *b = static_cast<Breakpoint *>(model()->itemForIndex(index));
    if (!b->pleaseEnterLocation()) {
        b->setDeleted();
    }
}

Breakpoint* Breakpoints::breakpoint(int row)
{
    return dynamic_cast<Breakpoint*>(child(row));
}

int Breakpoints::breakpointCount() const
{
    return childCount();
}

void KDevelop::Breakpoints::removeBreakpoint(int row)
{
    breakpoint(row)->removeSelf();
}

void Breakpoints::createHelperBreakpoint()
{
    Breakpoint* n = new Breakpoint(model(), this);
    appendChild(n);
}

Breakpoint* Breakpoints::addCodeBreakpoint()
{
    Breakpoint* n = new Breakpoint(model(), this, Breakpoint::CodeBreakpoint);
    insertChild(childItems.size()-1, n);
    return n;
}

Breakpoint*
Breakpoints::addCodeBreakpoint(const QString& location)
{
    Breakpoint* n = addCodeBreakpoint();
    n->setColumn(Breakpoint::LocationColumn, location);
    return n;
}

Breakpoint* Breakpoints::addWatchpoint()
{
    Breakpoint* n = new Breakpoint(model(), this, Breakpoint::WriteBreakpoint);
    insertChild(childItems.size()-1, n);
    return n;
}

Breakpoint* Breakpoints::addWatchpoint(const QString& expression)
{
    Breakpoint* n = addWatchpoint();
    n->setLocation(expression);
    return n;
}

Breakpoint* Breakpoints::addReadWatchpoint()
{
    Breakpoint* n = new Breakpoint(model(), this, Breakpoint::ReadBreakpoint);
    insertChild(childItems.size()-1, n);
    return n;
}

#include "breakpoints.moc"

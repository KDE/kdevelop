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

#include "ibreakpoints.h"
#include "inewbreakpoint.h"

#include <KConfigGroup>
#include <KGlobal>
#include <KSharedConfig>

using namespace KDevelop;

IBreakpoints::IBreakpoints(IBreakpointController *model)
 : TreeItem(model)
{}

void IBreakpoints::markOut()
{
    for (int i = 0; i < childItems.size(); ++i)
    {
        INewBreakpoint *b = dynamic_cast<INewBreakpoint *>(child(i));
        Q_ASSERT(b);
        b->markOut();
    }
}

void IBreakpoints::save()
{
    KConfigGroup breakpoints = KGlobal::config()->group("breakpoints");
    // Note that the last item is always "click to create" item, which
    // we don't want to save.
    breakpoints.writeEntry("number", childItems.size()-1);
    for (int i = 0; i < childItems.size()-1; ++i)
    {
        INewBreakpoint *b = dynamic_cast<INewBreakpoint *>(child(i));
        KConfigGroup g = breakpoints.group(QString::number(i));
        b->save(g);
    }
}

INewBreakpoint *IBreakpoints::breakpointById(int id)
{
    for (int i = 0; i < childItems.size(); ++i)
    {
        INewBreakpoint *b = static_cast<INewBreakpoint *>(child(i));
        if (b->id() == id)
            return b;
    }
    return NULL;
}

void IBreakpoints::remove(const QModelIndex &index)
{
    INewBreakpoint *b = static_cast<INewBreakpoint *>(model()->itemForIndex(index));
    b->setDeleted();
    b->sendMaybe();
}

#include "ibreakpoints.moc"
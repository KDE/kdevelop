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

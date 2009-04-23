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

#include "breakpoint.h"
#include "breakpoints.h"
#include "gdbcommand.h"
#include "gdbcontroller.h"

#include "mi/gdbmi.h"

#include <KConfigGroup>
#include <KLocale>
#include <KIcon>

using namespace GDBMI;
using namespace KDevelop;

namespace GDBDebugger
{

Breakpoint::Breakpoint(TreeModel *model, TreeItem *parent,
                             GDBController* controller, BreakpointKind kind)
: KDevelop::IBreakpoint(model, parent, kind), controller_(controller)
{
    setData(QVector<QVariant>() << QString() << QString() << QString() << QString() << QString());
}

Breakpoint::Breakpoint(TreeModel *model, TreeItem *parent,
                             GDBController* controller,
                             const KConfigGroup& config)
: KDevelop::IBreakpoint(model, parent, config), controller_(controller)
{
}

Breakpoint::Breakpoint(TreeModel *model, TreeItem *parent,
                             GDBController* controller)
: KDevelop::IBreakpoint(model, parent), controller_(controller)
{
}

void Breakpoint::handleConditionChanged(const GDBMI::ResultRecord &r)
{
    if (r.reason == "error")
    {
        errors_.insert(ConditionColumn);
        dirty_.remove(ConditionColumn);
        reportChange();
        static_cast<Breakpoints*>(parentItem)
            ->errorEmit(this, r["msg"].literal(), ConditionColumn);
    }
    else
    {
        /* GDB does not print the breakpoint in response to -break-condition.
           Presumably, it means that the condition is always what we want.  */
        dirty_.remove(ConditionColumn);
        reportChange();
        sendMaybe();
    }
}

void Breakpoint::handleAddressComputed(const GDBMI::ResultRecord &r)
{
    if (r.reason == "error")
    {
        errors_.insert(LocationColumn);
        dirty_.remove(LocationColumn);
        reportChange();
        static_cast<Breakpoints*>(parentItem)
            ->errorEmit(this, r["msg"].literal(), LocationColumn);
    }
    else
    {
        address_ = r["value"].literal();

        QString opt;
        if (kind_ == ReadBreakpoint)
            opt = "-r ";
        else if (kind_ == AccessBreakpoint)
            opt = "-a ";

        controller_->addCommand(
            new GDBCommand(
                BreakWatch,
                opt + QString("*%1").arg(address_),
                this, &Breakpoint::handleInserted, true));
    }
}

}

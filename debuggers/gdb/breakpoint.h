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

#ifndef BREAKPOINT_H
#define BREAKPOINT_H

#include <QSet>

#include <debugger/interfaces/ibreakpoint.h>


class KConfigGroup;
namespace GDBMI { class Value; class ResultRecord; }
namespace GDBDebugger
{
    
class GDBController;

class Breakpoint : public KDevelop::IBreakpoint
{
public:
    Breakpoint(KDevelop::TreeModel *model, KDevelop::TreeItem *parent, GDBController* controller, BreakpointKind kind);
    Breakpoint(KDevelop::TreeModel *model, KDevelop::TreeItem *parent, GDBController* controller, const KConfigGroup& config);

    /* This constructor creates a "please enter location" item, that will
       turn into real breakpoint when user types something.  */
    Breakpoint(KDevelop::TreeModel *model, KDevelop::TreeItem *parent,
                  GDBController* controller);

    virtual void sendMaybe();

private:
    void handleConditionChanged(const GDBMI::ResultRecord &r);
    void handleAddressComputed(const GDBMI::ResultRecord &r);

    friend class Breakpoints;

    GDBController* controller_;
};

}
#endif

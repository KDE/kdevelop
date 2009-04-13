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

#ifndef KDEV_IBREAKPOINTS_H
#define KDEV_IBREAKPOINTS_H

#include "../util/treeitem.h"
#include "ibreakpointcontroller.h"

namespace KDevelop
{
class IBreakpoint;
class IBreakpointController;

class KDEVPLATFORMDEBUGGER_EXPORT IBreakpoints : public KDevelop::TreeItem
{
    Q_OBJECT
public:
    IBreakpoints(IBreakpointController *model);

    void markOut();
    void sendMaybe();

    void remove(const QModelIndex &index);
    virtual void update() =0;

    virtual IBreakpoint* addCodeBreakpoint()=0;
    virtual IBreakpoint* addCodeBreakpoint(const QString& location)=0;
    virtual IBreakpoint* addWatchpoint()=0;
    virtual IBreakpoint* addWatchpoint(const QString& expression)=0;
    virtual IBreakpoint* addReadWatchpoint()=0;
    virtual void createHelperBreakpoint()=0;
    IBreakpoint *breakpointById(int id);

    IBreakpoint *breakpoint(int row);
    int breakpointCount() const;

    void errorEmit(IBreakpoint *b, const QString& message, int column) { emit error(b, message, column); }
Q_SIGNALS:
    void error(KDevelop::IBreakpoint *b, const QString& message, int column);

public Q_SLOTS:
    void save();
    virtual void load()=0;
};

}

#endif

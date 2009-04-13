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

#ifndef BREAKPOINTS_H
#define BREAKPOINTS_H

#include <debugger/interfaces/ibreakpoints.h>


class QModelIndex;

namespace GDBMI { class ResultRecord; }

namespace GDBDebugger
{
class GDBController;
class NewBreakpoint;

class Breakpoints : public KDevelop::IBreakpoints
{
    Q_OBJECT
public:
    Breakpoints(KDevelop::IBreakpointController *model, GDBController *controller);

    void sendToGDB();
    void markOut();

    void update();
    void fetchMoreChildren() {}
    virtual void createHelperBreakpoint();
    
    KDevelop::IBreakpoint* addCodeBreakpoint();
    KDevelop::IBreakpoint* addCodeBreakpoint(const QString& location);
    KDevelop::IBreakpoint* addWatchpoint();
    KDevelop::IBreakpoint* addWatchpoint(const QString& expression);
    KDevelop::IBreakpoint* addReadWatchpoint();

public slots:
    void load();

private:

    void handleBreakpointList(const GDBMI::ResultRecord &r);

    GDBController *controller_;
};

}

#endif

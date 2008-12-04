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

#ifndef IBREAKPOINTS_H
#define IBREAKPOINTS_H

#include "util/treeitem.h"
#include "ibreakpointcontroller.h"

class QModelIndex;

namespace KDevelop
{
class INewBreakpoint;
class IBreakpointController;
    
class IBreakpoints : public TreeItem
{
    Q_OBJECT
public:
    IBreakpoints(IBreakpointController *model);

    void markOut();

    void remove(const QModelIndex &index);
    virtual void update() =0;
    
    virtual INewBreakpoint* addCodeBreakpoint()=0;
    virtual INewBreakpoint* addWatchpoint()=0;
    virtual INewBreakpoint* addWatchpoint(const QString& expression)=0;
    virtual INewBreakpoint* addReadWatchpoint()=0;
    virtual void createHelperBreakpoint()=0;
    INewBreakpoint *breakpointById(int id);

    void errorEmit(INewBreakpoint *b, const QString& message, int column) { emit error(b, message, column); }
Q_SIGNALS:
    void error(INewBreakpoint *b, const QString& message, int column);

public slots:
    void save();
    virtual void load()=0;
};

}

#endif

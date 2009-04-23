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
   along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KDEV_BREAKPOINTS_H
#define KDEV_BREAKPOINTS_H

#include "../util/treeitem.h"
#include "breakpointmodel.h"

namespace KDevelop
{
class Breakpoint;

class KDEVPLATFORMDEBUGGER_EXPORT Breakpoints : public KDevelop::TreeItem
{
    Q_OBJECT
public:
    Breakpoints(BreakpointModel *model);

    void markOut();

    void remove(const QModelIndex &index);

    KDevelop::Breakpoint* addCodeBreakpoint();
    KDevelop::Breakpoint* addCodeBreakpoint(const QString& location);
    KDevelop::Breakpoint* addWatchpoint();
    KDevelop::Breakpoint* addWatchpoint(const QString& expression);
    KDevelop::Breakpoint* addReadWatchpoint();

    /**
     * Must create a "please enter location" item, that will
     * turn into real breakpoint when user types something.
     */
    virtual void createHelperBreakpoint();

    Breakpoint *breakpointById(int id);

    Breakpoint *breakpoint(int row);
    int breakpointCount() const;

    /**
     * Removed the Breakpoint from the Model. This won't do any communication with
     * the Debug-Engine to remove it there too.
     * only the Debug-Engine should call this
     */
    void removeBreakpoint(int row);

    void fetchMoreChildren() {}

    void errorEmit(Breakpoint *b, const QString& message, int column) { emit error(b, message, column); }
    
    BreakpointModel *model();
Q_SIGNALS:
    void error(KDevelop::Breakpoint *b, const QString& message, int column);

public Q_SLOTS:
    void save();
    virtual void load() {
        //TODO NIKO
    }
};

}

#endif

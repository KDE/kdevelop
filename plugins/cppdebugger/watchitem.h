/*
 * GDB Debugger Support
 *
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _WATCHITEM_H_
#define _WATCHITEM_H_

#include "abstractvariableitem.h"

namespace GDBDebugger
{

class WatchItem : public AbstractVariableItem
{
    Q_OBJECT

public:
    WatchItem(VariableCollection *parent);

    virtual QVariant data(int column, int role = Qt::DisplayRole) const;
};

class RecentItem : public AbstractVariableItem
{
    Q_OBJECT

public:
    RecentItem(VariableCollection *parent);

    virtual QVariant data(int column, int role = Qt::DisplayRole) const;
};

}

#endif

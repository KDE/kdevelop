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

#ifndef _VARIABLECOLLECTION_H_
#define _VARIABLECOLLECTION_H_

#include <QAbstractItemModel>

#include "mi/gdbmi.h"

namespace GDBDebugger
{

class GDBController;
class VariableItem;

class VariableCollection : public QAbstractItemModel
{
    Q_OBJECT

public:
    VariableCollection(GDBController* parent);
    virtual ~VariableCollection();

    GDBController* controller() const;
    
    void addItem(VariableItem* item);
    void deleteItem(VariableItem* item);

private:
    QList<VariableItem*> m_items;
};

}

#endif

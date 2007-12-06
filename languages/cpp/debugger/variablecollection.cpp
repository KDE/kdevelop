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

#include "variablecollection.h"

#include "gdbcontroller.h"
#include "variableitem.h"

using namespace GDBDebugger;

VariableCollection::VariableCollection(GDBController* parent)
    : QAbstractItemModel(parent)
{
}

VariableCollection::~ VariableCollection()
{
    qDeleteAll(m_items);
}

void VariableCollection::addItem(VariableItem * item)
{
    m_items.append(item);
}

void VariableCollection::deleteItem(VariableItem * item)
{
    int index = m_items.indexOf(item);
    Q_ASSERT(index != -1);
    delete m_items.takeAt(index);
}

GDBController * GDBDebugger::VariableCollection::controller() const
{
    return static_cast<GDBController*>(const_cast<QObject*>(parent()));
}

#include "variablecollection.moc"

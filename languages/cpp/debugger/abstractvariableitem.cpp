/*
 * GDB Debugger Support
 *
 * Copyright 1999 John Birch <jbb@kdevelop.org>
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

#include "abstractvariableitem.h"

#include "variablecollection.h"
#include "gdbcommand.h"
#include "gdbcontroller.h"

using namespace GDBDebugger;

AbstractVariableItem::AbstractVariableItem(AbstractVariableItem * parent)
    : QObject(parent)
    , m_registered(false)
    , m_childrenDirty(true)
    , m_valueDirty(true)
    , m_lastSeen(-1)
    , m_thread(-1)
    , m_frame(-1)
{
    if (parent) {
        setThread(parent->thread());
        setFrame(parent->frame());
    }
}

AbstractVariableItem::AbstractVariableItem(VariableCollection * parent)
    : QObject(parent)
    , m_registered(false)
    , m_childrenDirty(true)
    , m_valueDirty(true)
    , m_lastSeen(-1)
    , m_thread(-1)
    , m_frame(-1)
{
}

AbstractVariableItem::~ AbstractVariableItem()
{
    //Q_ASSERT(m_children.isEmpty());

    if (m_registered)
        deregisterWithGdb();
}

AbstractVariableItem * AbstractVariableItem::abstractParent() const
{
    return qobject_cast<AbstractVariableItem*>(const_cast<QObject*>(parent()));
}

VariableCollection * AbstractVariableItem::collection() const
{
    if (AbstractVariableItem* item = abstractParent())
        return item->collection();

    return qobject_cast<VariableCollection*>(const_cast<QObject*>(parent()));
}

const QList< AbstractVariableItem * > & AbstractVariableItem::children() const
{
    return m_children;
}

void AbstractVariableItem::addChild(AbstractVariableItem * item)
{
    item->registerWithGdb();

    collection()->prepareInsertItems(this, m_children.count(), 1);
    m_children.append(item);
    collection()->completeInsertItems();
}

void AbstractVariableItem::removeChild(AbstractVariableItem * item)
{
    int index = m_children.indexOf(item);
    Q_ASSERT(index != -1);
    if (index == -1)
        return;

    collection()->prepareRemoveItems(this, index, 1);
    m_children.takeAt(index);
    collection()->completeRemoveItems();

    item->deregisterWithGdb();
}

void AbstractVariableItem::registerWithGdb()
{
    Q_ASSERT(!m_registered);

    m_registered = true;
}

void AbstractVariableItem::deregisterWithGdb()
{
    Q_ASSERT(m_registered);

    m_registered = false;
}

void AbstractVariableItem::deleteChild(AbstractVariableItem * item)
{
    removeChild(item);
    delete item;
}

Qt::ItemFlags AbstractVariableItem::flags(int column) const
{
    Q_UNUSED(column);
    return Qt::ItemIsSelectable;
}

GDBController * AbstractVariableItem::controller() const
{
    return collection()->controller();
}

void AbstractVariableItem::setChildrenDirty(bool dirty)
{
    if (m_childrenDirty != dirty) {
        m_childrenDirty = dirty;
    }
}

bool AbstractVariableItem::isRegisteredWithGdb() const
{
    return m_registered;
}

AbstractVariableItem * AbstractVariableItem::abstractRoot() const
{
    if (AbstractVariableItem* item = abstractParent())
        return item->abstractRoot();

    return const_cast<AbstractVariableItem*>(this);
}

void AbstractVariableItem::setLastSeen(int serial)
{
    m_lastSeen = serial;
}

int AbstractVariableItem::lastSeen() const
{
    return m_lastSeen;
}

bool AbstractVariableItem::isChildrenDirty() const
{
    return m_childrenDirty;
}

void AbstractVariableItem::refreshChildren()
{
    setChildrenDirty(false);
}

void AbstractVariableItem::deleteAllChildren()
{
    if (!m_children.isEmpty()) {
        // TODO remove this plus destructor assert once no more rogue deletes found
        //foreach (AbstractVariableItem* child, m_children)
            //child->deleteAllChildren();

        collection()->prepareRemoveItems(this, 0, m_children.count());
        qDeleteAll(m_children);
        m_children.clear();
        collection()->completeRemoveItems();
    }
}

bool AbstractVariableItem::hasChildren() const
{
    // May have children if still dirty
    return !m_children.isEmpty() || isDirty();
}

int AbstractVariableItem::depth() const
{
    if (AbstractVariableItem* item = abstractParent())
        return item->depth() + 1;

    return 0;
}

int AbstractVariableItem::thread() const
{
    return m_thread;
}

int AbstractVariableItem::frame() const
{
    return m_frame;
}

void AbstractVariableItem::setThread(int thread)
{
    m_thread = thread;

    foreach (AbstractVariableItem* child, children())
        child->setThread(thread);
}

void AbstractVariableItem::setFrame(int frame)
{
    m_frame = frame;

    foreach (AbstractVariableItem* child, children())
        child->setFrame(frame);
}

void AbstractVariableItem::addCommand(GDBCommand * command)
{
    command->setThread(thread());
    command->setFrame(frame());
    controller()->addCommand(command);
}

void AbstractVariableItem::addCommandUnaltered(GDBCommand * command)
{
    controller()->addCommand(command);
}

void AbstractVariableItem::dataChanged(int column) const
{
    collection()->dataChanged(const_cast<AbstractVariableItem*>(this), column);
}

bool AbstractVariableItem::isValueDirty() const
{
    return m_valueDirty;
}

void AbstractVariableItem::setValueDirty(bool dirty)
{
    if (m_valueDirty != dirty) {
        m_valueDirty = dirty;

        if (dirty) {
            dataChanged(ColumnValue);
        }
    }
}

bool AbstractVariableItem::isDirty() const
{
    return isChildrenDirty() || isValueDirty();
}

void AbstractVariableItem::setDirty(bool dirty)
{
    setChildrenDirty(dirty);
    setValueDirty(dirty);
}

void AbstractVariableItem::refresh()
{
    updateValue();
    refreshChildren();
}

void AbstractVariableItem::updateValue()
{
    setValueDirty(false);
}

#include "abstractvariableitem.moc"

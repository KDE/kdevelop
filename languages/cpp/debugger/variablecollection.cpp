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

#include <KLocale>
#include <KDebug>

#include "gdbcontroller.h"
#include "variableitem.h"
#include "frameitem.h"
#include "watchitem.h"

#include "qt4/qstringvariableitem.h"
#include "qt4/qlistvariableitem.h"

//#include "modeltest.h"

using namespace GDBDebugger;

VariableCollection::VariableCollection(GDBController* parent)
    : QAbstractItemModel(parent)
    , activeFlag_(0)
    , recentExpressions_(0)
{
    //new ModelTest(this);
}

VariableCollection::~ VariableCollection()
{
    qDeleteAll(m_items);
}

void VariableCollection::addItem(AbstractVariableItem * item)
{
    item->registerWithGdb();

    beginInsertRows(QModelIndex(), m_items.count(), m_items.count());
    m_items.append(item);
    endInsertRows();
}

void VariableCollection::deleteItem(AbstractVariableItem * item)
{
    int index = m_items.indexOf(item);
    Q_ASSERT(index != -1);
    if (index == -1)
        return;

    item->deleteAllChildren();

    beginRemoveRows(QModelIndex(), index, index);
    delete m_items.takeAt(index);
    endRemoveRows();

#if 0
    if (item->isRegisteredWithGdb())
        item->deregisterWithGdb();
#endif
}

GDBController * VariableCollection::controller() const
{
    return static_cast<GDBController*>(const_cast<QObject*>(QObject::parent()));
}

int VariableCollection::rowCount(const QModelIndex & parent) const
{
    if (!parent.isValid())
        return m_items.count();

    if (parent.column() != 0)
        return 0;

    AbstractVariableItem* item = itemForIndex(parent);
    if (!item)
        return 0;

    return item->children().count();
}

QModelIndex VariableCollection::index(int row, int column, const QModelIndex & parent) const
{
    if (row < 0 || column < 0 || column > AbstractVariableItem::ColumnLast)
        return QModelIndex();

    if (!parent.isValid()) {
        if (row >= m_items.count())
            return QModelIndex();

        return createIndex(row, column, m_items.at(row));
    }

    if (parent.column() != 0)
        return QModelIndex();

    AbstractVariableItem* item = itemForIndex(parent);
    if (item && row < item->children().count())
        return createIndex(row, column, item->children().at(row));

    return QModelIndex();
}

QModelIndex VariableCollection::indexForItem(AbstractVariableItem * item, int column) const
{
    if (!item)
        return QModelIndex();

    if (AbstractVariableItem* parent = item->abstractParent()) {
        int row = parent->children().indexOf(item);
        if (row == -1)
            return QModelIndex();

        return createIndex(row, column, item);
    }

    int row = m_items.indexOf(item);
    if (row == -1)
        return QModelIndex();

    return createIndex(row, column, item);
}

AbstractVariableItem * VariableCollection::itemForIndex(const QModelIndex & index) const
{
    return static_cast<AbstractVariableItem*>(index.internalPointer());
}

AbstractVariableItem * VariableCollection::parentForIndex(const QModelIndex & index) const
{
    if (AbstractVariableItem* item = itemForIndex(index))
        return item->abstractParent();

    return 0;
}

int VariableCollection::columnCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);
    return AbstractVariableItem::ColumnLast + 1;
}

QVariant VariableCollection::data(const QModelIndex & index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (AbstractVariableItem* item = itemForIndex(index))
        return item->data(index.column(), role);

    return "<error - no item>";
}

Qt::ItemFlags VariableCollection::flags(const QModelIndex & index) const
{
    if (AbstractVariableItem* item = itemForIndex(index))
        return item->flags(index.column());

    return 0;
}

QVariant VariableCollection::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation);

    switch (role) {
        case Qt::DisplayRole:
            switch (section) {
                case AbstractVariableItem::ColumnName:
                    return i18n("Variable");

                case AbstractVariableItem::ColumnValue:
                    return i18n("Value");

                case AbstractVariableItem::ColumnType:
                    return i18n("Type");
            }
            break;
    }

    return QVariant();
}

QModelIndex VariableCollection::parent(const QModelIndex & index) const
{
    AbstractVariableItem* parent = parentForIndex(index);
    if (!parent)
        return QModelIndex();

    AbstractVariableItem* grandParent = parent->abstractParent();
    if (!grandParent)
        return createIndex(m_items.indexOf(parent), 0, parent);

    return createIndex(grandParent->children().indexOf(parent), 0, parent);
}

bool VariableCollection::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (index.column() != AbstractVariableItem::ColumnName)
        return false;

    VariableItem* item = qobject_cast<VariableItem*>(itemForIndex(index));
    if (!item)
        return false;

    if (role == Qt::EditRole) {
        item->setVariableName(value.toString());
        return true;

    } else {
        kWarning() << "Unsupported set data role" << role;
    }

    return false;
}


void VariableCollection::slotEvent(event_t event)
{
#if 0
    switch(event)
    {
        case program_exited:
        case debugger_exited:
        {
            // Remove all locals.
            foreach (AbstractVariableItem* item, m_items) {
                // only remove frame items
                if (qobject_cast<FrameItem*>(item))
                {
                    deleteItem(item);
                }
                else
                {
                    item->deregisterWithGdb();
                }
            }
            break;
        }

        case program_state_changed:

            // Fall-through intended.

        case thread_or_frame_changed:
            {
                FrameItem *frame = currentFrame();

                frame->setDirty();
            }
            break;

        default:
            break;
    }
#endif
}

void VariableCollection::slotAddWatchVariable(const QString &watchVar)
{
    // FIXME need thread +/- frame info??
    VariableItem *varItem = new VariableItem(findWatch());
    varItem->setExpression(watchVar);
    findWatch()->addChild(varItem);
}

void VariableCollection::slotEvaluateExpression(const QString &expression)
{
    if (!recentExpressions_)
    {
        recentExpressions_ = new RecentItem(this);
        addItem(recentExpressions_);
    }

    // FIXME need thread +/- frame info??
    VariableItem *varItem = new VariableItem(recentExpressions_);
    varItem->setExpression(expression);
    varItem->setFrozen();
    addItem(varItem);
}

FrameItem *VariableCollection::findFrame(int frameNo, int threadNo) const
{
    foreach (AbstractVariableItem* item, m_items)
        if (FrameItem* frame = qobject_cast<FrameItem*>(item))
            if (frame->matchDetails(frameNo, threadNo))
                return frame;

    return 0;
}

WatchItem *VariableCollection::findWatch()
{
    foreach (AbstractVariableItem* item, m_items)
        if (WatchItem* watch = qobject_cast<WatchItem*>(item))
            return watch;

    WatchItem* item = new WatchItem(this);
    addItem(item);
    return item;
}

void VariableCollection::prepareInsertItems(AbstractVariableItem * parent, int index, int count)
{
    beginInsertRows(indexForItem(parent), index, index + count - 1);
}

void VariableCollection::completeInsertItems()
{
    endInsertRows();
}

void VariableCollection::prepareRemoveItems(AbstractVariableItem * parent, int index, int count)
{
    beginRemoveRows(indexForItem(parent), index, index + count - 1);
}

void VariableCollection::completeRemoveItems()
{
    endRemoveRows();
}

void VariableCollection::dataChanged(AbstractVariableItem * item, int column)
{
    QModelIndex index = indexForItem(item, column);
    if (index.isValid())
        emit QAbstractItemModel::dataChanged(index, index);
}

AbstractVariableItem * VariableCollection::itemForVariableObject(const QString & variableObject) const
{
    if (m_variableObjectMap.contains(variableObject))
        return m_variableObjectMap[variableObject];

    return 0;
}

void VariableCollection::addVariableObject(const QString & variableObject, AbstractVariableItem * item)
{
    m_variableObjectMap.insert(variableObject, item);
}

void VariableCollection::removeVariableObject(const QString & variableObject)
{
    m_variableObjectMap.remove(variableObject);
}

FrameItem* VariableCollection::currentFrame()
{
    FrameItem* frame = findFrame(controller()->currentFrame(), controller()->currentThread());
    if (!frame)
    {
        frame = new FrameItem(this);
        frame->setThread(controller()->currentThread());
        frame->setFrame(controller()->currentFrame());
        addItem(frame);
    }
    return frame;
}

bool VariableCollection::canFetchMore(const QModelIndex & parent) const
{
    if (AbstractVariableItem* item = itemForIndex(parent))
        if (item->isChildrenDirty() && item->hasChildren())
            return true;

    return false;
}

void VariableCollection::fetchMore(const QModelIndex & parent)
{
    if (AbstractVariableItem* item = itemForIndex(parent))
        item->refreshChildren();
}

bool VariableCollection::hasChildren(const QModelIndex & parent) const
{
    if (!parent.isValid())
        return m_items.count();

    if (parent.column() != 0)
        return false;

    if (AbstractVariableItem* item = itemForIndex(parent))
        return item->hasChildren();

    // Shouldn't hit this
    Q_ASSERT(false);
    return false;
}

VariableItem* VariableCollection::createVariableItem(const QString & type, AbstractVariableItem * parent)
{
    static QRegExp qstring("^(const)?[ ]*QString[ ]*&?$");
    if (qstring.exactMatch(type))
        return new QStringVariableItem(parent);

    static QRegExp qlist("^(const)?[ ]*QList.*");
    if (qlist.exactMatch(type))
        return new QListVariableItem(parent);

    return new VariableItem(parent);
}

#include "variablecollection.moc"

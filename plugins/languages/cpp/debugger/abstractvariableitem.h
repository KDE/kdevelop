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

#ifndef _ABSTRACTVARIABLEITEM_H_
#define _ABSTRACTVARIABLEITEM_H_

#include <QObject>

namespace GDBDebugger
{

class VariableCollection;
class GDBController;
class GDBCommand;

class AbstractVariableItem : public QObject
{
    Q_OBJECT

public:
    enum Columns {
      ColumnName = 0,
      ColumnValue = 1,
      ColumnType = 2,
      ColumnLast = ColumnType
    };

    AbstractVariableItem(VariableCollection* parent);
    AbstractVariableItem(AbstractVariableItem* parent);
    virtual ~AbstractVariableItem();

    VariableCollection* collection() const;
    GDBController* controller() const;
    AbstractVariableItem* abstractParent() const;
    AbstractVariableItem* abstractRoot() const;
    int depth() const;

    int thread() const;
    void setThread(int thread);
    int frame() const;
    void setFrame(int frame);

    virtual Qt::ItemFlags flags(int column) const;
    virtual QVariant data(int column, int role = Qt::DisplayRole ) const = 0;
    /// Use this method to notify the collection that data has changed in the given \a column.
    void dataChanged(int column) const;
    virtual bool hasChildren() const;

    const QList<AbstractVariableItem*>& children() const;
    void addChild(AbstractVariableItem* item);
    void addChild(const QString& expression, const QString& type);
    void removeChild(AbstractVariableItem* item);
    void deleteChild(AbstractVariableItem* item);
    void deleteAllChildren();

    virtual void registerWithGdb();
    virtual void deregisterWithGdb();
    bool isRegisteredWithGdb() const;

    bool isDirty() const;
    void setDirty(bool dirty = true);
    void refresh();

    bool isChildrenDirty() const;
    virtual void setChildrenDirty(bool dirty = true);
    virtual void refreshChildren();

    bool isValueDirty() const;
    virtual void setValueDirty(bool dirty = true);
    virtual void updateValue();

    /**
     * Set a serial number for the last time this item was present 
     * when the frame was last updated.
     */
    void setLastSeen(int serial);

    /**
     * Find out when the item was last present in the frame.
     */
    int lastSeen() const;

protected:
    /**
     * Queue a command with the controller, adding thread and frame information to the command.
     */
    void addCommand(GDBCommand* command);

    /**
     * Queue a command with the controller, without adding thread and frame information to the command.
     */
    void addCommandUnaltered(GDBCommand* command);

private:
    QList<AbstractVariableItem*> m_children;

    bool m_registered : 1;
    bool m_childrenDirty : 1;
    bool m_valueDirty : 1;
    int m_lastSeen;
    int m_thread, m_frame;
};

}

#endif

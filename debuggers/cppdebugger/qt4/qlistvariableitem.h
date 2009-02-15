/*
 * Qt4 item GDB Debugger Support
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

#ifndef QLISTVARIABLEITEM_H
#define QLISTVARIABLEITEM_H

#include "variableitem.h"

namespace GDBDebugger
{

class QListVariableItem : public VariableItem
{
    Q_OBJECT

public:
    QListVariableItem(AbstractVariableItem* parent);

    //virtual Qt::ItemFlags flags(int column) const;
    virtual QVariant data(int column, int role = Qt::DisplayRole ) const;
    /*virtual bool hasChildren() const;

    virtual void registerWithGdb();
    virtual void deregisterWithGdb();*/

    virtual void refresh();

private:
    void handleAttributes(const GDBMI::ResultRecord& r);
    void handleBegin(const GDBMI::ResultRecord& r);
    void handleEnd(const GDBMI::ResultRecord& r);
    void setResult(const QString& result);

    QString m_result;
    bool m_resultSet;
    int m_begin;
    int m_end;
};

}

#endif //QLISTVARIABLEITEM_H

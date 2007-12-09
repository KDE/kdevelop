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

#ifndef _FRAMEITEM_H_
#define _FRAMEITEM_H_

#include <QTime>
#include <QVector>

#include "mi/gdbmi.h"

#include "abstractvariableitem.h"

namespace GDBDebugger
{

class GDBController;
class VariableCollection;

class FrameItem : public AbstractVariableItem
{
    Q_OBJECT

public:
    FrameItem(VariableCollection *parent, int frameNo, int threadNo);
    virtual ~FrameItem();

    void refresh();

    void setFrameName(const QString &frameName);

    bool matchDetails(int frameNo, int threadNo);

    virtual QVariant data(int column, int role = Qt::DisplayRole ) const;

private:
    // Callbacks for gdb commands
    void argumentsReady(const GDBMI::ResultRecord&);
    void localsReady(const GDBMI::ResultRecord&);
    void frameIdReady(const QStringList&);
    void handleVarUpdate(const GDBMI::ResultRecord&);
    void handleEvaluateExpression(const QStringList&);
    void variablesFetchDone();
    void fetchSpecialValuesDone();

    void checkVariable(const QString& variable);

    QTime fetch_time;

    int     frameNo_;
    int     threadNo_;
    int     m_serial;

    // Frame base and code address of the current inner-most
    // frame. Needed so that if we can know when 'frame N' no longer
    // is the same as 'frame N' when this 'VarFrameRoot' was created.
    quint64 currentFrameBase;
    quint64 currentFrameCodeAddress;

    friend class VariableTree;
};

}

#endif

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

#include "qlistvariableitem.h"

#include <QVariant>

#include <KDebug>
#include <KLocale>

#include "mi/gdbmi.h"
#include "gdbcommand.h"
#include "gdbparser.h"

using namespace GDBMI;
using namespace GDBDebugger;

QListVariableItem::QListVariableItem(AbstractVariableItem * parent)
    : VariableItem(parent)
    , m_resultSet(false)
    , m_begin(0)
    , m_end(0)
{
}

void QListVariableItem::refresh()
{
    if (isChildrenDirty()) {
        // Check that this variable is in scope
        addCommand(new GDBCommand(VarShowAttributes, variableName(), this, &QListVariableItem::handleAttributes));

        setChildrenDirty(false);
    }
}

void QListVariableItem::handleAttributes(const GDBMI::ResultRecord& r)
{
    // TODO support comma separated attr list when more than 2 flags
    if (!r.hasField("attr") || r["attr"].literal() != "editable") {
        setResult("<out of scope>");
        return;
    }

    addCommand(
        new GDBCommand(DataEvaluateExpression, QString("%1.d.begin").arg(gdbExpression()),
                        this, &QListVariableItem::handleBegin));

    addCommand(
        new GDBCommand(DataEvaluateExpression, QString("%1.d.end").arg(gdbExpression()),
                        this, &QListVariableItem::handleEnd));
}

void QListVariableItem::handleBegin(const GDBMI::ResultRecord& r)
{
    if (r.hasField("value")) {
        m_begin = r["value"].toInt();
    }
}

void QListVariableItem::handleEnd(const GDBMI::ResultRecord& r)
{
    if (r.hasField("value")) {
        m_end = r["value"].toInt();

        QRegExp childParser("QList<([^>])>");
        if (!childParser.isValid())
            kWarning() << childParser.errorString();

        QString childType;
        if (childParser.indexIn(type()))
            childType = childParser.cap(1);

        kDebug() << "Child type " << childType;

        for (int i = m_begin; i < m_end; ++i)
            addChild(QString("%1.d.array[%2]").arg(gdbExpression()).arg(i), childType);

        setResult(i18n("<%1 items>", m_end - m_begin));
    }
}

void QListVariableItem::setResult(const QString& result)
{
    // On the first stop, when VariableItem was just created,
    // don't show it in red.
    if (m_resultSet)
        setHighlight(m_result != result);
    else
        clearHighlight();

    m_result = result;
    m_resultSet = true;

    dataChanged(ColumnValue);
}

QVariant QListVariableItem::data(int column, int role) const
{
    if (column == ColumnValue) {
        switch (role) {
            case Qt::DisplayRole:
                return m_result;
        }

        return QVariant();
    }

    return VariableItem::data(column, role);
}

#include "qlistvariableitem.moc"


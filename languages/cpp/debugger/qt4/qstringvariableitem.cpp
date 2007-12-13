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

#include "qstringvariableitem.h"

#include <QVariant>

#include "mi/gdbmi.h"
#include "gdbcommand.h"
#include "gdbparser.h"

using namespace GDBMI;
using namespace GDBDebugger;

QStringVariableItem::QStringVariableItem(AbstractVariableItem * parent)
    : VariableItem(parent)
    , m_resultSet(false)
{
}

void QStringVariableItem::refresh()
{
    /*addCommand(
        new ResultlessCommand(QString("print $kdev_d=%1.d")
                                .arg(gdbExpression()),
                                true / ignore error /));

    addCommand(
        new ResultlessCommand(QString("print $kdev_s=$kdev_d.size"),
                                true));

    addCommand(
        new ResultlessCommand(
            QString("print $kdev_s= ($kdev_s > 0)? ($kdev_s > 100 ? 200 : 2*$kdev_s) : 0"),
            true));

    addCommand(
        new CliCommand(DataEvaluateExpression,
                        "($kdev_s>0) ? (*((char*)&$kdev_d.data[0])@$kdev_s) : \"\"",
                        this,
                        &QStringVariableItem::handleResult,
                        true));*/
}

void QStringVariableItem::handleResult(const QStringList& lines)
{
    QString s;

    foreach (QString line, lines)
        s += line;

    if (s[0] == '$')
    {
        int i = s.indexOf('=');
        if (i != -1)
            s = s.mid(i+2);
    }

    // A hack to nicely display QStrings. The content of QString is unicode
    // for for ASCII only strings we get ascii character mixed with \000.
    // Remove those \000 now.

    // This is not very nice, becuse we're doing this unconditionally
    // and this method can be called twice: first with data that gdb sends
    // for a variable, and second after we request the string data. In theory
    // the data sent by gdb might contain \000 that should not be translated.
    //
    // What's even worse, ideally we should convert the string data from
    // gdb into a QString again, handling all other escapes and composing
    // one QChar from two characters from gdb. But to do that, we *should*
    // now if the data if generic gdb value, and result of request for string
    // data. Fixing is is for later.
    s.replace( QRegExp("\\\\000|\\\\0"), "" );

    // FIXME: for now, assume that all special representations are
    // just strings.

    QString result = GDBParser::getGDBParser()->undecorateValue(s);

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

QVariant QStringVariableItem::data(int column, int role) const
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

#include "qstringvariableitem.moc"


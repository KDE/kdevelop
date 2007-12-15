/*
 * GDB Debugger Support
 *
 * Copyright 2003 Alexander Dymo <cloudtemple@mksat.net>
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

#include "gdbtable.h"

namespace GDBDebugger {

GDBTable::GDBTable(QWidget *parent)
 : QTableView(parent)
{
}

GDBTable::~GDBTable()
{
}

void GDBTable::keyPressEvent( QKeyEvent * e )
{
    emit keyPressed(e->key());

    if (e->key() == Qt::Key_Return)
        emit returnPressed();
    else if (e->key() == Qt::Key_F2)
        emit f2Pressed();
    else if ((e->text() == QString("a")) && (e->state() == Qt::AltModifier))
    {
        emit insertPressed();
        return;
    }
    else if ((e->text() == QString("A")) && (e->state() == Qt::AltModifier))
    {
        emit insertPressed();
        return;
    }
    else if (e->key() == Qt::Key_Delete)
        emit deletePressed();

    QTableView::keyPressEvent(e);
}

}

#include "gdbtable.moc"


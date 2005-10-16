/***************************************************************************
*   Copyright (C) 2003 by Alexander Dymo                                  *
*   cloudtemple@mksat.net                                                 *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/
#include "gdbtable.h"
//Added by qt3to4:
#include <QKeyEvent>

namespace GDBDebugger {

GDBTable::GDBTable(QWidget *parent, const char *name)
 : Q3Table(parent, name)
{
}

GDBTable::GDBTable(int nr, int nc, QWidget * parent, const char * name)
 : Q3Table(nr, nc, parent, name)
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

    Q3Table::keyPressEvent(e);
}

}

#include "gdbtable.moc"


/***************************************************************************
*   Copyright (C) 2003 by Alexander Dymo                                  *
*   cloudtemple@mksat.net                                                 *
*                                                                         *
*	                                                                      *
*                          Adapted for ruby debugging                     *
*                          --------------------------                     *
*    begin                : Mon Nov 1 2004                                *
*    copyright            : (C) 2004 by Richard Dale                      *
*    email                : Richard_Dale@tipitina.demon.co.uk             *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/
#include "rdbtable.h"

namespace RDBDebugger {

RDBTable::RDBTable(QWidget *parent, const char *name)
 : QTable(parent, name)
{
}

RDBTable::RDBTable(int nr, int nc, QWidget * parent, const char * name)
 : QTable(nr, nc, parent, name)
{
}

RDBTable::~RDBTable()
{
}

void RDBTable::keyPressEvent( QKeyEvent * e )
{
    emit keyPressed(e->key());

    if (e->key() == Key_Return)
        emit returnPressed();
    else if (e->key() == Key_F2)
        emit f2Pressed();
    else if ((e->text() == QString("a")) && (e->state() == AltButton))
    {
        emit insertPressed();
        return;
    }
    else if ((e->text() == QString("A")) && (e->state() == AltButton))
    {
        emit insertPressed();
        return;
    }
    else if (e->key() == Key_Delete)
        emit deletePressed();

    QTable::keyPressEvent(e);
}

}

#include "rdbtable.moc"


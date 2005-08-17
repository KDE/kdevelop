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
//Added by qt3to4:
#include <QKeyEvent>

namespace RDBDebugger {

RDBTable::RDBTable(QWidget *parent, const char *name)
 : Q3Table(parent, name)
{
}

RDBTable::RDBTable(int nr, int nc, QWidget * parent, const char * name)
 : Q3Table(nr, nc, parent, name)
{
}

RDBTable::~RDBTable()
{
}

void RDBTable::keyPressEvent( QKeyEvent * e )
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

#include "rdbtable.moc"


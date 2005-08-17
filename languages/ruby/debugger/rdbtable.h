/***************************************************************************
*   Copyright (C) 2003 by Alexander Dymo                                  *
*   cloudtemple@mksat.net                                                 *
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
#ifndef RDBDEBUGGERRDBTABLE_H
#define RDBDEBUGGERRDBTABLE_H

#include <q3table.h>
//Added by qt3to4:
#include <QKeyEvent>

namespace RDBDebugger {

class RDBTable : public Q3Table
{
Q_OBJECT
public:
    RDBTable(QWidget *parent = 0, const char *name = 0);
    RDBTable( int numRows, int numCols, QWidget * parent = 0, const char * name = 0 );
    ~RDBTable();

    virtual void keyPressEvent ( QKeyEvent * e );

signals:
    void keyPressed(int key);

    void returnPressed();
    void f2Pressed();
    void insertPressed();
    void deletePressed();
};

}

#endif


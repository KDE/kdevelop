/****************************************************************************
** $Id$
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include "$APPNAMELC$.h"

int main( int argc, char ** argv ) {
    QApplication a( argc, argv );
    $APPNAME$ * mw = new $APPNAME$();
    mw->setCaption( "$APPNAME$" );
    mw->show();
    a.connect( &a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()) );
    return a.exec();
}

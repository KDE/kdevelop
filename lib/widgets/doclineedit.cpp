/***************************************************************************
 *   Copyright (C) 2003 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "doclineedit.h"

void DocLineEdit::keyReleaseEvent( QKeyEvent * e )
{
    if (e->key() == Qt::Key_Up)
    {
        emit upPressed();
    }
    else if (e->key() == Qt::Key_Down)
    {
        emit downPressed();
    }
    else if (e->key() == Qt::Key_PageUp)
    {
        emit pgupPressed();
    }
    else if (e->key() == Qt::Key_PageDown)
    {
        emit pgdownPressed();
    }
    else if (e->key() == Qt::Key_Home)
    {
        emit homePressed();
    }
    else if (e->key() == Qt::Key_End)
    {
        emit endPressed();
    }
    else
        KLineEdit::keyReleaseEvent(e);
}

#include "doclineedit.moc"

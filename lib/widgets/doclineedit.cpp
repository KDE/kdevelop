/* This file is part of the KDE project
   Copyright (C) 2003 Alexander Dymo <cloudtemple@mksat.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
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

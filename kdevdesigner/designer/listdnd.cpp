/**********************************************************************
** Copyright (C) 2002 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "listdnd.h"
#include <qwidget.h>
#include <qpainter.h>

ListDnd::ListDnd( QScrollView * eventSource, const char * name )
    : QObject( eventSource, name ),
      dragInside( FALSE ), dragDelete( TRUE ), dropConfirmed( FALSE ), dMode( Both )
{
    src = eventSource;
    src->setAcceptDrops( TRUE );
    src->installEventFilter( this );
    src->viewport()->installEventFilter( this );

    line = new QWidget( src->viewport(), 0, Qt::WStyle_NoBorder | WStyle_StaysOnTop );
    line->setBackgroundColor( Qt::black );
    line->resize( src->viewport()->width(), 2 );
    line->hide();
}

void ListDnd::setDragMode( int mode )
{
    dMode = mode;
}

int ListDnd::dragMode() const
{
    return dMode;
}

bool ListDnd::eventFilter( QObject *, QEvent * event )
{
    switch ( event->type() ) {
    case QEvent::DragEnter:
	return dragEnterEvent( (QDragEnterEvent *) event );
    case QEvent::DragLeave:
	return dragLeaveEvent( (QDragLeaveEvent *) event );
    case QEvent::DragMove:
	return dragMoveEvent( (QDragMoveEvent *) event );
    case QEvent::Drop:
	return dropEvent( (QDropEvent *) event );
    case QEvent::MouseButtonPress:
	return mousePressEvent( (QMouseEvent *) event );
    case QEvent::MouseMove:
	return mouseMoveEvent( (QMouseEvent *) event );
    default:
	break;
    }
    return FALSE;
}


bool ListDnd::dragEnterEvent( QDragEnterEvent * event )
{
    if ( dMode == None ) {
	return TRUE;
    }

    bool ok = ( ( ( dMode & Both ) == Both ) ||
		( ( dMode & Internal ) && ( event->source() == src ) ) ||
		( ( dMode & External ) && ( event->source() != src ) ) );

    if ( ok && canDecode( event ) ) {
	event->accept();
	dragInside = TRUE;
	if ( !( dMode & NullDrop ) ) {
	    updateLine( event->pos() );
	    line->show();
	}
    }
    return TRUE;
}

bool ListDnd::dragLeaveEvent( QDragLeaveEvent * )
{
    dragInside = FALSE;
    line->hide();
    return TRUE;
}

bool ListDnd::dragMoveEvent( QDragMoveEvent * event )
{
    if ( dragInside && dMode && !( dMode & NullDrop ) ) {
	updateLine( event->pos() );
    }
    return TRUE;
}

bool ListDnd::dropEvent( QDropEvent * )
{
    // do nothing
    return FALSE;
}

bool ListDnd::mousePressEvent( QMouseEvent * event )
{
    if ( event->button() == LeftButton )
	mousePressPos = event->pos();
    return FALSE;
}

bool ListDnd::mouseMoveEvent( QMouseEvent * )
{
    // do nothing
    return FALSE;
}

void ListDnd::updateLine( const QPoint & )
{
    // do nothing
}

bool ListDnd::canDecode( QDragEnterEvent * )
{
    // do nothing
    return FALSE;
}

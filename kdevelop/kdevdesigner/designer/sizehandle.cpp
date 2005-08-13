/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
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

#include <qvariant.h>  // HP-UX compiler needs this here

#include "sizehandle.h"
#include "formwindow.h"
#include "widgetfactory.h"

#include <qwidget.h>
#include <qapplication.h>
#include <qlabel.h>
#include <klocale.h>

SizeHandle::SizeHandle( FormWindow *parent, Direction d, WidgetSelection *s )
    : QWidget( parent )
{
    active = TRUE;
    setBackgroundMode( active ? PaletteText : PaletteDark );
    setFixedSize( 6, 6 );
    widget = 0;
    dir =d ;
    setMouseTracking( FALSE );
    formWindow = parent;
    sel = s;
    updateCursor();
}

void SizeHandle::updateCursor()
{
    if ( !active ) {
	setCursor( arrowCursor );
	return;
    }

    switch ( dir ) {
    case LeftTop:
	setCursor( sizeFDiagCursor );
	break;
    case Top:
	setCursor( sizeVerCursor );
	break;
    case RightTop:
	setCursor( sizeBDiagCursor );
	break;
    case Right:
	setCursor( sizeHorCursor );
	break;
    case RightBottom:
	setCursor( sizeFDiagCursor );
	break;
    case Bottom:
	setCursor( sizeVerCursor );
	break;
    case LeftBottom:
	setCursor( sizeBDiagCursor );
	break;
    case Left:
	setCursor( sizeHorCursor );
	break;
    }
}

void SizeHandle::setActive( bool a )
{
    active = a;
    setBackgroundMode( active ? PaletteText : PaletteDark );
    updateCursor();
}

void SizeHandle::setWidget( QWidget *w )
{
    widget = w;
}

void SizeHandle::paintEvent( QPaintEvent * )
{
    if ( ( (FormWindow*)parentWidget() )->currentWidget() != widget )
	return;
    QPainter p( this );
    p.setPen( blue );
    p.drawRect( 0, 0, width(), height() );
}

void SizeHandle::mousePressEvent( QMouseEvent *e )
{
    if ( !widget || e->button() != LeftButton || !active )
	return;
    oldPressPos = e->pos();
    geom = origGeom = QRect( widget->pos(), widget->size() );
}

void SizeHandle::mouseMoveEvent( QMouseEvent *e )
{
    if ( !widget || ( e->state() & LeftButton ) != LeftButton || !active )
	return;
    QPoint rp = mapFromGlobal( e->globalPos() );
    QPoint d = oldPressPos - rp;
    oldPressPos = rp;
    QPoint checkPos = widget->parentWidget()->mapFromGlobal( e->globalPos() );
    QRect pr = widget->parentWidget()->rect();

    // ##### move code around a bit to reduce duplicated code here
    switch ( dir ) {
    case LeftTop: {
	if ( checkPos.x() > pr.width() - 2 * width() || checkPos.y() > pr.height() - 2 * height() )
	    return;
	int w = geom.width() + d.x();
	geom.setWidth( w );
	w = ( w / formWindow->grid().x() ) * formWindow->grid().x();
	int h = geom.height() + d.y();
	geom.setHeight( h );
	h = ( h / formWindow->grid().y() ) * formWindow->grid().y();
	int dx = widget->width() - w;
	int dy = widget->height() - h;
	trySetGeometry( widget, widget->x() + dx, widget->y() + dy, w, h );
    } break;
    case Top: {
	if ( checkPos.y() > pr.height() - 2 * height() )
	    return;
	int h = geom.height() + d.y();
	geom.setHeight( h );
	h = ( h / formWindow->grid().y() ) * formWindow->grid().y();
	int dy = widget->height() - h;
	trySetGeometry( widget, widget->x(), widget->y() + dy, widget->width(), h );
    } break;
    case RightTop: {
	if ( checkPos.x() < 2 * width() || checkPos.y() > pr.height() - 2 * height() )
	    return;
	int h = geom.height() + d.y();
	geom.setHeight( h );
	h = ( h / formWindow->grid().y() ) * formWindow->grid().y();
	int dy = widget->height() - h;
	int w = geom.width() - d.x();
	geom.setWidth( w );
	w = ( w / formWindow->grid().x() ) * formWindow->grid().x();
	trySetGeometry( widget, widget->x(), widget->y() + dy, w, h );
    } break;
    case Right: {
	if ( checkPos.x() < 2 * width() )
	    return;
	int w = geom.width() - d.x();
	geom.setWidth( w );
	w = ( w / formWindow->grid().x() ) * formWindow->grid().x();
	tryResize( widget, w, widget->height() );
    } break;
    case RightBottom: {
	if ( checkPos.x() < 2 * width() || checkPos.y() < 2 * height() )
	    return;
	int w = geom.width() - d.x();
	geom.setWidth( w );
	w = ( w / formWindow->grid().x() ) * formWindow->grid().x();
	int h = geom.height() - d.y();
	geom.setHeight( h );
	h = ( h / formWindow->grid().y() ) * formWindow->grid().y();
	tryResize( widget, w, h );
    } break;
    case Bottom: {
	if ( checkPos.y() < 2 * height() )
	    return;
	int h = geom.height() - d.y();
	geom.setHeight( h );
	h = ( h / formWindow->grid().y() ) * formWindow->grid().y();
	tryResize( widget, widget->width(), h );
    } break;
    case LeftBottom: {
	if ( checkPos.x() > pr.width() - 2 * width() || checkPos.y() < 2 * height() )
	    return;
	int w = geom.width() + d.x();
	geom.setWidth( w );
	w = ( w / formWindow->grid().x() ) * formWindow->grid().x();
	int dx = widget->width() - w;
	int h = geom.height() - d.y();
	geom.setHeight( h );
	h = ( h / formWindow->grid().y() ) * formWindow->grid().y();
	trySetGeometry( widget, widget->x() + dx, widget->y(), w, h );
    } break;
    case Left: {
	if ( checkPos.x() > pr.width() - 2 * width() )
	    return;
	int w = geom.width() + d.x();
	geom.setWidth( w );
	w = ( w / formWindow->grid().x() ) * formWindow->grid().x();
	int dx = widget->width() - w;
	trySetGeometry( widget, widget->x() + dx, widget->y(), w, widget->height() );
    } break;
    }

    QPoint p = pos();
    sel->updateGeometry();
    oldPressPos += ( p - pos() );

    formWindow->sizePreview()->setText( i18n( "%1/%2" ).arg( widget->width() ).arg( widget->height() ) );
    formWindow->sizePreview()->adjustSize();
    QRect lg( formWindow->mapFromGlobal( e->globalPos() ) + QPoint( 16, 16 ),
	      formWindow->sizePreview()->size() );
    formWindow->checkPreviewGeometry( lg );
    formWindow->sizePreview()->setGeometry( lg );
    formWindow->sizePreview()->show();
    formWindow->sizePreview()->raise();
    if ( WidgetFactory::layoutType( widget ) != WidgetFactory::NoLayout )
	formWindow->updateChildSelections( widget );
}

void SizeHandle::mouseReleaseEvent( QMouseEvent *e )
{
    if ( e->button() != LeftButton || !active )
	return;

    formWindow->sizePreview()->hide();
    if ( geom != widget->geometry() )
	formWindow->commandHistory()->addCommand( new ResizeCommand( i18n( "Resize" ),
								     formWindow,
								     widget, origGeom,
								     widget->geometry() ) );
    formWindow->emitUpdateProperties( widget );
}

void SizeHandle::trySetGeometry( QWidget *w, int x, int y, int width, int height )
{
    int minw = QMAX( w->minimumSizeHint().width(), w->minimumSize().width() );
    minw = QMAX( minw, 2 * formWindow->grid().x() );
    int minh = QMAX( w->minimumSizeHint().height(), w->minimumSize().height() );
    minh = QMAX( minh, 2 * formWindow->grid().y() );
    if ( QMAX( minw, width ) > w->maximumWidth() ||
	 QMAX( minh, height ) > w->maximumHeight() )
	return;
    if ( width < minw && x != w->x() )
	x -= minw - width;
    if ( height < minh && y != w->y() )
	y -= minh - height;
    w->setGeometry( x, y, QMAX( minw, width ), QMAX( minh, height ) );
}

void SizeHandle::tryResize( QWidget *w, int width, int height )
{
    int minw = QMAX( w->minimumSizeHint().width(), w->minimumSize().width() );
    minw = QMAX( minw, 16 );
    int minh = QMAX( w->minimumSizeHint().height(), w->minimumSize().height() );
    minh = QMAX( minh, 16 );
    w->resize( QMAX( minw, width ), QMAX( minh, height ) );
}

// ------------------------------------------------------------------------

WidgetSelection::WidgetSelection( FormWindow *parent, QPtrDict<WidgetSelection> *selDict )
    : selectionDict( selDict )
{
    formWindow = parent;
    for ( int i = SizeHandle::LeftTop; i <= SizeHandle::Left; ++i ) {
	handles.insert( i, new SizeHandle( formWindow, (SizeHandle::Direction)i, this ) );
    }
    hide();
}

void WidgetSelection::setWidget( QWidget *w, bool updateDict )
{
    if ( !w ) {
	hide();
	if ( updateDict )
	    selectionDict->remove( wid );
	wid = 0;
	return;
    }

    wid = w;
    bool active = !wid->parentWidget() || WidgetFactory::layoutType( wid->parentWidget() ) == WidgetFactory::NoLayout;
    for ( int i = SizeHandle::LeftTop; i <= SizeHandle::Left; ++i ) {
	SizeHandle *h = handles[ i ];
	if ( h ) {
	    h->setWidget( wid );
	    h->setActive( active );
	}
    }
    updateGeometry();
    show();
    if ( updateDict )
	selectionDict->insert( w, this );
}

bool WidgetSelection::isUsed() const
{
    return wid != 0;
}

void WidgetSelection::updateGeometry()
{
    if ( !wid || !wid->parentWidget() )
	return;

    QPoint p = wid->parentWidget()->mapToGlobal( wid->pos() );
    p = formWindow->mapFromGlobal( p );
    QRect r( p, wid->size() );

    int w = 6;
    int h = 6;

    for ( int i = SizeHandle::LeftTop; i <= SizeHandle::Left; ++i ) {
	SizeHandle *hndl = handles[ i ];
	if ( !hndl )
	    continue;
	switch ( i ) {
	case SizeHandle::LeftTop:
	    hndl->move( r.x() - w / 2, r.y() - h / 2 );
	    break;
	case SizeHandle::Top:
	    hndl->move( r.x() + r.width() / 2 - w / 2, r.y() - h / 2 );
	    break;
	case SizeHandle::RightTop:
	    hndl->move( r.x() + r.width() - w / 2, r.y() - h / 2 );
	    break;
	case SizeHandle::Right:
	    hndl->move( r.x() + r.width() - w / 2, r.y() + r.height() / 2 - h / 2 );
	    break;
	case SizeHandle::RightBottom:
	    hndl->move( r.x() + r.width() - w / 2, r.y() + r.height() - h / 2 );
	    break;
	case SizeHandle::Bottom:
	    hndl->move( r.x() + r.width() / 2 - w / 2, r.y() + r.height() - h / 2 );
	    break;
	case SizeHandle::LeftBottom:
	    hndl->move( r.x() - w / 2, r.y() + r.height() - h / 2 );
	    break;
	case SizeHandle::Left:
	    hndl->move( r.x() - w / 2, r.y() + r.height() / 2 - h / 2 );
	    break;
	default:
	    break;
	}
    }
}

void WidgetSelection::hide()
{
    for ( int i = SizeHandle::LeftTop; i <= SizeHandle::Left; ++i ) {
	SizeHandle *h = handles[ i ];
	if ( h )
	    h->hide();
    }
}

void WidgetSelection::show()
{
    for ( int i = SizeHandle::LeftTop; i <= SizeHandle::Left; ++i ) {
	SizeHandle *h = handles[ i ];
	if ( h ) {
	    h->show();
	    h->raise();
	}
    }
}

void WidgetSelection::update()
{
    for ( int i = SizeHandle::LeftTop; i <= SizeHandle::Left; ++i ) {
	SizeHandle *h = handles[ i ];
	if ( h )
	    h->update();
    }
}

QWidget *WidgetSelection::widget() const
{
    return wid;
}

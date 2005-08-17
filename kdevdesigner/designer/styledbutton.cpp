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
//Added by qt3to4:
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QResizeEvent>
#include <QDragEnterEvent>
#include <QMouseEvent>
#include "styledbutton.h"
#include "formwindow.h"
#include "pixmapchooser.h"
#include <qcolordialog.h>
#include <qpalette.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qapplication.h>
#include <q3dragobject.h>
#include <qstyle.h>

StyledButton::StyledButton(QWidget* parent, const char* name)
    : Q3Button( parent, name ), pix( 0 ), spix( 0 ), s( 0 ), formWindow( 0 ), mousePressed( FALSE )
{
    setMinimumSize( minimumSizeHint() );
    setAcceptDrops( TRUE );

    connect( this, SIGNAL(clicked()), SLOT(onEditor()));

    setEditor( ColorEditor );
}

StyledButton::StyledButton( const QBrush& b, QWidget* parent, const char* name, Qt::WFlags f )
    : Q3Button( parent, name, f ), spix( 0 ), s( 0 ), formWindow( 0 )
{
    col = b.color();
    pix = b.pixmap();
    setMinimumSize( minimumSizeHint() );
}

StyledButton::~StyledButton()
{
}

void StyledButton::setEditor( EditorType e )
{
    if ( edit == e )
	return;

    edit = e;
    update();
}

StyledButton::EditorType StyledButton::editor() const
{
    return edit;
}

void StyledButton::setColor( const QColor& c )
{
    col = c;
    update();
}

void StyledButton::setPixmap( const QPixmap & pm )
{
    if ( !pm.isNull() ) {
	delete pix;
	pix = new QPixmap( pm );
    } else {
	delete pix;
	pix = 0;
    }
    scalePixmap();
}

QColor StyledButton::color() const
{
    return col;
}

QPixmap* StyledButton::pixmap() const
{
    return pix;
}

bool StyledButton::scale() const
{
    return s;
}

void StyledButton::setScale( bool on )
{
    if ( s == on )
	return;

    s = on;
    scalePixmap();
}

QSize StyledButton::sizeHint() const
{
    return QSize( 50, 25 );
}

QSize StyledButton::minimumSizeHint() const
{
    return QSize( 50, 25 );
}

void StyledButton::scalePixmap()
{
    delete spix;

    if ( pix ) {
	spix = new QPixmap( 6*width()/8, 6*height()/8 );
	QImage img = pix->convertToImage();

	spix->convertFromImage( s? img.smoothScale( 6*width()/8, 6*height()/8 ) : img );
    } else {
	spix = 0;
    }

    update();
}

void StyledButton::resizeEvent( QResizeEvent* e )
{
    scalePixmap();
    Q3Button::resizeEvent( e );
}

void StyledButton::drawButton( QPainter *paint )
{
    style().drawPrimitive(QStyle::PE_ButtonBevel, paint, rect(), colorGroup(),
			  isDown() ? QStyle::State_Sunken : QStyle::State_Raised);
    drawButtonLabel(paint);

    if (hasFocus())
	style().drawPrimitive(QStyle::PE_FocusRect, paint,
			      style().subRect(QStyle::SR_PushButtonFocusRect, this),
			      colorGroup(), QStyle::State_None);
}

void StyledButton::drawButtonLabel( QPainter *paint )
{
    QColor pen = isEnabled() ?
		 hasFocus() ? palette().active().buttonText() : palette().inactive().buttonText()
		 : palette().disabled().buttonText();
    paint->setPen( pen );

    if(!isEnabled()) {
	paint->setBrush( QBrush( colorGroup().button() ) );
    }
    else if ( edit == PixmapEditor && spix ) {
	paint->setBrush( QBrush( col, *spix ) );
	paint->setBrushOrigin( width()/8, height()/8 );
    } else
	paint->setBrush( QBrush( col ) );

    paint->drawRect( width()/8, height()/8, 6*width()/8, 6*height()/8 );
}

void StyledButton::onEditor()
{
    switch (edit) {
    case ColorEditor: {
	QColor c = QColorDialog::getColor( palette().active().background(), this );
	if ( c.isValid() ) {
	    setColor( c );
	    emit changed();
	}
    } break;
    case PixmapEditor: {
	QPixmap p;
        if ( pixmap() )
		p = qChoosePixmap( this, formWindow, *pixmap() );
        else
		p = qChoosePixmap( this, formWindow, QPixmap() );
	if ( !p.isNull() ) {
	    setPixmap( p );
	    emit changed();
	}
    } break;
    default:
	break;
    }
}

void StyledButton::mousePressEvent(QMouseEvent* e)
{
    Q3Button::mousePressEvent(e);
    mousePressed = TRUE;
    pressPos = e->pos();
}

void StyledButton::mouseMoveEvent(QMouseEvent* e)
{
    Q3Button::mouseMoveEvent( e );
#ifndef QT_NO_DRAGANDDROP
    if ( !mousePressed )
	return;
    if ( ( pressPos - e->pos() ).manhattanLength() > QApplication::startDragDistance() ) {
	if ( edit == ColorEditor ) {
	    Q3ColorDrag *drg = new Q3ColorDrag( col, this );
	    QPixmap pix( 25, 25 );
	    pix.fill( col );
	    QPainter p( &pix );
	    p.drawRect( 0, 0, pix.width(), pix.height() );
	    p.end();
	    drg->setPixmap( pix );
	    mousePressed = FALSE;
	    drg->dragCopy();
	}
	else if ( edit == PixmapEditor && pix && !pix->isNull() ) {
	    QImage img = pix->convertToImage();
	    Q3ImageDrag *drg = new Q3ImageDrag( img, this );
	    if(spix)
		drg->setPixmap( *spix );
	    mousePressed = FALSE;
	    drg->dragCopy();
	}
    }
#endif
}

#ifndef QT_NO_DRAGANDDROP
void StyledButton::dragEnterEvent( QDragEnterEvent *e )
{
    setFocus();
    if ( edit == ColorEditor && Q3ColorDrag::canDecode( e ) )
	e->accept();
    else if ( edit == PixmapEditor && Q3ImageDrag::canDecode( e ) )
	e->accept();
    else
	e->ignore();
}

void StyledButton::dragLeaveEvent( QDragLeaveEvent * )
{
    if ( hasFocus() )
	parentWidget()->setFocus();
}

void StyledButton::dragMoveEvent( QDragMoveEvent *e )
{
    if ( edit == ColorEditor && Q3ColorDrag::canDecode( e ) )
	e->accept();
    else if ( edit == PixmapEditor && Q3ImageDrag::canDecode( e ) )
	e->accept();
    else
	e->ignore();
}

void StyledButton::dropEvent( QDropEvent *e )
{
    if ( edit == ColorEditor && Q3ColorDrag::canDecode( e ) ) {
	QColor color;
	Q3ColorDrag::decode( e, color );
	setColor(color);
	emit changed();
	e->accept();
    }
    else if ( edit == PixmapEditor && Q3ImageDrag::canDecode( e ) ) {
	QImage img;
	Q3ImageDrag::decode( e, img );
	QPixmap pm;
	pm.convertFromImage(img);
	setPixmap(pm);
	emit changed();
	e->accept();
    } else {
	e->ignore();
    }
}
#endif // QT_NO_DRAGANDDROP

/* This file is part of the KDE libraries
    Copyright (C) 1997 Martin Jones (mjones@kde.org)
    Copyright (C) 2003 Alexander Dymo (cloudtemple@mksat.net)

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
#ifndef PURE_QT
#include <klocale.h>
#else
#include "qlocale.h"
#endif

#include <qpainter.h>
#include <qpixmap.h>

#include "mycolorcombo.h"

#ifndef KDE_USE_FINAL
#define STANDARD_PAL_SIZE 17

static QColor *standardPalette = 0;

static void createStandardPalette()
{
    if ( standardPalette )
    return;

    standardPalette = new QColor [STANDARD_PAL_SIZE];

    int i = 0;

    standardPalette[i++] = Qt::red;
    standardPalette[i++] = Qt::green;
    standardPalette[i++] = Qt::blue;
    standardPalette[i++] = Qt::cyan;
    standardPalette[i++] = Qt::magenta;
    standardPalette[i++] = Qt::yellow;
    standardPalette[i++] = Qt::darkRed;
    standardPalette[i++] = Qt::darkGreen;
    standardPalette[i++] = Qt::darkBlue;
    standardPalette[i++] = Qt::darkCyan;
    standardPalette[i++] = Qt::darkMagenta;
    standardPalette[i++] = Qt::darkYellow;
    standardPalette[i++] = Qt::white;
    standardPalette[i++] = Qt::lightGray;
    standardPalette[i++] = Qt::gray;
    standardPalette[i++] = Qt::darkGray;
    standardPalette[i++] = Qt::black;
}
#endif

class KColorCombo::KColorComboPrivate
{
    protected:
    friend class KColorCombo;
    KColorComboPrivate(){}
    ~KColorComboPrivate(){}
    bool showEmptyList;
};

KColorCombo::KColorCombo( QWidget *parent, const char *name )
    : QComboBox( parent, name )
{
    d=new KColorComboPrivate();
    d->showEmptyList=false;

    customColor.setRgb( 255, 255, 255 );
    internalcolor.setRgb( 255, 255, 255 );

    createStandardPalette();

    addColors();

    connect( this, SIGNAL( activated(int) ), SLOT( slotActivated(int) ) );
    connect( this, SIGNAL( highlighted(int) ), SLOT( slotHighlighted(int) ) );
}


KColorCombo::~KColorCombo()
{
    delete d;
}
/**
Sets the current color
*/
void KColorCombo::setColor( const QColor &col )
{
    internalcolor = col;
    d->showEmptyList=false;
    addColors();
}


/**
Returns the currently selected color
*/
QColor KColorCombo::color() const {
    return internalcolor;
}

void KColorCombo::resizeEvent( QResizeEvent *re )
{
    QComboBox::resizeEvent( re );

    addColors();
}

/**
Show an empty list, till the next colour is set with setColor
*/
void KColorCombo::showEmptyList()
{
    d->showEmptyList=true;
    addColors();
}

void KColorCombo::slotActivated( int index )
{
    if ( index == 0 )
    {
        if ( QColorDialog::getColor( customColor, this ) == QDialog::Accepted )
        {
            QPainter painter;
            QPen pen;
            QRect rect( 0, 0, width(), QFontMetrics(painter.font()).height()+4);
            QPixmap pixmap( rect.width(), rect.height() );

            if ( qGray( customColor.rgb() ) < 128 )
                pen.setColor( white );
            else
                pen.setColor( black );

            painter.begin( &pixmap );
            QBrush brush( customColor );
            painter.fillRect( rect, brush );
            painter.setPen( pen );
            painter.drawText( 2, QFontMetrics(painter.font()).ascent()+2, i18n("Custom...") );
            painter.end();

            changeItem( pixmap, 0 );
            pixmap.detach();
        }

        internalcolor = customColor;
    }
    else
        internalcolor = standardPalette[ index - 1 ];

    emit activated( internalcolor );
}

void KColorCombo::slotHighlighted( int index )
{
    if ( index == 0 )
        internalcolor = customColor;
    else
        internalcolor = standardPalette[ index - 1 ];

    emit highlighted( internalcolor );
}

void KColorCombo::addColors()
{
    QPainter painter;
    QPen pen;
    QRect rect( 0, 0, width(), QFontMetrics(painter.font()).height()+4 );
    QPixmap pixmap( rect.width(), rect.height() );
    int i;

    clear();
    if (d->showEmptyList) return;

    createStandardPalette();

    for ( i = 0; i < STANDARD_PAL_SIZE; i++ )
        if ( standardPalette[i] == internalcolor ) break;

    if ( i == STANDARD_PAL_SIZE )
        customColor = internalcolor;

    if ( qGray( customColor.rgb() ) < 128 )
        pen.setColor( white );
    else
        pen.setColor( black );

    painter.begin( &pixmap );
    QBrush brush( customColor );
    painter.fillRect( rect, brush );
    painter.setPen( pen );
    painter.drawText( 2, QFontMetrics(painter.font()).ascent()+2, i18n("Custom...") );
    painter.end();

    insertItem( pixmap );
    pixmap.detach();

    for ( i = 0; i < STANDARD_PAL_SIZE; i++ )
    {
        painter.begin( &pixmap );
        QBrush brush( standardPalette[i] );
        painter.fillRect( rect, brush );
        painter.end();

        insertItem( pixmap );
        pixmap.detach();

        if ( standardPalette[i] == internalcolor )
            setCurrentItem( i + 1 );
    }
}

void KColorCombo::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#ifndef PURE_QT
#include "mycolorcombo.moc"
#endif

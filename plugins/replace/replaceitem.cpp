/***************************************************************************
 *   Copyright (C) 2003 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qpainter.h>
#include <qstyle.h>
#include <qpalette.h>
#include <QCheckBox>

#include "replaceitem.h"

bool ReplaceItem::s_listview_done = false;


bool ReplaceItem::hasCheckedChildren() const
{
    ReplaceItem const * item = firstChild();
    while ( item )
    {
        if ( item->isOn() )
        {
            return true;
        }
        item = item->nextSibling();
    }
    return false;
}

void ReplaceItem::stateChange( bool state )
{
    if ( s_listview_done && justClicked() )
    {
        setChecked( state );
    }
}

void ReplaceItem::setChecked( bool checked )
{
    if ( !isFile() )	// this is a child item
    {
        if ( checked || !(parent()->hasCheckedChildren()))
        {
            if ( parent()->isOn() != checked )
            {
                parent()->_clicked = false;
                parent()->setOn( checked );
            }
        }
        return;
    }

    // this is a parent item, set self and children
    ReplaceItem * item = firstChild();
    while ( item )
    {
        if ( item->isOn() != checked )
        {
            item->_clicked = false;
            item->setOn( checked );
        }
        item = item->nextSibling();
    }
}

#if KDE_VERSION > 305
// code mostly lifted from QCheckListItem::paintCell()
void ReplaceItem::paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int align )
{
    if ( !p )
        return;

    Q3ListView *lvv = listView();
    if ( !lvv )
        return;

    ReplaceView * lv = static_cast<ReplaceView*>(lvv);

    const Qt::BackgroundMode bgmode = lv->viewport()->backgroundMode();
    const QColorGroup::ColorRole crole = QPalette::backgroundRoleFromMode( bgmode );

    if ( cg.brush( crole ) != lv->colorGroup().brush( crole ) )
        p->fillRect( 0, 0, width, height(), cg.brush( crole ) );
    else
        lv->paintEmptyArea( p, QRect( 0, 0, width, height() ) );

    QFontMetrics fm( lv->fontMetrics() );
    int boxsize = lv->style().pixelMetric(QStyle::PM_CheckListButtonSize, lv);
    int marg = lv->itemMargin();
    int r = marg;

    // Draw controller / checkbox / radiobutton ---------------------
    int styleflags = QStyle::State_None;
    if ( isOn() )
        styleflags |= QStyle::State_On;
    else
        styleflags |= QStyle::State_Off;
    if ( isSelected() )
        styleflags |= QStyle::State_Selected;
    if ( isEnabled() && lv->isEnabled() )
        styleflags |= QStyle::State_Enabled;

    int x = 0;
    int y = 0;

    x += 3;

    if ( align & Qt::AlignVCenter )
        y = ( ( height() - boxsize ) / 2 ) + marg;

    else
        y = (fm.height() + 2 + marg - boxsize) / 2;

    lv->style().drawPrimitive(QStyle::PE_CheckListIndicator, p,
                              QRect(x, y, boxsize,
                                    fm.height() + 2 + marg),
                              cg, styleflags, QStyleOption(this));

    r += boxsize + 4;

    // Draw text ----------------------------------------------------
    p->translate( r, 0 );
    p->setPen( QPen( cg.text() ) );

    QColorGroup mcg = cg;
    mcg.setColor( QColorGroup::Text, ( isFile() ? Qt::darkGreen : Qt::blue ) );
    mcg.setColor( QColorGroup::HighlightedText, ( isFile() ? Qt::darkGreen : Qt::blue ) );

    Q3ListViewItem::paintCell( p, mcg, column, width - r, align );
}
#endif

void ReplaceItem::activate( int, QPoint const & localPos )
{
    Q3ListView * lv = listView();
    QCheckBox cb(0);
    int boxsize = cb.sizeHint().width();
//that's KDE-3.1 only    int boxsize = lv->style().pixelMetric(QStyle::PM_CheckListButtonSize, lv);
    int rightside = lv->itemMargin() + boxsize + ( isFile() ? 0 : lv->treeStepSize() );

    // _lineclicked indicates if the click was on the line or in the checkbox
    _lineclicked = rightside < localPos.x();
}

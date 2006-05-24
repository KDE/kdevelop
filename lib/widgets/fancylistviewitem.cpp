/* This file is part of the KDE project
   Copyright (C) 2006 David Nolden <david.nolden.kdevelop@art-master.de>

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


#include <qpixmap.h>
#include <qapplication.h>
#include <qnamespace.h>
#include <kdeversion.h>
#include "fancylistviewitem.h"


void FancyListViewItem::init(const QString& label1, const QString& label2) {
    if( !label1.isEmpty() ) {
        setText(0, label1);
    }
}


void FancyListViewItem::setItem(int column, TextPaintItem item) {
    if( column >= int(m_items.size()) ) {
        m_items.append( item );
    }else{
        m_items[column] = item;
    }
}


void FancyListViewItem::setText ( int column, const QString & text ) {
    if( column >= int(m_items.size()) ) {
        m_items.append( TextPaintItem(text) );
    }else{
        m_items[column] = TextPaintItem(text);
    }
}


QString FancyListViewItem::text(int column) const {
    if( m_items.isEmpty() ) return "";
    return (QString)m_items[column];
}

#if defined(ISKLISTVIEWITEM) && KDE_IS_VERSION(3,4,0)

QColor FancyListViewItem::backgroundColor(int col) {
    return KListViewItem::backgroundColor(col);
}

#else

QColor FancyListViewItem::backgroundColor(int col) {
    return QColor();
}

#endif


///this is a modified version of the original QListViewItem::paintCell from the qt source
///multiline is not supported!

void FancyListViewItem::paintCell( QPainter *painter, const QColorGroup &cg,
                                                int column, int width, int align)
{
    if(column < 0 || column >= int(m_items.size()) || m_items[column].items().isEmpty()) {
        QListViewItem::paintCell(painter, cg, column, width, align);
        return;
    }
    
    painter->save();
    QColorGroup grp(cg);
    
    int styleNum = m_items[column].items()[0].style;
    TextPaintStyleStore::Item& style = m_styles.getStyle( styleNum );
    ///currently only the first background-color is used
    if( style.bgValid() ) {
        grp.setColor( QColorGroup::Base, style.background );
    }else{
        if(backgroundColor(column).isValid())
            grp.setColor( QColorGroup::Base, backgroundColor(column) ); ///use the nice kde background-color
    }

    QListView *lv = listView();
    if ( !lv )
        return;
    QPainter* p = painter;
    QFontMetrics fm( p->fontMetrics() );

    QString t = text( column );

    int marg = lv->itemMargin();
    int r = marg;
    const QPixmap * icon = pixmap( column );

    const BackgroundMode bgmode = lv->viewport()->backgroundMode();
    const QColorGroup::ColorRole crole = QPalette::backgroundRoleFromMode( bgmode );
    p->fillRect( 0, 0, width, height(), grp.brush( crole ) );
    
    if ( isSelected() && (column == 0 || lv->allColumnsShowFocus()) ) {
        p->fillRect( r - marg, 0, width - r + marg, height(), cg.brush( QColorGroup::Highlight ) );
         
        if ( isEnabled() || !lv ) 
            p->setPen( cg.highlightedText() );
        else if ( !isEnabled() && lv)
            p->setPen( lv->palette().disabled().highlightedText() );
    } 
    {
        if ( isEnabled() || !lv )
            p->setPen( cg.text() );
        else if ( !isEnabled() && lv)
            p->setPen( lv->palette().disabled().text() );


        int iconWidth = 0;

        if ( icon ) {
            iconWidth = icon->width() + lv->itemMargin();
            int xo = r;
            int yo = ( height() - icon->height() ) / 2;

            if ( align & AlignBottom )
                yo = height() - icon->height();
            else if ( align & AlignTop )
                yo = 0;

// respect horizontal alignment when there is no text for an item.
            if ( t.isEmpty() ) {
                if ( align & AlignRight )
                    xo = width - 2 * marg - iconWidth;
                else if ( align & AlignHCenter )
                    xo = ( width - iconWidth ) / 2;
            }
            p->drawPixmap( xo, yo, *icon );
        }


        if ( !t.isEmpty() ) {
            if ( !(align & AlignTop || align & AlignBottom) )
                align |= AlignVCenter;

            r += iconWidth;
    
            TextPaintItem::Chain::iterator it = m_items[column].items().begin();
            while(it != m_items[column].items().end()) 
            {
                int styleNum = (*it).style;
                TextPaintStyleStore::Item& style = m_styles.getStyle( styleNum );
                
                painter->setFont(style.font);
                p->drawText( r, 0, width-marg-r, height(), align, (*it).text );
                r += textWidth( style.font, (*it).text );
                ++it;
            }
        }
    }
    
    painter->restore();
}


int FancyListViewItem::textWidth(const QFont& font, const QString& text) 
{
    QFontMetrics fm( font );
    if ( multiLinesEnabled() )
        return fm.size( AlignVCenter, text ).width();
    else
        return fm.width( text );
    
}


int FancyListViewItem::width(const QFontMetrics &fm, const QListView *lv, int column)
{
    int width = 0;
    if (column >= 0 && column < (int)m_items.size() && !multiLinesEnabled()) {
        TextPaintItem::Chain::iterator it = m_items[column].items().begin();
        while(it != m_items[column].items().end()) {
            int styleNum = (*it).style;
            TextPaintStyleStore::Item& style = m_styles.getStyle( styleNum );
            
            width += textWidth( style.font, (*it).text);
            ++it;
        }
        width += lv->itemMargin() * 2;// - lv->d->minLeftBearing - lv->d->minRightBearing;
        
        const QPixmap * pm = pixmap( column );
        if ( pm )
            width += pm->width() + lv->itemMargin();
        
        width =  QMAX( width, QApplication::globalStrut().width() );
    }
    else
        width = QListViewItem::width(fm, lv, column);
    return width;
}



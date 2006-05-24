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

#ifndef FANCYLISTVIEWITEM
#define FANCYLISTVIEWITEM

/**
    When this header is included, a list-view-item based on KListViewItem is
    with the name FancyKListViewItem is available.
    If it is necessary to have the same based on QListViewItem,
    the header fancyqlistviewitem.h can be used.
*/

#include <qvaluevector.h>
#include <qpainter.h>
#include <qfont.h>
#include <qlistview.h>
#include <klistview.h>

namespace Widgets{
};

class TextPaintStyleStore {
    public:
        class Item {
            public:
            QFont font;
            QColor color;
            QColor background;
            
            Item(const QFont& f = QFont(), const QColor& c = QColor(), const QColor b = QColor() ) : font(f), color(c), background(b) {
            }
            
            bool bgValid() {
                return background.isValid();
            }
            
            bool colValid() {
                return color.isValid();
            }
        };
        
        typedef QMap<int, Item> Store ;
    
        TextPaintStyleStore( QFont defaultFont=QFont() ) {
            m_styles.insert( 0, Item( defaultFont ) );
        }
        
        Item& getStyle( int num ) {
            Store::Iterator it = m_styles.find( num );
            if( it != m_styles.end() ) return *it;
            return m_styles[0];
        }
        
        void addStyle( int num, Item& style ) {
            m_styles[num] = style;
        }
        
        void addStyle( int num, const QFont& font ) {
            m_styles[num] = Item( font );
        }
        
        bool hasStyle( int num ) {
            Store::Iterator it = m_styles.find( num );
            return ( it != m_styles.end() );
        }
        
    private:
        Store m_styles;
};



class TextPaintItem {
    public:
        struct Item {
            QString text;
            int style;
            
            Item( const QString& t = "", int st = 0 ) : text(t), style(st) {
            }
            
        };
        typedef QValueList<Item> Chain;
        
        Chain& items() {
            return m_chain;
        }
        
        TextPaintItem(const QString& text="") {
            addItem( text );
        }
        
        Item& addItem(const QString& item, int style = 0) {
            m_chain.append( Item(item, style) );
            return m_chain.back();
        }
        
        void clear() {
            m_chain.clear();
        }
        
        operator QString () const {
            QString ret;
            Chain::const_iterator it = m_chain.begin();
            while(it != m_chain.end()) {
                ret += (*it).text;
                ++it;
            }
            return ret;
        }
        
    private:
        Chain m_chain;
};

///does not support multiple column, a "column" represents a part of the real first column
///KListViewItem is only needed for the background-color

///all this is ugly, but templates don't do, and this is still better than code-duplication :)
class FancyListViewItem : public KListViewItem
{
    public:
        FancyListViewItem(TextPaintStyleStore& styles, QListView *parent, const QString &label1, const QString &label2="") : KListViewItem(parent, label1, label2), m_styles(styles) {
            init(label1, label2);
        }
        
        FancyListViewItem(TextPaintStyleStore& styles, QListViewItem *parent, const QString &label1, const QString &label2="") : KListViewItem(parent, label1, label2), m_styles(styles) {
            init(label1, label2);
        }
        
        virtual void paintCell(QPainter *painter, const QColorGroup &cg, int column, int width, int align);
        virtual int width(const QFontMetrics &fm, const QListView *lv, int column);
        virtual void setText ( int column, const QString & text );
        virtual QString text(int column) const;
        
        inline void clear() {
            m_items.clear();
        }
        
        inline TextPaintItem& item(int column = 0) {
            if(m_items.isEmpty()) {
                m_items.append( TextPaintItem() );
            }
            
            return m_items[column];
        }
        
        void setItem(int column, TextPaintItem item);
    private:
        virtual QColor backgroundColor(int col);
        void init(const QString& label1, const QString& label2);
        int textWidth(const QFont& font, const QString& text);
        QValueVector<TextPaintItem> m_items;
    protected:
        TextPaintStyleStore& m_styles;
};

/*}*/


#endif

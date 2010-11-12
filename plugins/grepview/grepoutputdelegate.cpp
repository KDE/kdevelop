/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright (C) 2007 Andreas Pakulat <apaku@gmx.de>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "grepoutputdelegate.h"
#include "grepoutputmodel.h"

#include <QtGui/QPainter>
#include <QtCore/QModelIndex>
#include <QtGui/QTextDocument>
#include <QtGui/QTextCursor>
#include <QtGui/QAbstractTextDocumentLayout>
#include <QtGui/QTextCharFormat>

#include <kdebug.h>

GrepOutputDelegate* GrepOutputDelegate::m_self = 0;

GrepOutputDelegate* GrepOutputDelegate::self()
{
    Q_ASSERT(m_self);
    return m_self;
}

GrepOutputDelegate::GrepOutputDelegate( QObject* parent )
    : QStyledItemDelegate(parent), textBrush( KColorScheme::View, KColorScheme::LinkText ),
      fileBrush( KColorScheme::View, KColorScheme::InactiveText )
{
    Q_ASSERT(!m_self);
    m_self = this;
}

GrepOutputDelegate::~GrepOutputDelegate()
{
    m_self = 0;
}

void GrepOutputDelegate::paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    QStyleOptionViewItem opt = option;
    QVariant status = index.data(Qt::UserRole+1);
    if( status.isValid() && status.toInt() == GrepOutputItem::Text )
    {
        opt.palette.setBrush( QPalette::Text, textBrush.brush( option.palette ) );
    }
    else if( status.isValid() && status.toInt() == GrepOutputItem::FileCollapsed )
    {
        opt.palette.setBrush( QPalette::Text, fileBrush.brush( option.palette ) );
    }
    else if( status.isValid() && status.toInt() == GrepOutputItem::FileExpanded )
    {
        opt.palette.setBrush( QPalette::Text, fileBrush.brush( option.palette ) );
    }
    
    // rich text component
    const GrepOutputModel *model = dynamic_cast<const GrepOutputModel *>(index.model());
    const GrepOutputItem  *item  = dynamic_cast<const GrepOutputItem *>(model->itemFromIndex(index));
    if(item && item->isMatch())
    {
        QStyleOptionViewItemV4 options = option;
        initStyleOption(&options, index);

        // building item representation
        const KDevelop::SimpleRange rng = item->change()->m_range;
        QTextDocument doc;
        QTextCursor cur(&doc);
        QTextCharFormat normal = cur.charFormat(), bold = cur.charFormat();
        bold.setFontWeight(QFont::Bold);
        cur.insertText(QString("%1: ").arg(item->lineNumber()));
        cur.insertText(item->text().left(rng.start.column));
        cur.insertText(item->text().mid(rng.start.column, rng.end.column - rng.start.column), bold);
        cur.insertText(item->text().right(item->text().length() - rng.end.column), normal);
        
        painter->save();
        options.text = "";  // text will be drawn separately
        options.widget->style()->drawControl(QStyle::CE_ItemViewItem, &options, painter);

        // set correct draw area
        QRect clip = options.widget->style()->subElementRect(QStyle::SE_ItemViewItemText, &options);
        painter->translate(clip.topLeft());
        clip.setTopLeft(QPoint(0,0));
        
        painter->setClipRect(clip);
        QAbstractTextDocumentLayout::PaintContext ctx;
        ctx.clip = clip;
        doc.documentLayout()->draw(painter, ctx);

        painter->restore();
    }
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }
}


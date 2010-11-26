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
#include <QtCore/QRegExp>

GrepOutputDelegate* GrepOutputDelegate::m_self = 0;

GrepOutputDelegate* GrepOutputDelegate::self()
{
    Q_ASSERT(m_self);
    return m_self;
}

GrepOutputDelegate::GrepOutputDelegate( QObject* parent )
    : QStyledItemDelegate(parent)
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
    // there is no function in QString to left-trim. A call to remove this this regexp does the job
    static const QRegExp leftspaces("^\\s*", Qt::CaseSensitive, QRegExp::RegExp);
    
    // rich text component
    const GrepOutputModel *model = dynamic_cast<const GrepOutputModel *>(index.model());
    const GrepOutputItem  *item  = dynamic_cast<const GrepOutputItem *>(model->itemFromIndex(index));
    if(item && item->isText())
    {
        QStyleOptionViewItemV4 options = option;
        initStyleOption(&options, index);

        // building item representation
        const KDevelop::SimpleRange rng = item->change()->m_range;
        QTextDocument doc;
        QTextCursor cur(&doc);
        
        QPalette::ColorGroup cg = options.state & QStyle::State_Enabled
                                  ? QPalette::Normal : QPalette::Disabled;
        QPalette::ColorRole cr  = options.state & QStyle::State_Selected
                                  ? QPalette::HighlightedText : QPalette::Text;
        QTextCharFormat fmt = cur.charFormat();
        fmt.setFont(options.font);
        
        // the line number appears grayed
        fmt.setForeground(options.palette.brush(QPalette::Disabled, cr));
        cur.insertText(QString("%1: ").arg(item->lineNumber()), fmt);
        
        // switch to normal color
        fmt.setForeground(options.palette.brush(cg, cr));
        cur.insertText(item->text().left(rng.start.column).remove(leftspaces), fmt);
        
        fmt.setFontWeight(QFont::Bold);
        cur.insertText(item->text().mid(rng.start.column, rng.end.column - rng.start.column), fmt);
        
        fmt.setFontWeight(QFont::Normal);
        cur.insertText(item->text().right(item->text().length() - rng.end.column), fmt);
        
        painter->save();
        options.text = QString();  // text will be drawn separately
        options.widget->style()->drawControl(QStyle::CE_ItemViewItem, &options, painter, options.widget);

        // set correct draw area
        QRect clip = options.widget->style()->subElementRect(QStyle::SE_ItemViewItemText, &options);
        QFontMetrics metrics(options.font);
        painter->translate(clip.topLeft() - QPoint(0, metrics.descent()));
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


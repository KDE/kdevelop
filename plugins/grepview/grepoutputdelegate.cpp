/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright (C) 2007 Andreas Pakulat <apaku@gmx.de>                     *
 *   Copyright 2010 Julien Desgats <julien.desgats@gmail.com>              *
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

#include <QPainter>
#include <QtCore/QModelIndex>
#include <QTextDocument>
#include <QTextCursor>
#include <QAbstractTextDocumentLayout>
#include <QTextCharFormat>
#include <QtCore/QRegExp>
#include <KLocalizedString>
#include <cmath>
#include <algorithm>

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

QColor GrepOutputDelegate::blendColor(QColor color1, QColor color2, double blend) const
{
    return QColor(color1.red() * blend + color2.red() * (1-blend),
                  color1.green() * blend + color2.green() * (1-blend),
                  color1.blue() * blend + color2.blue() * (1-blend));
}

void GrepOutputDelegate::paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{ 
    // there is no function in QString to left-trim. A call to remove this this regexp does the job
    static const QRegExp leftspaces("^\\s*", Qt::CaseSensitive, QRegExp::RegExp);
    
    // rich text component
    const GrepOutputModel *model = dynamic_cast<const GrepOutputModel *>(index.model());
    const GrepOutputItem  *item  = dynamic_cast<const GrepOutputItem *>(model->itemFromIndex(index));

    QStyleOptionViewItemV4 options = option;
    initStyleOption(&options, index);

    // building item representation
    QTextDocument doc;
    QTextCursor cur(&doc);
    
    QPalette::ColorGroup cg = options.state & QStyle::State_Enabled
                                ? QPalette::Normal : QPalette::Disabled;
    QPalette::ColorRole cr  = options.state & QStyle::State_Selected
                                ? QPalette::HighlightedText : QPalette::Text;
    QTextCharFormat fmt = cur.charFormat();
    fmt.setFont(options.font);

    if(item && item->isText())
    {
        // Use custom manual highlighting

        const KDevelop::SimpleRange rng = item->change()->m_range;

        // the line number appears grayed
        fmt.setForeground(options.palette.brush(QPalette::Disabled, cr));
        cur.insertText(i18n("Line %1: ",item->lineNumber()), fmt);
        
        // switch to normal color
        fmt.setForeground(options.palette.brush(cg, cr));
        cur.insertText(item->text().left(rng.start.column).remove(leftspaces), fmt);
        
        fmt.setFontWeight(QFont::Bold);
        // Blend the highlighted background color
        // For some reason, it is extremely slow to use alpha-blending directly here
        QColor bgHighlight = blendColor(option.palette.brush(QPalette::Highlight).color(), option.palette.brush(QPalette::Base).color(), 0.3);
        fmt.setBackground(bgHighlight);
        cur.insertText(item->text().mid(rng.start.column, rng.end.column - rng.start.column), fmt);
        fmt.clearBackground();
        
        fmt.setFontWeight(QFont::Normal);
        cur.insertText(item->text().right(item->text().length() - rng.end.column), fmt);
    }else{
        QString text;
        if(item)
            text = item->text();
        else
            text = index.data().toString();
        // Simply insert the text as html. We use this for the titles.
        doc.setHtml(text);
    }
    
    painter->save();
    options.text = QString();  // text will be drawn separately
    options.widget->style()->drawControl(QStyle::CE_ItemViewItem, &options, painter, options.widget);

    // set correct draw area
    QRect clip = options.widget->style()->subElementRect(QStyle::SE_ItemViewItemText, &options);
    QFontMetrics metrics(options.font);
    painter->translate(clip.topLeft() - QPoint(0, metrics.descent()));

    // We disable the clipping for now, as it leads to strange clipping errors
//     clip.setTopLeft(QPoint(0,0));
    
//     painter->setClipRect(clip);
    QAbstractTextDocumentLayout::PaintContext ctx;
//     ctx.clip = clip;
    painter->setBackground(Qt::transparent);
    doc.documentLayout()->draw(painter, ctx);

    painter->restore();
}

QSize GrepOutputDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const GrepOutputModel *model = dynamic_cast<const GrepOutputModel *>(index.model());
    const GrepOutputItem  *item  = dynamic_cast<const GrepOutputItem *>(model->itemFromIndex(index));

    QSize ret = QStyledItemDelegate::sizeHint(option, index);

    //take account of additional width required for highlighting (bold text)
    //and line numbers. These are not included in the default Qt size calculation.
    if(item && item->isText())
    {
        QFont font = option.font;
        font.setBold(true);
        QFontMetrics bMetrics(font);

        //TODO: calculate width with more accuracy: here the whole text is considerated as bold
        int width =  bMetrics.width(item->text()) +
                     option.fontMetrics.width(i18n("Line %1: ",item->lineNumber())) +
                     std::max(option.decorationSize.width(), 0);
        ret.setWidth(width);
    }else{
        // This is only used for titles, so not very performance critical
        QString text;
        if(item)
            text = item->text();
        else
            text = index.data().toString();
        
        QTextDocument doc;
        doc.setDocumentMargin(0);
        doc.setHtml(text);
        QSize newSize = doc.size().toSize();
        if(newSize.height() > ret.height())
            ret.setHeight(newSize.height());
    }

    ret.setHeight(ret.height()+2); // We slightly increase the vertical size, else the view looks too crowded
    return ret;
}

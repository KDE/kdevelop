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
#include <QModelIndex>
#include <QTextDocument>
#include <QTextCursor>
#include <QAbstractTextDocumentLayout>
#include <QTextCharFormat>
#include <QRegExp>
#include <KLocalizedString>
#include <cmath>
#include <algorithm>

GrepOutputDelegate* GrepOutputDelegate::m_self = nullptr;

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
    m_self = nullptr;
}

void GrepOutputDelegate::paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{ 
    // there is no function in QString to left-trim. A call to remove this regexp does the job
    static const QRegExp leftspaces(QStringLiteral("^\\s*"), Qt::CaseSensitive, QRegExp::RegExp);
    
    // rich text component
    const auto* model = qobject_cast<const GrepOutputModel*>(index.model());
    const auto  *item  = dynamic_cast<const GrepOutputItem *>(model->itemFromIndex(index));

    QStyleOptionViewItem options = option;
    initStyleOption(&options, index);

    // building item representation
    QTextDocument doc;
    QTextCursor cur(&doc);
    
    QPalette::ColorGroup cg = (options.state & QStyle::State_Enabled)
                                ? QPalette::Normal : QPalette::Disabled;
    QPalette::ColorRole cr  = (options.state & QStyle::State_Selected)
                                ? QPalette::HighlightedText : QPalette::Text;
    QTextCharFormat fmt = cur.charFormat();
    fmt.setFont(options.font);

    if(item && item->isText())
    {
        if (item->hasChildren()) {
             // the line number appears grayed
            fmt.setForeground(options.palette.brush(QPalette::Disabled, cr));
            cur.insertText(i18n("Line %1: ",item->lineNumber()), fmt);
            fmt.setForeground(options.palette.brush(cg, cr));

            int firstStart = static_cast<GrepOutputItem*>(item->child(0))->change()->m_range.start().column();
            cur.insertText(item->text().left(firstStart).remove(leftspaces), fmt);
            KTextEditor::Range previousRange(item->lineNumber(), 0, item->lineNumber(), firstStart);
            for (int i = 0; i < item->rowCount(); ++i) {
                const KTextEditor::Range range = static_cast<GrepOutputItem*>(item->child(i))->change()->m_range;
                fmt.setForeground(options.palette.brush(cg, cr));
                cur.insertText(item->text().mid(previousRange.end().column(), range.start().column() - previousRange.end().column()), fmt);
                fmt.setFontWeight(QFont::Bold);
                if ( !(options.state & QStyle::State_Selected) ) {
                    QColor bgHighlight = option.palette.color(QPalette::AlternateBase);
                    fmt.setBackground(bgHighlight);
                }
                cur.insertText(item->text().mid(range.start().column(), range.columnWidth()), fmt);
                fmt.clearBackground();
                previousRange = range;
            }
            fmt.setFontWeight(QFont::Normal);
            cur.insertText(item->text().mid(previousRange.end().column()), fmt);
        } else {
            // Use custom manual highlighting

            const KTextEditor::Range rng = item->change()->m_range;

            // switch to normal color
            fmt.setForeground(options.palette.brush(cg, cr));
            cur.insertText(item->text().left(rng.start().column()).remove(leftspaces), fmt);

            fmt.setFontWeight(QFont::Bold);
            if ( !(options.state & QStyle::State_Selected) ) {
                QColor bgHighlight = option.palette.color(QPalette::AlternateBase);
                fmt.setBackground(bgHighlight);
            }
            cur.insertText(item->text().mid(rng.start().column(), rng.end().column() - rng.start().column()), fmt);
            fmt.clearBackground();

            fmt.setFontWeight(QFont::Normal);
            cur.insertText(item->text().mid(rng.end().column()), fmt);
        }
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
    const auto* model = qobject_cast<const GrepOutputModel*>(index.model());
    const GrepOutputItem  *item  = model ? dynamic_cast<const GrepOutputItem *>(model->itemFromIndex(index)) : nullptr;

    QSize ret = QStyledItemDelegate::sizeHint(option, index);

    //take account of additional width required for highlighting (bold text)
    //and line numbers. These are not included in the default Qt size calculation.
    if(item && item->isText())
    {
        QFont font = option.font;
        QFontMetrics metrics(font);
        font.setBold(true);
        QFontMetrics bMetrics(font);
        if (item->hasChildren()) {
            int bWidth = 0;
            for (int i = 0; i < item->rowCount(); ++i) {
                bWidth += static_cast<GrepOutputItem*>(item->child(i))->change()->m_range.columnWidth();
            }
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
            int width = option.fontMetrics.horizontalAdvance(i18n("Line %1: ",item->lineNumber())) +
                        metrics.horizontalAdvance(item->text().length() - bWidth) + bMetrics.horizontalAdvance(bWidth) +
                        std::max(option.decorationSize.width(), 0);
#else
            int width = option.fontMetrics.width(i18n("Line %1: ",item->lineNumber())) +
                        metrics.width(item->text().length() - bWidth) + bMetrics.width(bWidth) +
                        std::max(option.decorationSize.width(), 0);
#endif
            ret.setWidth(width);
        } else {
            const KTextEditor::Range rng = item->change()->m_range;

#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
            int width = metrics.horizontalAdvance(item->text().left(rng.start().column())) +
                        metrics.horizontalAdvance(item->text().mid(rng.end().column())) +
                        bMetrics.horizontalAdvance(item->text().mid(rng.start().column(), rng.end().column() - rng.start().column())) +
                        std::max(option.decorationSize.width(), 0);
#else
            int width = metrics.width(item->text().left(rng.start().column())) +
                        metrics.width(item->text().mid(rng.end().column())) +
                        bMetrics.width(item->text().mid(rng.start().column(), rng.end().column() - rng.start().column())) +
                        std::max(option.decorationSize.width(), 0);
#endif
            ret.setWidth(width);
        }
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
    return ret;
}

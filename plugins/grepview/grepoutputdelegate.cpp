/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2010 Julien Desgats <julien.desgats@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "grepoutputdelegate.h"
#include "grepoutputmodel.h"

#include <KLocalizedString>

#include <QAbstractTextDocumentLayout>
#include <QModelIndex>
#include <QPainter>
#include <QRegularExpression>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>

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
    static const QRegularExpression leftspaces(QStringLiteral("^\\s*"));

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
        // Use custom manual highlighting

        const KTextEditor::Range rng = item->change()->m_range;

        // the line number appears grayed
        fmt.setForeground(options.palette.brush(QPalette::Disabled, cr));
        cur.insertText(i18n("Line %1: ",item->lineNumber()), fmt);
        
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
        const KTextEditor::Range rng = item->change()->m_range;
        int width = metrics.horizontalAdvance(item->text().left(rng.start().column()))
            + metrics.horizontalAdvance(item->text().mid(rng.end().column()))
            + bMetrics.horizontalAdvance(
                item->text().mid(rng.start().column(), rng.end().column() - rng.start().column()))
            + option.fontMetrics.horizontalAdvance(i18n("Line %1: ", item->lineNumber()))
            + std::max(option.decorationSize.width(), 0);
        ret.setWidth(width);
    }
    return ret;
}

#include "moc_grepoutputdelegate.cpp"

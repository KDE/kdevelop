/***************************************************************************
 *   Copyright (C) 2005 by Robert Gruber                                   *
 *   rgruber@users.sourceforge.net                                         *
 *                                                                         *
 *   This file has been taken from cervisia an adapted to fit my needs:    *
 *   Copyright (C) 1999-2002 Bernd Gehrmann <bernd@mail.berlios.de>        *
 *   Copyright (c) 2003-2005 André Wöbbeking <Woebbeking@web.de>           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "annotateview.h"

#include <qheader.h>
#include <qdatetime.h>
#include <qpainter.h>
#include <kglobalsettings.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include "annotatepage.h"

class AnnotateViewItem : public QListViewItem
{
    friend class AnnotateView;

public:
    enum { LineNumberColumn, AuthorColumn, DateColumn,ContentColumn };

    AnnotateViewItem(AnnotateView *parent, QString rev, QString author, 
            QDateTime date, QString content, QString comment, 
            bool odd, int linenumber);

    virtual int compare(QListViewItem *item, int col, bool ascending) const;
    virtual int width(const QFontMetrics &, const QListView *, int col) const;
    virtual QString text(int col) const;
    virtual void paintCell(QPainter *, const QColorGroup &, int, int, int);

private:
    QString m_revision;
    QString m_author;
    QString m_content;
    QString m_comment;
    QDateTime m_logDate;
    bool m_odd;
    int m_lineNumber;

    static const int BORDER;
};


const int AnnotateViewItem::BORDER = 4;


AnnotateViewItem::AnnotateViewItem(AnnotateView *parent, QString rev, 
    QString author, QDateTime date, QString content, QString comment, 
    bool odd, int linenumber)
    : QListViewItem(parent)
    , m_revision(rev)
    , m_author(author)
    , m_content(content)
    , m_comment(comment)
    , m_logDate(date)
    , m_odd(odd)
    , m_lineNumber(linenumber)
{}


int AnnotateViewItem::compare(QListViewItem *item, int, bool) const
{
    int linenum1 = m_lineNumber;
    int linenum2 = static_cast<AnnotateViewItem*>(item)->m_lineNumber;

    return (linenum2 > linenum1)? -1 : (linenum2 < linenum1)? 1 : 0;
}


QString AnnotateViewItem::text(int col) const
{
    switch (col)
    {
    case LineNumberColumn:
        return QString::number(m_lineNumber);
    case AuthorColumn:
        return (m_revision + QChar(' ') + m_author);
    case DateColumn:
        return KGlobal::locale()->formatDate(m_logDate.date(), true);
    case ContentColumn:
        return m_content;
    default:
        ;
    };

    return QString::null;
}


void AnnotateViewItem::paintCell(QPainter *p, const QColorGroup &, int col, int width, int align)
{
    QColor backgroundColor;

    switch (col)
    {
    case LineNumberColumn:
        backgroundColor = KGlobalSettings::highlightColor();
        p->setPen(KGlobalSettings::highlightedTextColor());
        break;
    default:
        backgroundColor = m_odd ? KGlobalSettings::baseColor()
                                : KGlobalSettings::alternateBackgroundColor();
        p->setPen(KGlobalSettings::textColor());
        break;
    };

    p->fillRect(0, 0, width, height(), backgroundColor);

    QString str = text(col);
    if (str.isEmpty())
        return;

    if (align & (AlignTop || AlignBottom) == 0)
            align |= AlignVCenter;

    p->drawText(BORDER, 0, width - 2*BORDER, height(), align, str);
}


int AnnotateViewItem::width(const QFontMetrics &fm, const QListView *, int col) const
{
    return fm.width(text(col)) + 2*BORDER;
}


/******************************************************************************/
/*****************Definition of class AnnotateView ****************************/
/******************************************************************************/

AnnotateView::AnnotateView(AnnotatePage *parent, const char *name)
    : KListView(parent, name), QToolTip( viewport() ), 
    m_page(parent)
{
    setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
    setAllColumnsShowFocus(true);
    setShowToolTips(false);
    header()->hide();

    addColumn(QString::null);
    addColumn(QString::null);
    addColumn(QString::null);
    addColumn(QString::null);

    setSorting(AnnotateViewItem::LineNumberColumn);
    setColumnAlignment(AnnotateViewItem::LineNumberColumn, Qt::AlignRight);

    connect( this, SIGNAL(executed(QListViewItem*)),
             this, SLOT(itemClicked(QListViewItem*)) );
}


void AnnotateView::addLine(QString rev, QString author, QDateTime date, 
        QString content, QString comment, bool odd)
{
    new AnnotateViewItem(this, rev, author, date, content, comment, 
            odd, childCount()+1);
}


QSize AnnotateView::sizeHint() const
{
    QFontMetrics fm(fontMetrics());
    return QSize(100 * fm.width("0"), 20 * fm.lineSpacing());
}


void AnnotateView::maybeTip( const QPoint & p )
{
    AnnotateViewItem * item = dynamic_cast<AnnotateViewItem*>( itemAt( p ) );
    if (!item)
        return;

    const int column(header()->sectionAt(p.x()));
    if (column != AnnotateViewItem::AuthorColumn &&
        column != AnnotateViewItem::DateColumn) {
        return;
    }

    QRect r = itemRect( item );
    //get the dimension of the author + the date column
    QRect headerRect = header()->sectionRect(AnnotateViewItem::AuthorColumn);
    headerRect = headerRect.unite(header()->sectionRect(AnnotateViewItem::DateColumn));

    r.setLeft(headerRect.left());
    r.setWidth(headerRect.width());

    if (r.isValid())
    {
        tip( r, "<nobr><b>"+item->text(AnnotateViewItem::AuthorColumn)+"</b></nobr><br>"
                "<nobr>"+item->text(AnnotateViewItem::DateColumn)+"</nobr>"
                "<pre>"+item->m_comment+"</pre>");
    }
}

void AnnotateView::itemClicked(QListViewItem *item)
{
    kdDebug(9006) << "itemClicked()" << endl;

    AnnotateViewItem * line = dynamic_cast<AnnotateViewItem*>(item);
    if (line) {
        kdDebug(9006) << "requesting annotate for revision " << line->m_revision << endl;
        emit m_page->requestAnnotate(line->m_revision);
    } else {
        kdDebug(9006) << "This is not an AnnotateViewItem" << endl;
    }
}

#include "annotateview.moc"

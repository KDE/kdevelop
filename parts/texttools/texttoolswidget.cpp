/***************************************************************************
 *   Copyright (C) 2002 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "texttoolswidget.h"

#include <q3header.h>
#include <qregexp.h>
#include <qtimer.h>
#include <kdebug.h>
#include <klocale.h>
#include <kparts/part.h>
#include <kpopupmenu.h>
#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/selectioninterface.h>
#include <ktexteditor/editinterface.h>

#include "kdevmainwindow.h"
#include "kdevpartcontroller.h"
#include "texttoolspart.h"


class TextStructItem : public Q3ListViewItem
{
public:
    TextStructItem(Q3ListView *parent)
        : Q3ListViewItem(parent)
    {}
    TextStructItem(Q3ListViewItem *parent)
        : Q3ListViewItem(parent)
    {
        Q3ListViewItem *item = this;
        while (item->nextSibling())
            item = item->nextSibling();
        if (item != this)
            moveItem(item);
    }
    
    QString text(int) const
    {
        return extra.isNull()? tag : QString("%1: %2").arg(tag).arg(extra);
    }
    TextStructItem *parentStructItem()
    { return static_cast<TextStructItem*>(parent()); }
    
    QString tag;
    QString extra;
    int pos;
    int endpos;
};


TextToolsWidget::TextToolsWidget(TextToolsPart *part, QWidget *parent, const char *name)
    : KListView(parent, name)
{
    setResizeMode(Q3ListView::LastColumn);
    setSorting(-1);
    header()->hide();
    addColumn(QString::null);

    m_part = part;

    m_timer = new QTimer(this);
    connect( this, SIGNAL(mouseButtonPressed(int, Q3ListViewItem*, const QPoint&, int)),
             this, SLOT(slotItemPressed(int,Q3ListViewItem*)) );
    //    connect( this, SIGNAL(doubleClicked(QListViewItem*)),
    //             this, SLOT(slotItemPressed(int,QListViewItem*)) );
    connect( this, SIGNAL(returnPressed(Q3ListViewItem*)),
             this, SLOT(slotReturnPressed(Q3ListViewItem*)) );
    connect( this, SIGNAL(contextMenu(KListView*, Q3ListViewItem*, const QPoint&)),
             this, SLOT(slotContextMenu(KListView*, Q3ListViewItem*, const QPoint&)) );
}


TextToolsWidget::~TextToolsWidget()
{}


void TextToolsWidget::slotItemPressed(int button, Q3ListViewItem *item)
{
    if (!item)
        return;

    TextStructItem *tsitem = static_cast<TextStructItem*>(item);
    int searchedPos = tsitem->pos;
    int searchedEndpos = tsitem->endpos;
    kdDebug(9030) << "Searched pos " << searchedPos << ", " << searchedEndpos << endl;
    
    int endline = 0;
    int endcol = 0;
    int line = 0;
    int col = 0;

    int len = m_cachedText.length();
    int pos = 0;
    while (pos < len) {
        if (pos == searchedPos) {
            line = endline;
            col = endcol;
        }
        if (pos == searchedEndpos)
            break;
        QChar ch = m_cachedText[pos];
        if (ch == '\n') {
            ++endline;
            endcol = 0;
        } else {
            ++endcol;
        }
        ++pos;
    }

    KParts::Part *rwpart
        = dynamic_cast<KParts::Part*>(m_part->partController()->activePart());
    QWidget *view = m_part->partController()->activeWidget();

    KTextEditor::ViewCursorInterface *cursorIface
        = dynamic_cast<KTextEditor::ViewCursorInterface*>(view);
    if (cursorIface) {
        kdDebug(9030) << "set cursor " << line << ", " << col << endl;
        cursorIface->setCursorPosition(line, col);
    }
    
    if (button == MidButton) {
        KTextEditor::SelectionInterface *selectionIface
            = dynamic_cast<KTextEditor::SelectionInterface*>(rwpart);
        if (selectionIface) {
            kdDebug(9030) << "set selection " << line << ", " << col
                      << ", " << endline << ", " << endcol << endl;
            selectionIface->setSelection((int)line, (int)col, (int)endline, (int)endcol+1);
        }
    }

    m_part->mainWindow()->lowerView(this);
}


void TextToolsWidget::slotReturnPressed(Q3ListViewItem *item)
{
    slotItemPressed(LeftButton, item);
}


void TextToolsWidget::slotContextMenu(KListView *, Q3ListViewItem *item, const QPoint &)
{
    if (!item)
        return;

#if 0
    KPopupMenu popup(i18n("Text Structure"), this);
    popup.exec(p);
#endif
}


void TextToolsWidget::stop()
{
    disconnect( m_timer );
    m_relevantTags.clear();
    m_emptyTags.clear();
    m_cachedText = QString::null;
}


void TextToolsWidget::setMode(Mode mode, KParts::Part *part)
{
    connect( part, SIGNAL(textChanged()),
             this, SLOT(startTimer()) );
    m_editIface = dynamic_cast<KTextEditor::EditInterface*>(part);
    
    switch (mode) {
    case HTML:
        m_relevantTags << "h1" << "h2" << "h3" << "h4"
                       << "table" << "tr";
        m_emptyTags << "br" << "hr" << "img" << "input" << "p" << "meta";
        
        connect( m_timer, SIGNAL(timeout()),
                 this, SLOT(parseXML()) );
        break;
    case Docbook:
        m_relevantTags << "chapter" << "sect1" << "sect2"
                       << "para" << "formalpara";
        connect( m_timer, SIGNAL(timeout()),
                 this, SLOT(parseXML()) );
        break;
    case LaTeX:
        connect( m_timer, SIGNAL(timeout()),
                 this, SLOT(parseLaTeX()) );
        break;
    default: ;
    }

    m_timer->start(0, true);
}


void TextToolsWidget::startTimer()
{
    kdDebug(9030) << "Starting parse timer" << endl;
    m_timer->start(1000, true);
}


void TextToolsWidget::parseXML()
{
    kdDebug(9030) << "Starting to parse XML" << endl;
    clear();
    QString text = m_editIface->text();
    m_cachedText = text;

    TextStructItem *currentItem = new TextStructItem(this);
    currentItem->tag = "Root";
    currentItem->pos = -1;
    currentItem->endpos = -1;
    
    int len = text.length();
    for (int pos=0; pos+1 < len; ++pos) {
        QChar ch1 = text[pos];
        QChar ch2 = text[pos+1];

        if (ch1 == '<' && ch2 == '?') {
            
            // PHP and other similar stuff
            QString tag;
            int endpos = pos+2;
            bool foundspace = false;
            while (endpos+1 < len) {
                QChar ch3 = text[endpos];
                QChar ch4 = text[endpos+1];
                if ((ch3 == ' ' || ch3 == '\t' || ch3 == '\n') && !foundspace) {
                    tag = text.mid(pos+2, endpos-pos-2).lower();
                    foundspace = true;
                } else if (ch3 == '?' && ch4 == '>') {
                    if (!foundspace)
                        tag = text.mid(pos+2, endpos-pos-2).lower();
                    break;
                }
                ++endpos;
            }

            TextStructItem *item = new TextStructItem(currentItem);
            item->tag = "<?" + tag + "?>";
            item->pos = pos;
            item->endpos = endpos+1;

            pos = endpos+1;
            
        } else  if (ch1 == '<' && ch2 == '!') {
            
            // Processing instructions like !DOCTYPE
            QString tag;
            int endpos = pos+2;
            bool foundspace = false;
            while (endpos+1 < len) {
                QChar ch3 = text[endpos];
                if ((ch3 == ' ' || ch3 == '\t' || ch3 == '\n') && !foundspace) {
                    tag = text.mid(pos+2, endpos-pos-2).lower();
                    foundspace = true;
                } else if (ch3 == '>') {
                    if (!foundspace)
                        tag = text.mid(pos+2, endpos-pos-2).lower();
                    break;
                }
                ++endpos;
            }

            TextStructItem *item = new TextStructItem(currentItem);
            item->tag = "<!" + tag + ">";
            item->pos = pos;
            item->endpos = endpos+1;

            pos = endpos+1;
            
        } else if (ch1 == '<' && ch2 == '/') {

            QString tag;
            int endpos = pos+2;
            while (endpos < len) {
                QChar ch3 = text[endpos];
                if (ch3 == '>') {
                    tag = text.mid(pos+2, endpos-pos-2).lower();
                    break;
                }
                ++endpos;
            }

            if (!m_relevantTags.contains(tag)) {
                pos = endpos;
                continue;
            }
            
            TextStructItem *closingItem = currentItem;
            while (closingItem->parent() && closingItem->tag != tag)
                closingItem = closingItem->parentStructItem();
            if (closingItem->parent()) {
                closingItem->endpos = endpos;
                currentItem = closingItem->parentStructItem();
            } else {
                kdDebug(9030) << "found no opening tag " << tag << "." << endl;
            }
            
            pos = endpos;
            
        } else if (ch1 == '<') {

            QString tag;
            int endpos = pos+1;
            bool foundspace = false;
            while (endpos < len) {
                QChar ch3 = text[endpos];
                if ((ch3 == ' ' || ch3 == '\t' || ch3 == '\n') && !foundspace) {
                    tag = text.mid(pos+1, endpos-pos-1).lower();
                    foundspace = true;
                } else if (ch3 == '>') {
                    if (!foundspace) {
                        tag = text.mid(pos+1, endpos-pos-1).lower();
                    }
                    break;
                }
                ++endpos;
            }

            if (!m_relevantTags.contains(tag)) {
                pos = endpos;
                continue;
            }

            TextStructItem *item = new TextStructItem(currentItem);
            item->tag = tag;
            item->pos = pos;
            item->endpos = -1;

            if (m_emptyTags.contains(tag))
                item->endpos = endpos;
            else
                currentItem = item;
            pos = endpos;

        }
    }

    //    firstChild()->setOpen(true);
    Q3ListViewItemIterator it(this);
    for (; it.current(); ++it)
        it.current()->setOpen(true);
}


void TextToolsWidget::parseLaTeX()
{
    kdDebug(9030) << "Starting to parse LaTeX" << endl;
    clear();
    QString text = m_editIface->text();
    m_cachedText = text;

    TextStructItem *currentItem = new TextStructItem(this);
    currentItem->tag = "Root";
    currentItem->pos = -1;
    currentItem->endpos = -1;

    QString hierarchyLevels = "Root,chapter,section,subsection,subsubsection";
    QRegExp re("\n[ \t]*s*\\\\(chapter|section|subsection|subsubsection)\\{([^}]*)\\}");
    
    int pos=0;
    for (;;) {
        pos = re.search(text, pos);
        if (pos == -1)
            break;
        QString tag = re.cap(1);
        QString title = re.cap(2);
        kdDebug(9030) << "Match with " << tag << " and title " << title << endl;
        int level = hierarchyLevels.find(tag);
        while (currentItem->parent() && level <= hierarchyLevels.find(currentItem->tag))
            currentItem = currentItem->parentStructItem();
        
        TextStructItem *item = new TextStructItem(currentItem);
        item->tag = tag;
        item->extra = title;
        item->pos = pos+1;
        item->endpos = pos+re.matchedLength()-1; // lie

        if (level > hierarchyLevels.find(currentItem->tag))
            currentItem = item;

        pos = pos+re.matchedLength();
    }

    Q3ListViewItemIterator it(this);
    for (; it.current(); ++it)
        it.current()->setOpen(true);
}

#include "texttoolswidget.moc"

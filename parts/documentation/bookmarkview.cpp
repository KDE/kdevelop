/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "bookmarkview.h"

#include <qlayout.h>
#include <qheader.h>
#include <qlineedit.h>
#include <qpoint.h>

#include <kstandarddirs.h>
#include <klocale.h>
#include <kdialog.h>
#include <kpushbutton.h>
#include <kurlrequester.h>
#include <kpopupmenu.h>
#include <kparts/part.h>
#include <khtml_part.h>
#include <dom/html_document.h>

#include <kdevpartcontroller.h>
#include <kdevdocumentationplugin.h>

#include "documentation_part.h"
#include "documentation_widget.h"
#include "editbookmarkdlg.h"
#include "docutils.h"

DocBookmarkManager::DocBookmarkManager(DocumentationPart */*part*/)
    :KBookmarkManager(locateLocal("data",
    "kdevdocumentation/bookmarks/bookmarks.xml"), false)
{
    setEditorOptions(i18n("Documentation"), false);
}

DocBookmarkOwner::DocBookmarkOwner(DocumentationPart *part)
    :KBookmarkOwner(), m_part(part)
{
}

void DocBookmarkOwner::openBookmarkURL(const QString &_url)
{
    m_part->partController()->showDocument(KURL(_url));
}

QString DocBookmarkOwner::currentURL() const
{
    KParts::ReadOnlyPart *activePart = dynamic_cast<KParts::ReadOnlyPart*>(m_part->partController()->activePart());
    if (activePart)
        return activePart->url().url();
    else
        return QString::null;
}

QString DocBookmarkOwner::currentTitle() const
{
    KParts::ReadOnlyPart *activePart = dynamic_cast<KParts::ReadOnlyPart*>(m_part->partController()->activePart());
    if (activePart)
    {
        KHTMLPart *htmlPart = dynamic_cast<KHTMLPart*>(activePart);
        if (htmlPart)
            return htmlPart->htmlDocument().title().string();
        return activePart->url().prettyURL();
    }
    else
        return QString::null;
}

class DocBookmarkItem: public DocumentationItem {
public:
    DocBookmarkItem(Type type, KListView *parent, const QString &name)
        :DocumentationItem(type, parent, name)
    {
    }
    DocBookmarkItem(Type type, KListView *parent, DocumentationItem *after, const QString &name)
        :DocumentationItem(type, parent, after, name)
    {
    }
    DocBookmarkItem(Type type, DocumentationItem *parent, const QString &name)
        :DocumentationItem(type, parent, name)
    {
    }
    void setBookmark(const KBookmark &bm) { m_bm = bm; }
    KBookmark bookmark() const { return m_bm; }
    
private:
    KBookmark m_bm;
};



//class BookmarkView

BookmarkView::BookmarkView(DocumentationWidget *parent, const char *name)
    :QWidget(parent, name), m_widget(parent)
{
    m_bmManager = new DocBookmarkManager(m_widget->part());
    m_bmOwner = new DocBookmarkOwner(m_widget->part());

    QVBoxLayout *l = new QVBoxLayout(this, 0, KDialog::spacingHint());    
    m_view = new KListView(this);
    m_view->addColumn(i18n("Title"));
    m_view->setSorting(-1);
    m_view->header()->hide();
    m_view->setResizeMode(QListView::AllColumns);
    l->addWidget(m_view);
    QHBoxLayout *l2 = new QHBoxLayout(l, KDialog::spacingHint());
    m_addButton = new KPushButton(i18n("Add"), this);
    m_editButton = new KPushButton(i18n("Edit"), this);
    m_removeButton = new KPushButton(i18n("Remove"), this);
    l2->addWidget(m_addButton);
    l2->addWidget(m_editButton);
    l2->addWidget(m_removeButton);
    l2->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));
    l->addSpacing(2);

    showBookmarks();

    connect(m_view, SIGNAL(executed(QListViewItem*, const QPoint&, int )),
        this, SLOT(itemExecuted(QListViewItem*, const QPoint&, int )));
    connect(m_addButton, SIGNAL(pressed()), this, SLOT(addBookmark()));
    connect(m_editButton, SIGNAL(clicked()), this, SLOT(editBookmark()));
    connect(m_removeButton, SIGNAL(clicked()), this, SLOT(removeBookmark()));
    
    connect(m_widget->part(), SIGNAL(bookmarkLocation(const QString&, const KURL& )),
        this, SLOT(addBookmark(const QString&, const KURL& )));
    connect(m_view, SIGNAL(mouseButtonPressed(int, QListViewItem*, const QPoint&, int )),
        this, SLOT(itemMouseButtonPressed(int, QListViewItem*, const QPoint&, int )));
}

BookmarkView::~BookmarkView()
{
    delete m_bmManager;
    delete m_bmOwner;
}

void BookmarkView::showBookmarks()
{
    const KBookmarkGroup &group = m_bmManager->root();
    DocBookmarkItem *item = 0;
    for (KBookmark bm = group.first(); !bm.isNull(); bm = group.next(bm))
    {
        if (item == 0)
            item = new DocBookmarkItem(DocumentationItem::Document, m_view, bm.fullText());
        else
            item = new DocBookmarkItem(DocumentationItem::Document, m_view, item, bm.fullText());
        item->setURL(bm.url());
        item->setBookmark(bm);
    }
}

void BookmarkView::itemExecuted(QListViewItem *item, const QPoint &p, int col)
{
    DocumentationItem *docItem = dynamic_cast<DocumentationItem*>(item);
    if (!docItem)
        return;
    m_widget->part()->partController()->showDocument(docItem->url());
}

void BookmarkView::removeBookmark()
{
    if (!m_view->currentItem())
        return;
    DocBookmarkItem *item = dynamic_cast<DocBookmarkItem*>(m_view->currentItem());
    m_bmManager->root().deleteBookmark(item->bookmark());
    m_bmManager->save();
    delete item;
}

void BookmarkView::editBookmark()
{
    if (!m_view->currentItem())
        return;
    DocBookmarkItem *item = dynamic_cast<DocBookmarkItem*>(m_view->currentItem());
    if (!item)
        return;
    
    EditBookmarkDlg dlg(this);
    dlg.setCaption(i18n("Edit Bookmark"));
    dlg.nameEdit->setText(item->bookmark().fullText());
    dlg.locationEdit->setURL(item->bookmark().url().url());
    dlg.nameEdit->setFocus();
    if (dlg.exec())
    {
        item->bookmark().internalElement().namedItem("title").firstChild().toText().setData(dlg.nameEdit->text());        
        item->bookmark().internalElement().setAttribute("href", KURL(dlg.locationEdit->url()).url());
        m_bmManager->save();
        
        item->setText(0, item->bookmark().fullText());
        item->setURL(item->bookmark().url());
    }
}

void BookmarkView::addBookmark()
{
    QString title = m_bmOwner->currentTitle();
    QString url = m_bmOwner->currentURL();
    
    KPopupMenu menu;
    bool useMenu = false;
    if (!title.isEmpty() && !url.isEmpty())
    {
        menu.insertItem(i18n("Current Document"), 1);
        menu.insertItem(i18n("Custom..."), 2);
        useMenu = true;
    }
    int mode = 2;
    if (useMenu)
    {
        m_addButton->setDown(true);
        mode = menu.exec(mapToGlobal(QPoint(m_addButton->x(), m_addButton->y()+m_addButton->height())));
        m_addButton->setDown(false);
    }

    switch (mode)
    {
        case 1:
            addBookmark(title, url);
            break;
        case 2:
            EditBookmarkDlg dlg(this);
            dlg.setCaption(i18n("Add Bookmark"));
/*            dlg.nameEdit->setText(title);
            dlg.locationEdit->setURL(url);*/
            dlg.nameEdit->setFocus();
            if (dlg.exec())
                addBookmark(dlg.nameEdit->text(), KURL(dlg.locationEdit->url()));
            m_addButton->setDown(false);
            break;
    }
}

void BookmarkView::addBookmark(const QString &title, const KURL &url)
{
    KBookmark bm = m_bmManager->root().addBookmark(m_bmManager, title, url);
    m_bmManager->save();
    
    DocBookmarkItem *item = 0;
    if (m_view->lastItem())
        item = dynamic_cast<DocBookmarkItem*>(m_view->lastItem());
    if (item == 0)
        item = new DocBookmarkItem(DocumentationItem::Document, m_view, bm.fullText());
    else
        item = new DocBookmarkItem(DocumentationItem::Document, m_view, item, bm.fullText());
    item->setURL(bm.url());
    item->setBookmark(bm);
}

void BookmarkView::itemMouseButtonPressed(int button, QListViewItem *item, const QPoint &pos, int c)
{
    if ((button != Qt::RightButton) || (!item))
        return;
    DocumentationItem *docItem = dynamic_cast<DocumentationItem*>(item);
    if (!docItem)
        return;
    
    DocUtils::docItemPopup(m_widget->part(), docItem, pos, false, true);
}

void BookmarkView::focusInEvent(QFocusEvent */*e*/)
{
    m_view->setFocus();
}

#include "bookmarkview.moc"

/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "filetreewidget.h"

#include <qheader.h>
#include <qpainter.h>
#include <qregexp.h>
#include <qstringlist.h>

#include <kdebug.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kfileitem.h>

#include "kdevcore.h"
#include "kdevproject.h"
#include "kdevpartcontroller.h"
#include "kdevmainwindow.h"
#include "domutil.h"
#include "fileviewpart.h"

class MyFileTreeViewItem : public KFileTreeViewItem
{
public:
    MyFileTreeViewItem( KFileTreeViewItem* parent, KFileItem* item, KFileTreeBranch* branch )
       : KFileTreeViewItem( parent, item, branch )
    {
        hideOrShow();
    }
    MyFileTreeViewItem( KFileTreeView* parent, KFileItem* item, KFileTreeBranch* branch )
       : KFileTreeViewItem( parent, item, branch )
    {
        hideOrShow();
    }

    virtual void paintCell(QPainter *p, const QColorGroup &cg,
                           int column, int width, int alignment);
    void hideOrShow();

protected:
    virtual int compare( QListViewItem *i, int col, bool ascending ) const;

private:
    FileTreeWidget* listView()
    { return static_cast<FileTreeWidget*>(QListViewItem::listView()); }
};

void MyFileTreeViewItem::hideOrShow()
{
    setVisible( listView()->shouldBeShown( this ) );

    MyFileTreeViewItem* item = static_cast<MyFileTreeViewItem*>(firstChild());
    while( item ) {
        item->hideOrShow();
        item = static_cast<MyFileTreeViewItem*>(item->nextSibling());
    }
}

void MyFileTreeViewItem::paintCell(QPainter *p, const QColorGroup &cg,
                             int column, int width, int alignment)
{
    if ( listView()->m_showNonProjectFiles &&
         listView()->m_projectFiles.contains( path() ) ) {
        QFont font(p->font());
        font.setBold(true);
        p->setFont(font);
    }
    QListViewItem::paintCell(p, cg, column, width, alignment);
}


int MyFileTreeViewItem::compare( QListViewItem *i, int col, bool ascending ) const
{
    KFileTreeViewItem* rhs = dynamic_cast<KFileTreeViewItem*> (i);
    if (rhs)
    {
        if (rhs->isDir() && !isDir())
            return (ascending) ? 1 : -1;
        else
            if (!rhs->isDir() && isDir())
                return (ascending) ? -1 : 1;
    }

    return QListViewItem::compare( i, col, ascending );
}


class MyFileTreeBranch : public KFileTreeBranch
{
public:
   MyFileTreeBranch( KFileTreeView* view, const KURL& url, const QString& name, const QPixmap& pix )
       : KFileTreeBranch( view, url, name, pix, false,
           new MyFileTreeViewItem(
               view, new KFileItem( url, "inode/directory", S_IFDIR  ), this ) )
   {
   }

   ~MyFileTreeBranch()
   {
       if( root() )
           delete( root()->fileItem() );
   }

   virtual KFileTreeViewItem* createTreeViewItem( KFileTreeViewItem* parent, KFileItem* fileItem )
   {
       if( !parent || !fileItem )
           return 0;
       return new MyFileTreeViewItem( parent, fileItem, this );
   }
};

FileTreeWidget::FileTreeWidget(FileViewPart *part, QWidget *parent, const char *name)
    : KFileTreeView(parent, name)
{
    setResizeMode(QListView::LastColumn);
    setSorting(0);
    header()->hide();
    addColumn(QString::null);

    setDragEnabled( true );

    m_part = part;

    connect( this, SIGNAL(executed(QListViewItem*)),
             this, SLOT(slotItemExecuted(QListViewItem*)) );
    connect( this, SIGNAL(returnPressed(QListViewItem*)),
             this, SLOT(slotItemExecuted(QListViewItem*)) );
    connect( this, SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint&)),
             this, SLOT(slotContextMenu(KListView*, QListViewItem*, const QPoint&)) );

    QDomDocument &dom = *m_part->projectDom();
    m_showNonProjectFiles = !DomUtil::readBoolEntry(dom, "/kdevfileview/tree/hidenonprojectfiles");

 // Comment out until config UI is implemented
    QString patterns; // = DomUtil::readEntry(dom, "/kdevfileview/tree/hidepatterns");
//    if (patterns.isEmpty())
        patterns = "*.o,*.lo,CVS";
    m_hidePatterns = QStringList::split(",", patterns);
}

FileTreeWidget::~FileTreeWidget()
{
    QDomDocument &dom = *m_part->projectDom();
    DomUtil::writeBoolEntry( dom, "/kdevfileview/tree/hidenonprojectfiles", !m_showNonProjectFiles );
    DomUtil::writeEntry( dom, "/kdevfileview/tree/hidepatterns", m_hidePatterns.join(",") );
}

void FileTreeWidget::openDirectory( const QString& dirName )
{
	QStringList::iterator it;

	QStringList fileList = m_part->project()->allFiles();

	for ( it = fileList.begin(); it != fileList.end(); ++it )
	{
    	m_projectFiles.append ( m_part->project()->projectDirectory() + "/" + ( *it ) );
	}

    KURL url;
    url.setPath( dirName );
    const QPixmap& pix = KMimeType::mimeType("inode/directory")->pixmap( KIcon::Small );
    addBranch( new MyFileTreeBranch( this, url, url.prettyURL(), pix ) )->root()->setOpen( true );
}

bool FileTreeWidget::shouldBeShown( KFileTreeViewItem* item )
{
    return( (m_showNonProjectFiles || item->isDir() || m_projectFiles.contains( item->path() ))
             && !matchesHidePattern( item->text(0) ) );
}

bool FileTreeWidget::matchesHidePattern(const QString &fileName)
{
    QStringList::ConstIterator it;
    for (it = m_hidePatterns.begin(); it != m_hidePatterns.end(); ++it) {
        QRegExp re(*it, true, true);
        if (re.search(fileName) == 0 && (uint)re.matchedLength() == fileName.length())
            return true;
    }

    return false;
}

void FileTreeWidget::hideOrShow()
{
    MyFileTreeViewItem* item = static_cast<MyFileTreeViewItem*>(firstChild());
    if( !item )
      return;

    // Need to skip the root item.
    item = static_cast<MyFileTreeViewItem*>(item->firstChild());
    while( item ) {
        item->hideOrShow();
        item = static_cast<MyFileTreeViewItem*>(item->nextSibling());
    }
}

void FileTreeWidget::slotItemExecuted( QListViewItem* item )
{
    if (!item)
        return;

    KFileTreeViewItem* ftitem = static_cast<KFileTreeViewItem*>(item);

    if( ftitem->isDir() )
        return;

    m_part->partController()->editDocument( ftitem->url() );
    m_part->mainWindow()->lowerView( this );
}

void FileTreeWidget::slotContextMenu( KListView*, QListViewItem* item, const QPoint &p )
{
    KPopupMenu popup(i18n("File Tree"), this);

    int id = popup.insertItem( i18n("Show Non-Project Files"),
                               this, SLOT(slotToggleShowNonProjectFiles()) );
    popup.setItemChecked(id, m_showNonProjectFiles);

    if( item != 0 ) {
      KFileTreeViewItem* ftitem = static_cast<KFileTreeViewItem*>(item);
      FileContext context( ftitem->path(), ftitem->isDir() );
      m_part->core()->fillContextMenu(&popup, &context);
    }

    popup.exec(p);
}

void FileTreeWidget::slotToggleShowNonProjectFiles()
{
    m_showNonProjectFiles = !m_showNonProjectFiles;
    hideOrShow();
}

#include "filetreewidget.moc"

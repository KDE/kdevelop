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
#include <kurl.h>

#include "kdevcore.h"
#include "kdevproject.h"
#include "kdevpartcontroller.h"
#include "kdevmainwindow.h"
#include "domutil.h"
#include "urlutil.h"

#include "fileviewpart.h"

class MyFileTreeViewItem : public KFileTreeViewItem
{
public:
    MyFileTreeViewItem( KFileTreeViewItem* parent, KFileItem* item, KFileTreeBranch* branch, bool pf )
       : KFileTreeViewItem( parent, item, branch ), _isProjectFile( pf )
    {
        hideOrShow();
    }
    MyFileTreeViewItem( KFileTreeView* parent, KFileItem* item, KFileTreeBranch* branch )
       : KFileTreeViewItem( parent, item, branch ), _isProjectFile( false )
    {
        hideOrShow();
    }

    virtual void paintCell(QPainter *p, const QColorGroup &cg,
                           int column, int width, int alignment);
    void hideOrShow();

    FileTreeWidget* listView()
    {
        return static_cast<FileTreeWidget*>(QListViewItem::listView());
    }

    bool isProjectFile()
    {
        return _isProjectFile;
    }

    bool setProjectFile( QString const & path, bool pf );

protected:
    virtual int compare( QListViewItem *i, int col, bool ascending ) const;

private:

    bool _isProjectFile;
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

bool MyFileTreeViewItem::setProjectFile( QString const & path, bool pf )
{
    if ( this->path() == path )
    {
        _isProjectFile = pf;
        setVisible( listView()->shouldBeShown( this ) );
        repaint();
        return true;
    }

    MyFileTreeViewItem* item = static_cast<MyFileTreeViewItem*>(firstChild());
    while( item )
    {
        if ( item->setProjectFile( path, pf ) )
        {
            return true;
        }
        else
        {
            item = static_cast<MyFileTreeViewItem*>(item->nextSibling());
        }
    }
    return false;
}

void MyFileTreeViewItem::paintCell(QPainter *p, const QColorGroup &cg,
                             int column, int width, int alignment)
{
    if ( listView()->m_showNonProjectFiles && isProjectFile() )
    {
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

        FileTreeWidget * lv = static_cast<MyFileTreeViewItem*>(parent)->listView();
        return new MyFileTreeViewItem( parent, fileItem, this, lv->projectFiles().contains( fileItem->url().path() ) > 0 );
    }
};

FileTreeWidget::FileTreeWidget(FileViewPart *part, QWidget *parent, const char *name)
    : KFileTreeView(parent, name), m_rootBranch( 0 )
{
    setResizeMode(QListView::LastColumn);
    setSorting(0);
    header()->hide();
    addColumn(QString::null);
    setSelectionMode( QListView::Extended ); // Enable multiple items selection by use of Ctrl/Shift
    setDragEnabled( false );

    m_part = part;

    connect( this, SIGNAL(executed(QListViewItem*)),
             this, SLOT(slotItemExecuted(QListViewItem*)) );
    connect( this, SIGNAL(returnPressed(QListViewItem*)),
             this, SLOT(slotItemExecuted(QListViewItem*)) );
    connect( this, SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint&)),
             this, SLOT(slotContextMenu(KListView*, QListViewItem*, const QPoint&)) );
    connect( this, SIGNAL(selectionChanged()),
             this, SLOT(slotSelectionChanged()) );
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
    // if we're reloading
    if ( m_rootBranch )
    {
        removeBranch( m_rootBranch );
        m_projectFiles.clear();
    }

    addProjectFiles( m_part->project()->allFiles(), true );

    KURL url;
    url.setPath( dirName );
    const QPixmap& pix = KMimeType::mimeType("inode/directory")->pixmap( KIcon::Small );

    // this is a bit odd, but the order of these calls seems to be important
    MyFileTreeBranch * b = new MyFileTreeBranch( this, url, url.prettyURL(), pix );
    b->setChildRecurse( false );
    m_rootBranch = addBranch( b );
    m_rootBranch->setOpen( true );
}

bool FileTreeWidget::shouldBeShown( KFileTreeViewItem* item )
{
    MyFileTreeViewItem * i = static_cast<MyFileTreeViewItem *>( item );
    return( (m_showNonProjectFiles || item->isDir() || i->isProjectFile() )
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

void FileTreeWidget::slotContextMenu( KListView *, QListViewItem* item, const QPoint &p )
{
    KPopupMenu popup(i18n("File Tree"), this);

    if ( item == this->firstChild() ) // rootnode
    {
        popup.insertItem( i18n( "Reload Tree"), this, SLOT( slotReloadTree() ) );
    }

    int id = popup.insertItem( i18n("Show Non-Project Files"),
                               this, SLOT(slotToggleShowNonProjectFiles()) );
    popup.setItemChecked(id, m_showNonProjectFiles);

    if( item != 0 ) {
//      KFileTreeViewItem* ftitem = static_cast<KFileTreeViewItem*>(item);
//      FileContext context( ftitem->path(), ftitem->isDir() );

      FileContext context( selectedPathUrls() );

      m_part->core()->fillContextMenu(&popup, &context);
    }

    popup.exec(p);
}

void FileTreeWidget::slotToggleShowNonProjectFiles()
{
    m_showNonProjectFiles = !m_showNonProjectFiles;
    hideOrShow();
}

void FileTreeWidget::slotReloadTree()
{
    openDirectory( projectDirectory() );
}

QString FileTreeWidget::projectDirectory()
{
    return m_part->project()->projectDirectory();
}

QStringList FileTreeWidget::projectFiles()
{
    return m_projectFiles;
}

void FileTreeWidget::addProjectFiles( QStringList const & fileList, bool constructing )
{
    kdDebug(9017) << "files added to project: " << fileList.count() << endl;

    QStringList::ConstIterator it;
    for ( it = fileList.begin(); it != fileList.end(); ++it )
    {
        QString file = m_part->project()->projectDirectory() + "/" + ( *it );
        if ( ! m_projectFiles.contains( file ) )
        {
            m_projectFiles.append( file );
//            kdDebug(9017) << "file added: " << file << endl;
        }

        if ( ! constructing )
        {
            MyFileTreeViewItem* item = static_cast<MyFileTreeViewItem*>(firstChild());
            if( item )
            {
                item->setProjectFile( file, true );
            }
        }
    }
}

void FileTreeWidget::removeProjectFiles( QStringList const & fileList )
{
    kdDebug(9017) << "files removed from project: " << fileList.count() << endl;

    QStringList::ConstIterator it;
    for ( it = fileList.begin(); it != fileList.end(); ++it )
    {
        QString file = m_part->project()->projectDirectory() + "/" + ( *it );
        m_projectFiles.remove( file );
        kdDebug(9017) << "file removed: " << file << endl;

        MyFileTreeViewItem* item = static_cast<MyFileTreeViewItem*>(firstChild());
        if( item )
        {
            item->setProjectFile( file, false );
        }
    }
}

void FileTreeWidget::slotSelectionChanged()
{
    kdDebug(9017) << "FileTreeWidget::slotSelectionChanged()" << endl;

    // Check for this item
    MyFileTreeViewItem *item = static_cast<MyFileTreeViewItem*>( currentItem() );
    if (!item)
        return;
    if (item->isSelected())
    {
        if (m_selectedItems.find( item ) != -1)
        {
        kdDebug(9017) << "Warning: Item " << item->path() << " is already present. Skipping." << endl;
        return;
        }
        m_selectedItems.append( item );

        kdDebug(9017) << "Added item: " << item->path() << " ( " << m_selectedItems.count() << " )" << endl;
    }
    else // It has	 been removed
    {
        m_selectedItems.remove( item );

        kdDebug(9017) << "Removed item: " << item->path() << " ( " << m_selectedItems.count() << " )" << endl;
    }

    // Now we clean-up the selection of old elements which are no more selected.
    // FIXME: Any better way?
    KFileTreeViewItem *it = m_selectedItems.first();
    while (it != 0)
    {
        if (!it->isSelected()) {
            KFileTreeViewItem *toDelete = it;
            it = m_selectedItems.next();
            m_selectedItems.remove( toDelete );
        }
        else
        {
            it = m_selectedItems.next();
        }
    }
}

KURL::List FileTreeWidget::selectedPathUrls()
{
    kdDebug(9017) << "FileTreeWidget::selectedPathUrls()" << endl;

    QStringList pathUrls;

    // They should be all selected but I want to be sure about this.
    MyFileTreeViewItem *item = static_cast<MyFileTreeViewItem *>( m_selectedItems.first() );
    while (item)
    {
        if (item->isSelected())
        {
            pathUrls << item->path();

            kdDebug(9017) << "Added path " << item->path() << endl;
        }
        item = static_cast<MyFileTreeViewItem *>( m_selectedItems.next() );
    }

    return KURL::List( pathUrls );
}

#include "filetreewidget.moc"

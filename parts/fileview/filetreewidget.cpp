/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2003 by Mario Scalas (VCS Support)                      *
 *   mario.scalas@libero.it                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

 /**
  * @todo This piece of code is quite bulky: it should be refactored,
  * putting in a separate class hierarchy the knowledge about vcs file info
  * provider objects and provide ad-hoc factory class. (This is a memo for
  * mysqlf (mario) or for whoever has some nice ideas about fixing this
  * rather hacked code ;-)
 */

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
#include <kaction.h>

#include "kdevcore.h"
#include "kdevproject.h"
#include "kdevpartcontroller.h"
#include "kdevmainwindow.h"
#include "kdevversioncontrol.h"
#include "domutil.h"
#include "urlutil.h"

#include "fileviewpart.h"

#define FILENAME_COLUMN     0
#define STATUS_COLUMN       1
#define WORKREVISION_COLUMN 2
#define REPOREVISION_COLUMN 3

///////////////////////////////////////////////////////////////////////////////
// class MyFileTreeViewItem
///////////////////////////////////////////////////////////////////////////////

#include <kdeversion.h>
#if (KDE_VERSION_MINOR==0) && (KDE_VERSION_MAJOR==3)
#include <kdevkurl.h>
#define KURL KdevKURL
#endif


class MyFileTreeViewItem : public KFileTreeViewItem
{
public:
    MyFileTreeViewItem( KFileTreeViewItem* parent, KFileItem* item, KFileTreeBranch* branch, bool pf )
       : KFileTreeViewItem( parent, item, branch ), m_isProjectFile( pf ), m_status( VCSFileInfo::Unknown )
    {
        hideOrShow();
    }
    MyFileTreeViewItem( KFileTreeView* parent, KFileItem* item, KFileTreeBranch* branch )
       : KFileTreeViewItem( parent, item, branch ), m_isProjectFile( false ), m_status( VCSFileInfo::Directory )
    {
        hideOrShow();
    }

    virtual void paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int alignment );
    //! Update content
    void hideOrShow();
    //! update VCS info for this directory
    //void updateVCSInfo( const VCSFileInfoMap &vcsFileInfo );
    void setVCSInfo( const VCSFileInfo &info );
    bool setProjectFile( QString const & path, bool pf );

    FileTreeWidget* listView() const { return static_cast<FileTreeWidget*>( QListViewItem::listView() ); }
    bool isProjectFile() const { return m_isProjectFile; }

    QString fileName() const { return text( FILENAME_COLUMN ); }
    QString workingRev() const { return text( WORKREVISION_COLUMN ); }
    QString repositoryRev() const { return text( REPOREVISION_COLUMN ); }
    QString status() const { return text( STATUS_COLUMN ); }
    void setFileName( const QString &p ) { setText( FILENAME_COLUMN, p ); }
    void setWorkingRev( const QString &p ) { setText( WORKREVISION_COLUMN, p ); }
    void setRepositoryRev( const QString &p ) { setText( REPOREVISION_COLUMN, p ); }
    void setStatus( const VCSFileInfo::FileStatus status ) { setText( STATUS_COLUMN, VCSFileInfo::vcsState2String( status ) ); }

protected:
    virtual int compare( QListViewItem *i, int col, bool ascending ) const;

private:
    bool m_isProjectFile;
    VCSFileInfo::FileStatus m_status;
};

///////////////////////////////////////////////////////////////////////////////

void MyFileTreeViewItem::hideOrShow()
{
    kdDebug( 9017 ) << "MyFileTreeViewItem::hideOrShow(): " + path() << endl;
    setVisible( listView()->shouldBeShown( this ) );

    MyFileTreeViewItem* item = static_cast<MyFileTreeViewItem*>( firstChild() );
    while (item)
    {
        //kdDebug( 9017 ) << "MyFileTreeViewItem::hideOrShow(): " + item->path() << endl;
        item->hideOrShow();
        item = static_cast<MyFileTreeViewItem*>( item->nextSibling() );
    }
}

///////////////////////////////////////////////////////////////////////////////

void MyFileTreeViewItem::setVCSInfo( const VCSFileInfo &info )
{
    //setFileName( info.fileName );
    setRepositoryRev( info.repoRevision );
    setWorkingRev( info.workRevision );
    setStatus( info.state );
}

///////////////////////////////////////////////////////////////////////////////

bool MyFileTreeViewItem::setProjectFile( QString const & path, bool pf )
{
    kdDebug( 9017 ) << "MyFileTreeViewItem::setProjectFile(): " + path << endl;

    if ( this->path() == path )
    {
        m_isProjectFile = pf;
        setVisible( listView()->shouldBeShown( this ) );
        repaint();
        return true;
    }

    MyFileTreeViewItem* item = static_cast<MyFileTreeViewItem*>( firstChild() );
    while( item )
    {
        if ( item->setProjectFile( path, pf ) )
            return true;
        else
            item = static_cast<MyFileTreeViewItem*>(item->nextSibling());
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

void MyFileTreeViewItem::paintCell(QPainter *p, const QColorGroup &cg,
                             int column, int width, int alignment)
{
    if ( listView()->showNonProjectFiles() && isProjectFile() )
    {
        QFont font( p->font() );
        font.setBold( true );
        p->setFont( font );

        // @todo paint cell in a different color
        switch (status)
        {
            case VCSFileInfo::Added: break;
            case VCSFileInfo::Uptodate: break;
            case VCSFileInfo::Modified: break;
            case VCSFileInfo::Conflict: break;
            case VCSFileInfo::Sticky: break;
            case VCSFileInfo::Unknown:
            default:
                break;
        }
    }

    QListViewItem::paintCell(p, cg, column, width, alignment);
}

///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////
// class MyFileTreeBranch
///////////////////////////////////////////////////////////////////////////////

class MyFileTreeBranch : public KFileTreeBranch
{
public:
    MyFileTreeBranch( KFileTreeView* view, const KURL& url, const QString& name, const QPixmap& pix )
        : KFileTreeBranch( view, url, name, pix, false,
            new MyFileTreeViewItem( view, new KFileItem( url, "inode/directory", S_IFDIR ), this ) )
    {
        kdDebug( 9017 ) << "MyFileTreeBranch::MyFileTreeBranch(): " << url.path() << endl;
    }

    virtual ~MyFileTreeBranch()
    {
        if (root())
            delete root()->fileItem();
    }

    virtual KFileTreeViewItem* createTreeViewItem( KFileTreeViewItem* parent, KFileItem* fileItem )
    {
        kdDebug(9017) << "MyFileTreeBranch::createTreeViewItem(): " + fileItem->url().path() << endl;
        if (!parent || !fileItem)
            return 0;

        FileTreeWidget *lv = static_cast<MyFileTreeViewItem*>( parent )->listView();
        const KURL fileURL = fileItem->url();
        bool isDirectory = lv->projectFiles().contains( fileURL.path() ) > 0;
        KDevVCSFileInfoProvider *infoProvider = lv->vcsFileInfoProvider();

        MyFileTreeViewItem *newItem = new MyFileTreeViewItem( parent, fileItem, this, isDirectory );

        if (infoProvider)
        {
            QString fileName = fileURL.fileName();
            QString dirName = URLUtil::extractPathNameRelative( lv->projectDirectory(), fileURL.directory() );
            kdDebug(9017) << "searching for file: " + fileName + " in directory " + dirName << endl;

            const VCSFileInfoMap &vcsFiles = *infoProvider->status( dirName );
            kdDebug(9017) << "Dir has " << vcsFiles.count() << " registered files!" << endl;

            if (vcsFiles.contains( fileName ))
            {
                newItem->setVCSInfo( vcsFiles[ fileName ] );
                //kdDebug(9017) << "Found vcs info: " << vcsFileInfo.toString() << endl;
                if (URLUtil::isDirectory(fileURL))
                    newItem->setStatus( "directory" );
            }
            else
                kdDebug(9017) << "!!!No VCS info for this file!!!" << endl;
        }
        return newItem;
    }
};

///////////////////////////////////////////////////////////////////////////////
// class FileTreeWidget
///////////////////////////////////////////////////////////////////////////////

FileTreeWidget::FileTreeWidget(FileViewPart *part, QWidget *parent, const char *name)
    : KFileTreeView(parent, name), m_part( part ), m_rootBranch( 0 ),
    m_isReloadingTree( false ), m_isSyncingWithRepository( false )
    m_actionToggleShowVCSFields( 0 ), m_actionToggleShowNonProjectFiles( 0 )
{
    //setResizeMode( QListView::LastColumn );
    setSorting( 0 );
    setAllColumnsShowFocus( true );
    setSelectionMode( QListView::Extended ); // Enable multiple items selection by use of Ctrl/Shift
    setDragEnabled( false );

    connect( this, SIGNAL(executed(QListViewItem*)),
             this, SLOT(slotItemExecuted(QListViewItem*)) );
    connect( this, SIGNAL(returnPressed(QListViewItem*)),
             this, SLOT(slotItemExecuted(QListViewItem*)) );
    connect( this, SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint&)),
             this, SLOT(slotContextMenu(KListView*, QListViewItem*, const QPoint&)) );
    connect( this, SIGNAL(selectionChanged()),
             this, SLOT(slotSelectionChanged()) );
    // Intercepts KDevelop core signals and VCS notifications (if available)
    connect( m_part->project(), SIGNAL( addedFilesToProject( const QStringList & ) ),
             this, SLOT( addProjectFiles( const QStringList & ) ) );
    connect( m_part->project(), SIGNAL( removedFilesFromProject( const QStringList & ) ),
             this, SLOT( removeProjectFiles( const QStringList & ) ) );
    // We can do this since the version control has global scope while the file tree has project scope: hence
    // the former is always loaded first.
    if (vcsFileInfoProvider())
    {
        connect( vcsFileInfoProvider(), SIGNAL(statusReady(const VCSFileInfoMap&, void *)),
            this, SLOT(vcsDirStatusReady(const VCSFileInfoMap&, void*)) );
    }
    // Update the #define order on top if you change this order!
    addColumn( "Filename" );
    addColumn( "Status" );
    addColumn( "Work" );
    addColumn( "Repo." );

    // Actions
    m_actionToggleShowNonProjectFiles = new KToggleAction( i18n("Show Non Project Files"), KShortcut(),
        this, SLOT(slotToggleShowNonProjectFiles()), this, "actiontoggleshowshownonprojectfiles" );
    m_actionToggleShowNonProjectFiles->setWhatsThis(i18n("<b>Show non project files</b><p>Shows files that do not belong to a project in a file tree."));

    m_actionToggleShowVCSFields = new KToggleAction( i18n("Show VCS Fields"), KShortcut(),
        this, SLOT(slotToggleShowVCSFields()), this, "actiontoggleshowvcsfieldstoggleaction" );
    m_actionToggleShowVCSFields->setWhatsThis(i18n("<b>Show VCS fields</b><p>Shows <b>Revision</b> and <b>Timestamp</b> for each file contained in VCS repository."));
    connect( m_actionToggleShowVCSFields, SIGNAL(toggled(bool)), this, SLOT(slotToggleShowVCSFields(bool)) );

    // Reload ol' good settings
    QDomDocument &dom = *m_part->projectDom();
    m_actionToggleShowNonProjectFiles->setChecked( !DomUtil::readBoolEntry(dom, "/kdevfileview/tree/hidenonprojectfiles") );
    m_actionToggleShowVCSFields->setChecked( DomUtil::readBoolEntry(dom, "/kdevfileview/tree/showvcsfields") );
    slotToggleShowVCSFields( showVCSFields() );

    // Hide pattern for files
    QString defaultHidePattern = "*.o,*.lo,CVS";
    QString hidePattern = DomUtil::readEntry( dom, "/kdevfileview/tree/hidepatterns", defaultHidePattern );
    m_hidePatterns = QStringList::split( ",", hidePattern );
}

///////////////////////////////////////////////////////////////////////////////

FileTreeWidget::~FileTreeWidget()
{
    QDomDocument &dom = *m_part->projectDom();
    DomUtil::writeBoolEntry( dom, "/kdevfileview/tree/hidenonprojectfiles", !showNonProjectFiles() );
    DomUtil::writeBoolEntry( dom, "/kdevfileview/tree/showvcsfields", showVCSFields() );
    DomUtil::writeEntry( dom, "/kdevfileview/tree/hidepatterns", hidePatterns() );
}

///////////////////////////////////////////////////////////////////////////////

void FileTreeWidget::openDirectory( const QString& dirName )
{
    kdDebug(9017) << "FileTreeWidget::openDirectory(): " + dirName << endl;

    // if we're reloading
    if (m_rootBranch)
    {
        removeBranch( m_rootBranch );
        m_projectFiles.clear();
    }

    addProjectFiles( m_part->project()->allFiles(), true );

    KURL url = KURL::fromPathOrURL( dirName );

    const QPixmap& pix = KMimeType::mimeType("inode/directory")->pixmap( KIcon::Small );

    // this is a bit odd, but the order of these calls seems to be important
    MyFileTreeBranch *b = new MyFileTreeBranch( this, url, url.prettyURL(), pix );
    b->setChildRecurse( false );
    m_rootBranch = addBranch( b );
    m_rootBranch->setOpen( true );
}

///////////////////////////////////////////////////////////////////////////////

bool FileTreeWidget::shouldBeShown( KFileTreeViewItem* item )
{
    MyFileTreeViewItem * i = static_cast<MyFileTreeViewItem *>( item );
    return( (showNonProjectFiles() || i->isDir() || i->isProjectFile() )
             && !matchesHidePattern( i->fileName() ) );
}

///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////

void FileTreeWidget::hideOrShow()
{
    // This is called the first time the tree branch is expanded
    MyFileTreeViewItem* item = static_cast<MyFileTreeViewItem*>(firstChild());
    if( !item )
      return;

    // Need to skip the root item (which is the sub-directory)
    // i.e. "/home/devmario/src/kdevelop/parts/cvsservice"
    item = static_cast<MyFileTreeViewItem*>( item->firstChild() );
/*
    // Grab info from the info provider for this directory
    KDevVCSFileInfoProvider *vcsInfoProvider = vcsFileInfoProvider();
    VCSFileInfoMap vcsFiles;
    if (vcsInfoProvider && item)
    {
        QString relDirPath = URLUtil::extractPathNameRelative( projectDirectory(), item->path() );
        vcsFiles = vcsInfoProvider->status( relDirPath );
    }
*/
    // Now fill the sub-tree
    while (item)
    {
        item->hideOrShow();
/*
        QString fileName = item->url().fileName();
        if (vcsFiles.contains( fileName ))
            item->setVCSInfo( vcsFiles[fileName] );
*/
        item = static_cast<MyFileTreeViewItem*>(item->nextSibling());
    }
}

///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////

void FileTreeWidget::slotContextMenu( KListView *, QListViewItem* item, const QPoint &p )
{
    kdDebug(9017) << "FileTreeWidget::slotContextMenu(...)" << endl;

    KPopupMenu popup( i18n("File Tree"), this );

    if (item == this->firstChild() && !m_isSyncingWithRepository) // rootnode
    {
        int id = popup.insertItem( i18n( "Reload Tree"), this, SLOT( slotReloadTree() ) );
        popup.setWhatsThis( id, i18n("<b>Reload tree</b><p>Reloads the project files tree.") );
    }

    // Submenu for visualization options
    m_actionToggleShowVCSFields->plug( &popup );
    m_actionToggleShowNonProjectFiles->plug( &popup );

    // Give a change for syncing status with remote repository: a file info provider must
    // be available and the item must be a directory (so we can safely use isExpandable()?)
    if (vcsFileInfoProvider())
    {
        const MyFileTreeViewItem *myFileItem = static_cast<MyFileTreeViewItem *>( item );
        // @fixme Is there a better way to find out if a node is a folder or leaf (i.e. playing with the
        // QListView) ?
        if (URLUtil::isDirectory( myFileItem->url() ))
        {
            m_vcsStatusRequestedItem = item;
            int id = popup.insertItem( i18n( "Sync with repository"), this, SLOT( slotSyncWithRepository() ) );
            popup.setWhatsThis( id,
                i18n("<b>Sync with repository</b><p>Synchronize file status with remote repository.") );
        }
    }
    // If an item is selected, fill the file context with selected files' list
    if (item != 0)
    {
        FileContext context( selectedPathUrls() );
        m_part->core()->fillContextMenu( &popup, &context );
    }

    popup.exec( p );
}

///////////////////////////////////////////////////////////////////////////////

void FileTreeWidget::slotToggleShowNonProjectFiles()
{
    hideOrShow();
}

///////////////////////////////////////////////////////////////////////////////

void FileTreeWidget::slotReloadTree()
{
//    m_isReloadingTree = true;
    m_selectedItems.clear();
    openDirectory( projectDirectory() );
//    m_isReloadingTree = false;
}

///////////////////////////////////////////////////////////////////////////////

QString FileTreeWidget::projectDirectory()
{
    return m_part->project()->projectDirectory();
}

///////////////////////////////////////////////////////////////////////////////

QStringList FileTreeWidget::projectFiles()
{
    return m_projectFiles;
}

///////////////////////////////////////////////////////////////////////////////

void FileTreeWidget::addProjectFiles( QStringList const & fileList, bool constructing )
{
    kdDebug(9017) << "files added to project: " << fileList.count() << endl;

    QStringList::ConstIterator it;
    for ( it = fileList.begin(); it != fileList.end(); ++it )
    {
        QString file = projectDirectory() + "/" + ( *it );
        if ( !m_projectFiles.contains( file ) )
        {
            m_projectFiles.append( file );
//            kdDebug(9017) << "file added: " << file << endl;
        }

        if ( !constructing )
        {
            MyFileTreeViewItem* item = static_cast<MyFileTreeViewItem*>(firstChild());
            if( item )
            {
                item->setProjectFile( file, true );
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////

void FileTreeWidget::slotSelectionChanged()
{
    kdDebug(9017) << "FileTreeWidget::slotSelectionChanged()" << endl;

    if (m_isReloadingTree)
        return;

    // Check for this item
    MyFileTreeViewItem *item = static_cast<MyFileTreeViewItem*>( currentItem() );
    if (!item)
        return;
    if (item->isSelected())
    {
        if (m_selectedItems.find( item ) != -1)
            return;
        m_selectedItems.append( item );
    }
    else // It has	 been removed
        m_selectedItems.remove( item );

    // Now we clean-up the selection of old elements which are no more selected.
    KFileTreeViewItem *it = m_selectedItems.first();
    while (it != 0)
    {
        if (!it->isSelected()) {
            KFileTreeViewItem *toDelete = it;
            it = m_selectedItems.next();
            m_selectedItems.remove( toDelete );
        }
        else
            it = m_selectedItems.next();
    }
}

///////////////////////////////////////////////////////////////////////////////

KURL::List FileTreeWidget::selectedPathUrls()
{
    kdDebug(9017) << "FileTreeWidget::selectedPathUrls()" << endl;

    if (m_isReloadingTree)
        return KURL::List();

    QStringList pathUrls;

    // They should be all selected but I want to be sure about this.
    MyFileTreeViewItem *item = static_cast<MyFileTreeViewItem *>( m_selectedItems.first() );
    while (item)
    {
        if (item->isSelected())
            pathUrls << item->path();
        item = static_cast<MyFileTreeViewItem *>( m_selectedItems.next() );
    }

    return KURL::List( pathUrls );
}

///////////////////////////////////////////////////////////////////////////////

bool FileTreeWidget::showVCSFields() const
{
    return m_actionToggleShowVCSFields->isChecked();
}

///////////////////////////////////////////////////////////////////////////////

bool FileTreeWidget::showNonProjectFiles() const
{
    return m_actionToggleShowNonProjectFiles->isChecked();
}

///////////////////////////////////////////////////////////////////////////////

void FileTreeWidget::applyHidePatterns( const QString &hidePatterns )
{
    m_hidePatterns = QStringList::split( ",", hidePatterns );
    hideOrShow();
}

///////////////////////////////////////////////////////////////////////////////

QString FileTreeWidget::hidePatterns() const
{
    return m_hidePatterns.join( "," );
}

///////////////////////////////////////////////////////////////////////////////

void FileTreeWidget::slotToggleShowVCSFields( bool checked )
{
    kdDebug(9017) << "FileTreeWidget::slotToggleShowVCSFields()" << endl;
    kdDebug(9017) << "Yet to be implemented!!" << endl;

    // @fixme: even if I hide the columns they are taken in account when resizing
    // the list view :-/
    if (checked)
    {
        setColumnWidth( 0, contentsWidth() / 3 ); // "Filename"
        setColumnWidth( 1, contentsWidth() / 3 ); // "Revision"
        setColumnWidth( 2, contentsWidth() / 3 ); // "Timestamp"

        header()->show();
    }
    else
    {
        header()->hide();
        setColumnWidth( 2 ,0 ); // Hide columns
        setColumnWidth( 1, 0 );
        setColumnWidth( 0, contentsWidth() ); // Make the column to occupy all the row
    }

    triggerUpdate();
}

///////////////////////////////////////////////////////////////////////////////

void FileTreeWidget::vcsDirStatusReady( const VCSFileInfoMap &modifiedFiles, void *callerData )
{
    kdDebug(9017) << "FileTreeWidget::vcsDirStatusReady(const VCSFileInfoMap &, void*)" << endl;

    MyFileTreeViewItem *item = static_cast<MyFileTreeViewItem*>( callerData );
    Q_ASSERT( item ); // this must _not_ fail!
    // Update vcs file info for all childs in this tree folder ...
    item = static_cast<MyFileTreeViewItem*>( item->firstChild() );
    while (item)
    {
        const QString fileName = item->fileName();
        kdDebug(9017) << "Widget item filename is: " << fileName << endl;
        if (modifiedFiles.contains( fileName ))
        {
            const VCSFileInfo &vcsInfo = modifiedFiles[ fileName ];
            kdDebug(9017) << "Nice! Found info for this file: " << vcsInfo.toString() << endl;
            item->setVCSInfo( modifiedFiles[ fileName ] );
        }
        else
            kdDebug(9017) << "Map does not contain anything useful about this file ;-(" << fileName << endl;
        item = static_cast<MyFileTreeViewItem*>( item->nextSibling() );
    }
    triggerUpdate(); // @fixme Paint only the updated widget's region ?

    m_isSyncingWithRepository = false;
}

///////////////////////////////////////////////////////////////////////////////

void FileTreeWidget::slotSyncWithRepository()
{
    kdDebug(9017) << "FileTreeWidget::slotSyncWithRepository()" << endl;
    const MyFileTreeViewItem *myFileItem = static_cast<MyFileTreeViewItem *>( m_vcsStatusRequestedItem );
    const QString relDirPath = URLUtil::extractPathNameRelative( projectDirectory(), myFileItem->fileItem()->url().path() );
    kdDebug(9017) << "VCS Info requested for: " << relDirPath << endl;
    vcsFileInfoProvider()->requestStatus( relDirPath, m_vcsStatusRequestedItem );
    m_isSyncingWithRepository = true;
}

///////////////////////////////////////////////////////////////////////////////

KDevVCSFileInfoProvider *FileTreeWidget::vcsFileInfoProvider() const
{
    if (part() && part()->versionControl() && part()->versionControl()->fileInfoProvider())
        return part()->versionControl()->fileInfoProvider();
    else
        return 0;
}

#include "filetreewidget.moc"

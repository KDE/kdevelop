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

#include "filetreewidget.h"

#include <qheader.h>
#include <qpainter.h>
#include <qregexp.h>
#include <qstringlist.h>
#include <qcolor.h>

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
#include "fileitemfactory.h"
#include "vcsfiletreewidgetimpl.h"
#include "stdfiletreewidgetimpl.h"

using namespace filetreeview;

///////////////////////////////////////////////////////////////////////////////
// class FileTreeViewItem
///////////////////////////////////////////////////////////////////////////////

#include <kdeversion.h>

///////////////////////////////////////////////////////////////////////////////
// class FileTreeWidget
///////////////////////////////////////////////////////////////////////////////

FileTreeWidget::FileTreeWidget( FileViewPart *part, QWidget *parent, KDevVCSFileInfoProvider *infoProvider )
    : KFileTreeView( parent, "filetreewidget" ), m_part( part ), m_rootBranch( 0 )
{
    kdDebug(9017) << "Requested FileTree for: " << projectDirectory() << endl;
    if (versionControl() && infoProvider)
        kdDebug(9017) << "Valid VCS directory: " << versionControl()->isValidDirectory( projectDirectory() ) << endl;

    if (infoProvider && versionControl() && versionControl()->isValidDirectory( projectDirectory() ))
        m_impl = new VCSFileTreeWidgetImpl( this, infoProvider );
    else
        m_impl = new StdFileTreeWidgetImpl( this );

    //setResizeMode( QListView::LastColumn );
    setSorting( 0 );
    setAllColumnsShowFocus( true );
    setSelectionMode( QListView::Extended ); // Enable multiple items selection by use of Ctrl/Shift
    setDragEnabled( false );

    // Slot connections
    connect( this, SIGNAL(executed(QListViewItem*)), this, SLOT(slotItemExecuted(QListViewItem*)) );
    connect( this, SIGNAL(returnPressed(QListViewItem*)), this, SLOT(slotItemExecuted(QListViewItem*)) );
    connect( this, SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint&)),
             this, SLOT(slotContextMenu(KListView*, QListViewItem*, const QPoint&)) );
    // Intercepts KDevelop core signals and VCS notifications (if available)
    connect( m_part->project(), SIGNAL( activeDirectoryChanged( const QString&, const QString& ) ),
             this, SLOT( changeActiveDirectory( const QString&, const QString& ) ) );
    connect( m_part->project(), SIGNAL( addedFilesToProject( const QStringList & ) ),
             this, SLOT( addProjectFiles( const QStringList & ) ) );
    connect( m_part->project(), SIGNAL( removedFilesFromProject( const QStringList & ) ),
             this, SLOT( removeProjectFiles( const QStringList & ) ) );
    // Safeguard against VCS plug-in unloading at run-time
    connect( m_impl, SIGNAL(implementationInvalidated()), this, SLOT(slotImplementationInvalidated()) );

    // Hide pattern for files
    QDomDocument &dom = *m_part->projectDom();
    QString defaultHidePattern = "*.o,*.lo,CVS";
    QString hidePattern = DomUtil::readEntry( dom, "/kdevfileview/tree/hidepatterns", defaultHidePattern );
    m_hidePatterns = QStringList::split( ",", hidePattern );
}

///////////////////////////////////////////////////////////////////////////////

FileTreeWidget::~FileTreeWidget()
{
    kdDebug(9017) << "FileTreeWidget::~FileTreeWidget()" << endl;

    QDomDocument &dom = *m_part->projectDom();
    DomUtil::writeEntry( dom, "/kdevfileview/tree/hidepatterns", hidePatterns() );

//    delete m_impl;
}

///////////////////////////////////////////////////////////////////////////////

void FileTreeWidget::openDirectory( const QString& dirName )
{
    kdDebug(9017) << "FileTreeWidget::openDirectory(): " + dirName << endl;

    // if we're reloading
    if (m_rootBranch)
    {
        disconnect( m_rootBranch, SIGNAL(populateFinished(KFileTreeViewItem*)), this, SLOT(finishPopulate(KFileTreeViewItem*)) );
        removeBranch( m_rootBranch );
        m_projectFiles.clear();
    }

    addProjectFiles( m_part->project()->allFiles(), true );

    KURL url = KURL::fromPathOrURL( dirName );
    const QPixmap& pix = KMimeType::mimeType("inode/directory")->pixmap( KIcon::Small );

    // this is a bit odd, but the order of these calls seems to be important
    //FileTreeBranch *b = new FileTreeBranch( this, url, url.prettyURL(), pix );
    FileTreeBranchItem *b = m_impl->branchItemFactory()->makeBranchItem( this, url, url.prettyURL(), pix );
    b->setChildRecurse( false );
    m_rootBranch = addBranch( b );
    m_rootBranch->setOpen( true );
    connect( m_rootBranch, SIGNAL(populateFinished(KFileTreeViewItem*)), this, SLOT(finishPopulate(KFileTreeViewItem*)) );
}

///////////////////////////////////////////////////////////////////////////////

bool FileTreeWidget::shouldBeShown( KFileTreeViewItem* item )
{
    FileTreeViewItem * i = static_cast<FileTreeViewItem *>( item );
    return ( i->isDir() || ( (m_impl->showNonProjectFiles() || i->isProjectFile() )
             && !matchesHidePattern( i->url().fileName() ) ) ) ;
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
    FileTreeViewItem* item = static_cast<FileTreeViewItem*>(firstChild());
    if( !item )
      return;

    // Need to skip the root item (which is the sub-directory)
    // i.e. "/home/devmario/src/kdevelop/parts/cvsservice"
    item = static_cast<FileTreeViewItem*>( item->firstChild() );
    // Now fill the sub-tree
    while (item)
    {
        item->hideOrShow();
        item = static_cast<FileTreeViewItem*>(item->nextSibling());
    }
}

void FileTreeWidget::finishPopulate(KFileTreeViewItem* item)
{
    if( item == firstChild() )
    {
        changeActiveDirectory( "", m_part->project()->activeDirectory() );
    }
}

///////////////////////////////////////////////////////////////////////////////

void FileTreeWidget::slotItemExecuted( QListViewItem* item )
{
    if (!item)
        return;

    KFileTreeViewItem* ftitem = static_cast<KFileTreeViewItem*>(item);

    if (ftitem->isDir())
        return;

    m_part->partController()->editDocument( ftitem->url() );
}

///////////////////////////////////////////////////////////////////////////////

void FileTreeWidget::slotContextMenu( KListView *, QListViewItem* item, const QPoint &p )
{
    kdDebug(9017) << "FileTreeWidget::slotContextMenu(...)" << endl;

    KPopupMenu popup( i18n("File Tree"), this );

    // If an item is selected, fill the file context with selected files' list
    if (item)
    {
        m_impl->fillPopupMenu( &popup, item );
        FileContext context( m_impl->selectedPathUrls() );
        m_part->core()->fillContextMenu( &popup, &context );
    }


    popup.exec( p );
}

///////////////////////////////////////////////////////////////////////////////

QString FileTreeWidget::projectDirectory()
{
    return m_part->project()->projectDirectory();
}

///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Test whether given file (or a directory) is part of this project.
 *
 * @param fileName or directory to test for presence.
 */
bool FileTreeWidget::isInProject(const QString &fileName) const
{
    return m_projectFiles.contains(fileName);
}

///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Add a bunch of files to this project.
 *
 * Whenever we load a project or user chooses to add a bunch of files using UI,
 * we end in this method.
 * We merge the list of files already in the project (if any) with the incoming set.
 *
 * @param fileList
 * @param constructing
 *
 * @see m_projectFiles
 */
void FileTreeWidget::addProjectFiles( QStringList const & fileList, bool constructing )
{
    kdDebug(9017) << "files added to project: " << fileList << endl;

    QStringList::ConstIterator it;
    for ( it = fileList.begin(); it != fileList.end(); ++it )
    {
        if( (*it).isEmpty() )
            continue;
        kdDebug(9017) << "adding file: " << *it << endl;
        const QString file = projectDirectory() + "/" + ( *it );
        if ( !m_projectFiles.contains( file ) )
        {
            // We got a new file to add to this project.
            // Ensure all the parent directories are part of the project set, too.
            QStringList paths = QStringList::split( "/", *it);
            paths.pop_back();
            while( !paths.isEmpty() )
            {
                // We are adding the directories from longest (the one containing our file),
                // to the shortest, measured from root directory of our project.
                // Whenever we find out that a directory is already recorded as part of our project,
                // we may stop adding, because its parent directories were already added -
                // in some previous addition.
                QString joinedPaths = paths.join("/");
                if( m_projectFiles.contains( joinedPaths ) )
                    break;
                m_projectFiles.insert( projectDirectory() + "/" + joinedPaths, true );
                paths.pop_back();
            }
            m_projectFiles.insert( file, false );
//            kdDebug(9017) << "file added: " << file << endl;
        }

        if ( !constructing )
        {
            FileTreeViewItem* item = static_cast<FileTreeViewItem*>(firstChild());
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

        FileTreeViewItem* item = static_cast<FileTreeViewItem*>(firstChild());
        if( item )
        {
            item->setProjectFile( file, false );
        }
    }
}

void FileTreeWidget::changeActiveDirectory( const QString& olddir, const QString& newdir )
{
    FileTreeViewItem* item = static_cast<FileTreeViewItem*>(firstChild());
    if( item )
    {
      item->changeActiveDir( projectDirectory() + "/" + olddir, projectDirectory() + "/" + newdir );
    }
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

KDevVersionControl *FileTreeWidget::versionControl() const
{
    if (part() && part()->versionControl())
        return part()->versionControl();
    else
        return 0;
}

///////////////////////////////////////////////////////////////////////////////

bool FileTreeWidget::showNonProjectFiles() const
{
    return m_impl->showNonProjectFiles();
}

///////////////////////////////////////////////////////////////////////////////

void FileTreeWidget::slotImplementationInvalidated()
{
    kdDebug(9017) << "FileTreeWidget::slotImplementationInvalidated()" << endl;

    // Destroy old implementation, create the simpler default impl. and
    // reload list view
    // remove old branch
    removeBranch( m_rootBranch );
    m_rootBranch = 0; // avoid openDirectory() trying to release the branch

    // Restore a clean situation for an eventual new & different implementation
    /** \FIXME this for-loop should really go in ~FileTreeViewWidgetImpl() but
      * it crashes there: here it works :-/
      */
    for (int i=columns()-1; i>=0; --i)
    {
        kdDebug(9017) << "Removing column: " << i << endl;
        removeColumn( i );
    }

    delete m_impl;
    m_impl = new StdFileTreeWidgetImpl( this );
    openDirectory( projectDirectory() );
}

#include "filetreewidget.moc"

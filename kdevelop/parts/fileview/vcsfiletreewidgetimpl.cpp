/***************************************************************************
 *   Copyright (C) 2003 by Mario Scalas                                    *
 *   mario.scalas@libero.it                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qpainter.h>
#include <qheader.h>
#include <qpopupmenu.h>
#include <kaction.h>
#include <klocale.h>
#include <kdebug.h>
#include <kdeversion.h>

//#include <kdevproject.h>
#include <urlutil.h>
#include <domutil.h>

#include "fileviewpart.h"
#include "filetreewidget.h"
#include "fileitemfactory.h"
#include "vcsfiletreewidgetimpl.h"
#include "vcscolorsconfigwidget.h"

using namespace filetreeview;

///////////////////////////////////////////////////////////////////////////////
// class VCSFileTreeViewItem
///////////////////////////////////////////////////////////////////////////////

#define FILENAME_COLUMN     0
#define STATUS_COLUMN       1
#define WORKREVISION_COLUMN 2
#define REPOREVISION_COLUMN 3

class VCSFileTreeViewItem : public filetreeview::FileTreeViewItem
{
public:
    VCSFileTreeViewItem( KFileTreeViewItem* parent, KFileItem* item, KFileTreeBranch* branch, bool pf )
        : FileTreeViewItem( parent, item, branch, pf ), m_statusColor( &FileViewPart::vcsColors.unknown ) {}
    VCSFileTreeViewItem( KFileTreeView* parent, KFileItem* item, KFileTreeBranch* branch )
        : FileTreeViewItem( parent, item, branch ), m_statusColor( &FileViewPart::vcsColors.unknown ) {}
    virtual void paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int alignment )
    {
        // paint cell in a different color depending on VCS state
        QColorGroup mycg( cg );
        mycg.setColor( QColorGroup::Base, *m_statusColor );
        FileTreeViewItem::paintCell( p, mycg, column, width, alignment );
    }
    void setVCSInfo( const VCSFileInfo &info );

    QString fileName() const { return text( FILENAME_COLUMN ); }
    QString workingRev() const { return text( WORKREVISION_COLUMN ); }
    QString repositoryRev() const { return text( REPOREVISION_COLUMN ); }
    QString status() const { return text( STATUS_COLUMN ); }
    void setFileName( const QString &p ) { setText( FILENAME_COLUMN, p ); }
    void setWorkingRev( const QString &p ) { setText( WORKREVISION_COLUMN, p ); }
    void setRepositoryRev( const QString &p ) { setText( REPOREVISION_COLUMN, p ); }
    void setStatus( const VCSFileInfo::FileState status );

private:
    QColor *m_statusColor; // cached
};

///////////////////////////////////////////////////////////////////////////////

void VCSFileTreeViewItem::setVCSInfo( const VCSFileInfo &info )
{
    //setFileName( info.fileName );
    setRepositoryRev( info.repoRevision );
    setWorkingRev( info.workRevision );
    setStatus( info.state );
}

///////////////////////////////////////////////////////////////////////////////

void VCSFileTreeViewItem::setStatus( const VCSFileInfo::FileState status )
{
    setText( STATUS_COLUMN, VCSFileInfo::state2String( status ) );
    // Update color too
    switch (status)
    {
        case VCSFileInfo::Added:
            m_statusColor = &FileViewPart::vcsColors.added;
            break;
        case VCSFileInfo::Uptodate:
            m_statusColor = &FileViewPart::vcsColors.updated;
            break;
        case VCSFileInfo::Modified:
            m_statusColor = &FileViewPart::vcsColors.modified;
            break;
        case VCSFileInfo::Conflict:
            m_statusColor = &FileViewPart::vcsColors.conflict;
            break;
        case VCSFileInfo::Sticky:
            m_statusColor = &FileViewPart::vcsColors.sticky;
            break;
        case VCSFileInfo::NeedsCheckout:
            m_statusColor = &FileViewPart::vcsColors.needsCheckout;
            break;
        case VCSFileInfo::NeedsPatch:
            m_statusColor = &FileViewPart::vcsColors.needsPatch;
            break;
        case VCSFileInfo::Unknown:
            m_statusColor = &FileViewPart::vcsColors.unknown;
            break;
        case VCSFileInfo::Directory:
        default:
            // No color change
            m_statusColor = &FileViewPart::vcsColors.defaultColor;
            //kdDebug() << "MyFileTreeViewItem::paintCell(): Unknown color!" << endl;
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////
// class VCSFileTreeBranchItem
///////////////////////////////////////////////////////////////////////////////

class VCSFileTreeBranchItem : public filetreeview::FileTreeBranchItem
{
public:
    VCSFileTreeBranchItem( KFileTreeView* view, const KURL& url, const QString& name, const QPixmap& pix,
        KDevVCSFileInfoProvider *vcsInfoProvider )
        : FileTreeBranchItem( view, url, name, pix ), m_vcsInfoProvider( vcsInfoProvider ) {}
//    virtual ~VCSFileTreeBranchItem() {}
    virtual KFileTreeViewItem* createTreeViewItem( KFileTreeViewItem* parent, KFileItem* fileItem );

private:
    KDevVCSFileInfoProvider *m_vcsInfoProvider;
};

///////////////////////////////////////////////////////////////////////////////

KFileTreeViewItem* VCSFileTreeBranchItem::createTreeViewItem( KFileTreeViewItem* parent, KFileItem* fileItem )
{
//  kdDebug(9017) << "MyFileTreeBranch::createTreeViewItem(): " + fileItem->url().path() << endl;
    if (!parent || !fileItem)
        return 0;

    FileTreeWidget *lv = static_cast<filetreeview::FileTreeViewItem*>( parent )->listView();
    const KURL fileURL = fileItem->url();
    bool isDirectory = lv->projectFiles().contains( fileURL.path() ) > 0;

    VCSFileTreeViewItem *newItem = new VCSFileTreeViewItem( parent, fileItem, this, isDirectory );

    QString fileName = fileURL.fileName();
    QString dirName = URLUtil::extractPathNameRelative( lv->projectDirectory(), fileURL.directory() );

    const VCSFileInfoMap &vcsFiles = *m_vcsInfoProvider->status( dirName );
//  kdDebug(9017) << "Dir has " << vcsFiles.count() << " registered files!" << endl;
    if (vcsFiles.contains( fileName ))
        newItem->setVCSInfo( vcsFiles[ fileName ] );
//            else
//                kdDebug(9017) << "!!!No VCS info for this file!!!" << endl;
    return newItem;
}

///////////////////////////////////////////////////////////////////////////////
// class VCSFileItemFactory
///////////////////////////////////////////////////////////////////////////////

class VCSBranchItemFactory : public filetreeview::BranchItemFactory
{
public:
    VCSBranchItemFactory( KDevVCSFileInfoProvider *vcsInfoProvider ) : m_vcsInfoProvider( vcsInfoProvider ) {}
    virtual filetreeview::FileTreeBranchItem *makeBranchItem( KFileTreeView* view, const KURL& url, const QString& name, const QPixmap& pix )
    {
        return new VCSFileTreeBranchItem( view, url, name, pix, m_vcsInfoProvider );
    }
private:
    KDevVCSFileInfoProvider *m_vcsInfoProvider;
};

///////////////////////////////////////////////////////////////////////////////
// class VCSFileTreeWidgetImpl
///////////////////////////////////////////////////////////////////////////////

VCSFileTreeWidgetImpl::VCSFileTreeWidgetImpl( FileTreeWidget *parent, KDevVCSFileInfoProvider *infoProvider )
    : FileTreeViewWidgetImpl( parent, "vcsfiletreewidgetimpl" ),
    m_actionToggleShowVCSFields( 0 ), m_actionSyncWithRepository( 0 ), m_vcsInfoProvider( infoProvider ),
    m_isSyncingWithRepository( false ), m_vcsStatusRequestedItem( 0 )
{
    kdDebug(9017) << "VCSFileTreeWidgetImpl::VCSFileTreeWidgetImpl()" << endl;

    Q_ASSERT( m_vcsInfoProvider );
    setBranchItemFactory( new VCSBranchItemFactory( m_vcsInfoProvider ) );

    // Update the #define order on top if you change this order!
    parent->addColumn( "Filename" );
    parent->addColumn( "Status" );
    parent->addColumn( "Work" );
    parent->addColumn( "Repo" );

    connect( m_vcsInfoProvider, SIGNAL(statusReady(const VCSFileInfoMap&, void *)),
        this, SLOT(vcsDirStatusReady(const VCSFileInfoMap&, void*)) );
    // Harakiri itself if the infoProvider object is destroyed since we cannot work anymore :-(
    connect( m_vcsInfoProvider, SIGNAL(destroyed()), SIGNAL(implementationInvalidated()) );

    m_actionToggleShowVCSFields = new KToggleAction( i18n("Show VCS Fields"), KShortcut(),
        this, "actiontoggleshowvcsfieldstoggleaction" );
#if KDE_IS_VERSION(3,2,90)
    m_actionToggleShowVCSFields->setCheckedState(i18n("Hide VCS Fields"));
#endif
    QString aboutAction = i18n("<b>Show VCS fields</b><p>Shows <b>Revision</b> and <b>Timestamp</b> for each file contained in VCS repository.");
    m_actionToggleShowVCSFields->setWhatsThis( aboutAction );
    connect( m_actionToggleShowVCSFields, SIGNAL(toggled(bool)), this, SLOT(slotToggleShowVCSFields(bool)) );

    m_actionSyncWithRepository = new KAction( i18n( "Sync with Repository"), KShortcut(),
        this, SLOT(slotSyncWithRepository()), this, "actionsyncwithrepository" );
    aboutAction = i18n("<b>Sync with repository</b><p>Synchronize file status with remote repository.");
    m_actionSyncWithRepository->setWhatsThis( aboutAction );

    QDomDocument &dom = projectDom();
    m_actionToggleShowVCSFields->setChecked( DomUtil::readBoolEntry(dom, "/kdevfileview/tree/showvcsfields") );
    slotToggleShowVCSFields( showVCSFields() ); // show or hide fields depending on read settings
}

///////////////////////////////////////////////////////////////////////////////

VCSFileTreeWidgetImpl::~VCSFileTreeWidgetImpl()
{
    kdDebug(9017) << "VCSFileTreeWidgetImpl::~VCSFileTreeWidgetImpl()" << endl;

    QDomDocument &dom = projectDom();
    DomUtil::writeBoolEntry( dom, "/kdevfileview/tree/showvcsfields", showVCSFields() );
}

///////////////////////////////////////////////////////////////////////////////

bool VCSFileTreeWidgetImpl::canReloadTree() const
{
    return !m_isSyncingWithRepository;
}

///////////////////////////////////////////////////////////////////////////////

void VCSFileTreeWidgetImpl::fillPopupMenu( QPopupMenu *popupMenu, QListViewItem *item ) const
{
    FileTreeViewWidgetImpl::fillPopupMenu( popupMenu, item );

    // Submenu for visualization options
    m_actionToggleShowVCSFields->plug( popupMenu );
    // Give a change for syncing status with remote repository: a file info provider must
    // be available and the item must be a project directory
    FileTreeViewItem *fileItem = static_cast<FileTreeViewItem *>( item );
    if (fileItem->isDir())
    {
        m_vcsStatusRequestedItem = fileItem;
        popupMenu->insertSeparator();
        m_actionSyncWithRepository->plug( popupMenu );
    }
}

///////////////////////////////////////////////////////////////////////////////

void VCSFileTreeWidgetImpl::slotToggleShowVCSFields( bool checked )
{
    kdDebug(9017) << "VCSFileTreeWidgetImpl::slotToggleShowVCSFields()" << endl;

    if (checked)
    {
        setColumnWidth( 0, contentsWidth() / 2 ); // "Filename"
        setColumnWidth( 1, contentsWidth() / 4 ); // "status"
        setColumnWidth( 2, contentsWidth() / 5 ); // "work revision"
        setColumnWidth( 3, contentsWidth() / 5 ); // "repository revision"
        header()->show();
    }
    else
    {
        setColumnWidth( 3 ,0 ); // Hide columns
        setColumnWidth( 2 ,0 );
        setColumnWidth( 1, 0 );
        setColumnWidth( 0, contentsWidth() ); // Make the "Filename" column to occupy all the row
        header()->hide();
    }

    triggerUpdate();
}

///////////////////////////////////////////////////////////////////////////////

bool VCSFileTreeWidgetImpl::showVCSFields() const
{
    return m_actionToggleShowVCSFields->isChecked();
}

///////////////////////////////////////////////////////////////////////////////

void VCSFileTreeWidgetImpl::vcsDirStatusReady( const VCSFileInfoMap &modifiedFiles, void *callerData )
{
    kdDebug(9017) << "VCSFileTreeWidgetImpl::vcsDirStatusReady(const VCSFileInfoMap &, void*)" << endl;

    VCSFileTreeViewItem *item = static_cast<VCSFileTreeViewItem*>( callerData );
    Q_ASSERT( item ); // this must _not_ fail!
    // Update vcs file info for all childs in this tree folder ...
    item = static_cast<VCSFileTreeViewItem*>( item->firstChild() );
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
            kdDebug(9017) << "Map does not contain anything useful about this file ;-( " << fileName << endl;
        item = static_cast<VCSFileTreeViewItem*>( item->nextSibling() );
    }
    triggerUpdate();

    m_isSyncingWithRepository = false;
}

///////////////////////////////////////////////////////////////////////////////

void VCSFileTreeWidgetImpl::slotSyncWithRepository()
{
    kdDebug(9017) << "VCSFileTreeWidgetImpl::slotSyncWithRepository()" << endl;
    const VCSFileTreeViewItem *myFileItem = static_cast<VCSFileTreeViewItem *>( m_vcsStatusRequestedItem );
    const QString relDirPath = URLUtil::extractPathNameRelative( projectDirectory(), myFileItem->fileItem()->url().path() );
    kdDebug(9017) << "VCS Info requested for: " << relDirPath << endl;
    m_vcsInfoProvider->requestStatus( relDirPath, m_vcsStatusRequestedItem );
    m_isSyncingWithRepository = true;
}

#include "vcsfiletreewidgetimpl.moc"

/**************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2003 by Mario Scalas                                    *
 *   mario.scalas@libero.it                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qdir.h>
#include <qpopupmenu.h>

#include <kpopupmenu.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdialogbase.h>
#include <kstandarddirs.h>
#include <kaction.h>
#include <kurl.h>
#include <kapplication.h>
#include <kmainwindow.h>
// Because of KShellProcess::quote()
#include <kprocess.h>
#include <kiconloader.h>

#include <dcopref.h>
#include <repository_stub.h>
#include <cvsservice_stub.h>
#include <cvsjob_stub.h>

#include <kparts/part.h>
#include <kdevpartcontroller.h>
#include <kgenericfactory.h>

#include "kdevcore.h"
#include "kdevmakefrontend.h"
#include "kdevdifffrontend.h"
#include "kdevappfrontend.h"
#include "domutil.h"
#include "kdevmainwindow.h"
#include "kdevproject.h"
#include "urlutil.h"

#include "cvsform.h"
#include "commitdlg.h"
#include "checkoutdialog.h"
#include "tagdialog.h"
#include "cvsprocesswidget.h"
#include "cvsoptions.h"
#include "cvsoptionswidget.h"
#include "cvspartimpl.h"
#include "cvsdir.h"

#include "cvspart.h"

QStringList prependToStringList( const QString &s, const QStringList &paths )
{
    QStringList l = paths;
    for (size_t i=0; i<l.count(); ++i)
    {
        l[i] = s + QDir::separator() + l[i];
    }
    return l;
}

///////////////////////////////////////////////////////////////////////////////
// Global vars
///////////////////////////////////////////////////////////////////////////////

// See createNewProject( const QString &) and slotProjectOpened()
bool g_projectWasJustCreated = false;

///////////////////////////////////////////////////////////////////////////////
// Plugin factory
///////////////////////////////////////////////////////////////////////////////

typedef KGenericFactory<CvsServicePart> CvsFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevcvsservice, CvsFactory( "kdevcvsservice" ) )

///////////////////////////////////////////////////////////////////////////////
// class CvsServicePart
///////////////////////////////////////////////////////////////////////////////

CvsServicePart::CvsServicePart( QObject *parent, const char *name, const QStringList & )
    : KDevVersionControl( "KDevCvsServicePart", "kdevcvsservicepart", parent,
        name ? name : "CvsService" ),
    actionCommit( 0 ), actionDiff( 0 ), actionLog( 0 ), actionAdd( 0 ),
    actionAddBinary( 0 ), actionRemove( 0 ), actionUpdate( 0 ),
    actionRemoveSticky( 0 ),
    actionAddToIgnoreList( 0 ), actionRemoveFromIgnoreList( 0 ),
    actionTag( 0 ), actionUnTag( 0 ),
    actionLogin( 0), actionLogout( 0 ),
    m_impl( 0 )
{
    setInstance( CvsFactory::instance() );

    setXMLFile( "kdevcvsservicepart.rc" );

    init();

    // @fixme (at all costs!), Ok, this is a crime but for now CvsServicePart is the only implementation
    // of KDevVersionControl
    setVersionControl( this );
}

///////////////////////////////////////////////////////////////////////////////

CvsServicePart::~CvsServicePart()
{
    delete m_cvsConfigurationForm;
    delete m_impl;
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePart::init()
{
    setupActions();

    // Here we change the implementation
    // @fixme factory pattern here? :-/
    m_impl = new CvsServicePartImpl( this );
    connect( m_impl, SIGNAL(checkoutFinished(QString)), SIGNAL(finishedFetching(QString)) );
    connect( m_impl, SIGNAL(fileStateChanged(const VCSFileInfoList&)), SIGNAL(fileStateChanged(const VCSFileInfoList&)) );

    // Load / store project configuration every time project is opened/closed
    connect( core(), SIGNAL(projectOpened()), this, SLOT(slotProjectOpened()) );
    connect( core(), SIGNAL(projectClosed()), this, SLOT(slotProjectClosed()) );

    // Context menu
    connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
        this, SLOT(contextMenu(QPopupMenu *, const Context *)) );
    connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
        this, SLOT(projectConfigWidget(KDialogBase*)) );
    connect( core(), SIGNAL(stopButtonClicked(KDevPlugin*)),
        this, SLOT(slotStopButtonClicked(KDevPlugin*)) );

    m_impl->processWidget()->setIcon( SmallIcon("db") );

    mainWindow()->embedOutputView( m_impl->processWidget(), i18n("CvsService"), i18n("cvs output") );
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePart::setupActions()
{
    // This actions are used in the menubar: for context menu we build the
    // context at runtime. See CvsServicePart::contextMenu().

    actionCommit = new KAction( i18n("&Commit to repository"), 0, this,
        SLOT(slotActionCommit()), actionCollection(), "cvsservice_commit" );
    actionCommit->setToolTip( i18n("Commit file(s)") );
    actionCommit->setWhatsThis( i18n("Commit file(s)<p>Commit file to repository if modified") );

    actionDiff = new KAction( i18n("&Difference between revisions"), 0, this, SLOT(slotActionDiff()),
        actionCollection(), "cvsservice_diff" );
    actionDiff->setToolTip( i18n("Build difference") );
    actionDiff->setWhatsThis( i18n("Build difference<p>Build difference between releases") );

    actionLog = new KAction( i18n("Generate &log"), 0, this, SLOT(slotActionLog()),
        actionCollection(), "cvsservice_log" );
    actionLog->setToolTip( i18n("Build log") );
    actionLog->setWhatsThis( i18n("Build log<p>Produce log for this file") );

    actionAdd = new KAction( i18n("&Add to repository"), 0, this, SLOT(slotActionAdd()),
        actionCollection(), "cvsservice_add" );
    actionAdd->setToolTip( i18n("Add file to repository") );
    actionAdd->setWhatsThis( i18n("Add file <p>Add file to repository") );

    actionAddBinary = new KAction( i18n("Add to repository as &binary"), 0, this,
        SLOT(slotActionAddBinary()), actionCollection(), "cvsservice_add_bin" );
    actionAddBinary->setToolTip( i18n("Add file to repository") );
    actionAddBinary->setWhatsThis( i18n("Add file as binary<p>Add file to repository as binary (-kb option)") );

    actionRemove = new KAction( i18n("&Remove from repository"), 0, this,
        SLOT(slotActionRemove()), actionCollection(), "cvsservice_remove" );
    actionRemove->setToolTip( i18n("Remove from repository") );
    actionRemove->setWhatsThis( i18n("Remove from repository<p>Remove file(s) from repository") );

    actionUpdate = new KAction( i18n("&Update/revert to another release"), 0, this,
        SLOT(slotActionUpdate()), actionCollection(), "cvsservice_update" );
    actionUpdate->setToolTip( i18n("Update/revert") );
    actionUpdate->setWhatsThis( i18n("Update/revert<p>Update/revert file(s) to another release") );

    actionRemoveSticky = new KAction( i18n("R&emove sticky flag"), 0,
        this, SLOT(slotActionRemoveSticky()), actionCollection(), "cvsservice_removesticky" );
    actionRemoveSticky->setToolTip( i18n("Remove sticky flag") );
    actionRemoveSticky->setWhatsThis( i18n("Remove sticky flag<p>Remove sticky flag from file(s)") );

    actionTag = new KAction( i18n("Make &tag/branch"), 0,
        this, SLOT(slotActionTag()), actionCollection(), "cvsservice_tag" );
    actionTag->setToolTip( i18n("Make tag/branch") );
    actionTag->setWhatsThis( i18n("Make tag/branch<p>Tags/branches selected file(s)") );

    actionUnTag = new KAction( i18n("&Delete tag/branch"), 0,
        this, SLOT(slotActionUnTag()), actionCollection(), "cvsservice_untag" );
    actionUnTag->setToolTip( i18n("Delete tag/branch") );
    actionUnTag->setWhatsThis( i18n("Delete tag/branch<p>Delete tag/branches from selected file(s)") );

    actionAddToIgnoreList = new KAction( i18n("&Ignore in CVS operations"), 0,
        this, SLOT(slotActionAddToIgnoreList()), actionCollection(), "cvsservice_ignore" );
    actionAddToIgnoreList->setToolTip( i18n("Ignore in CVS operations") );
    actionAddToIgnoreList->setWhatsThis( i18n("Ignore in CVS operations<p>Ignore file(s) by adding it to .cvsignore file") );

    actionRemoveFromIgnoreList = new KAction( i18n("Do &not ignore in CVS operations"), 0,
        this, SLOT(slotActionRemoveFromIgnoreList()), actionCollection(), "cvsservice_donot_ignore" );
    actionRemoveFromIgnoreList->setToolTip( i18n("Do not ignore in CVS operations") );
    actionRemoveFromIgnoreList->setWhatsThis( i18n("Do not ignore in CVS operations<p>Do not ignore file(s) by removing\nit from .cvsignore file") );

    actionLogin = new KAction( i18n("&Log to server"), 0, this,
        SLOT(slotActionLogin()), actionCollection(), "cvsservice_login" );
    actionLogin->setToolTip( i18n("Login to server") );
    actionLogin->setWhatsThis( i18n("Login to server<p>Log-in to remote CVS server") );

    actionLogout = new KAction( i18n("L&ogout from server"), 0, this,
        SLOT(slotActionLogout()), actionCollection(), "cvsservice_logout" );
    actionLogout->setToolTip( i18n("Logout from server") );
    actionLogout->setWhatsThis( i18n("Logout from server<p>Log-out from remote CVS server") );
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePart::fetchFromRepository()
{
    m_impl->checkout();
}

///////////////////////////////////////////////////////////////////////////////

KDevVCSFileInfoProvider *CvsServicePart::fileInfoProvider() const
{
    return m_impl->fileInfoProvider();
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePart::createNewProject( const QString &dirName )
{
    kdDebug( 9006 ) << "====> CvsServicePart::createNewProject( const QString& )" << endl;

    if (!m_cvsConfigurationForm)
        return;

    // @fixme: actually there is no way to inform that a _new_ ("just created")
    // project has been opened because projectOpened() is emitted after the project
    // has been created :-/ So the only way to inform that slotProjectOpened() to not
    // load default settings (overriding the CvsOptions instance is to set this flag
    // here ...
    g_projectWasJustCreated = true;

    m_impl->createNewProject( dirName,
        m_cvsConfigurationForm->cvsRsh(), m_cvsConfigurationForm->location(),
        m_cvsConfigurationForm->message(), m_cvsConfigurationForm->module(),
        m_cvsConfigurationForm->vendor(), m_cvsConfigurationForm->release(),
        m_cvsConfigurationForm->mustInitRoot()
    );
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePart::projectConfigWidget( KDialogBase *dlg )
{
    QVBox *vbox = dlg->addVBoxPage( i18n("CvsService") );
    CvsOptionsWidget *w = new CvsOptionsWidget( (QWidget *)vbox, "cvs config widget" );
    connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
}

///////////////////////////////////////////////////////////////////////////////

QWidget* CvsServicePart::newProjectWidget( QWidget *parent )
{
    m_cvsConfigurationForm = new CvsForm( parent, "cvsform" );
    return m_cvsConfigurationForm;
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePart::contextMenu( QPopupMenu *popup, const Context *context )
{
    kdDebug(9006) << "contextMenu()" << endl;
    if (context->hasType( Context::FileContext ) ||
        context->hasType( Context::EditorContext ))
    {

        if (context->hasType( Context::FileContext ))
        {
            kdDebug(9006) << "Requested for a FileContext" << endl;
            const FileContext *fcontext = static_cast<const FileContext*>( context );
            m_urls = fcontext->urls();
        }
        else
        {
            kdDebug(9006) << "Requested for an EditorContext" << endl;
            const EditorContext *editorContext = static_cast<const EditorContext*>( context );
            m_urls << editorContext->url();
        }
        // THis stuff should end up into prepareOperation()
        URLUtil::dump( m_urls );
        if (m_urls.count() <= 0)
                return;

        KPopupMenu *subMenu = new KPopupMenu( popup );
        popup->insertSeparator();

        subMenu->insertItem( actionCommit->text(), this, SLOT(slotCommit()) );
        // CvsService let to do log and diff operations only on one file (or directory) at time
        if (m_urls.count() == 1)
        {
            subMenu->insertItem( actionDiff->text(), this, SLOT(slotDiff()) );
            subMenu->insertItem( actionLog->text(), this, SLOT(slotLog()) );
        }
        subMenu->insertItem( actionAdd->text(), this, SLOT(slotAdd()) );
        subMenu->insertItem( actionAddBinary->text(), this, SLOT(slotAddBinary()) );
        subMenu->insertItem( actionRemove->text(), this, SLOT(slotRemove()) );

        subMenu->insertSeparator();
        subMenu->insertItem( actionTag->text(), this, SLOT(slotTag()) );
        subMenu->insertItem( actionUnTag->text(), this, SLOT(slotUnTag()) );
        subMenu->insertItem( actionUpdate->text(), this, SLOT(slotUpdate()) );
        subMenu->insertItem( actionRemoveSticky->text(), this, SLOT(slotRemoveSticky()) );

        subMenu->insertSeparator();
        subMenu->insertItem( actionAddToIgnoreList->text(), this, SLOT(slotAddToIgnoreList()) );
        subMenu->insertItem( actionRemoveFromIgnoreList->text(), this, SLOT(slotRemoveFromIgnoreList()) );

        // Now insert in parent menu
        popup->insertItem( i18n("CvsService"), subMenu );
    }
}

///////////////////////////////////////////////////////////////////////////////

bool CvsServicePart::urlFocusedDocument( KURL &url )
{
    kdDebug(9006) << "CvsServicePartImpl::retrieveUrlFocusedDocument() here!" << endl;
    KParts::ReadOnlyPart *part = dynamic_cast<KParts::ReadOnlyPart*>( partController()->activePart() );
    if ( part )
    {
        if (part->url().isLocalFile() )
        {
            url = part->url();
            return true;
        }
        else
        {
            kdDebug(9006) << "Cannot handle non-local files!" << endl;
        }
    }
   return false;
}

///////////////////////////////////////////////////////////////////////////////

bool CvsServicePart::isValidDirectory( const QDir &dir ) const
{
    return m_impl->isValidDirectory( dir );
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePart::slotActionLogin()
{
    m_impl->login();
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePart::slotActionLogout()
{
    m_impl->logout();
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePart::slotActionCommit()
{
    KURL currDocument;
    if (urlFocusedDocument( currDocument ))
    {
        m_impl->commit( currDocument );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePart::slotActionUpdate()
{
    KURL currDocument;
    if (urlFocusedDocument( currDocument ))
    {
        m_impl->update( currDocument );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePart::slotActionAdd()
{
    KURL currDocument;
    if (urlFocusedDocument( currDocument ))
    {
        m_impl->add( currDocument, false );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePart::slotActionAddBinary()
{
    KURL currDocument;
    if (urlFocusedDocument( currDocument ))
    {
        m_impl->add( currDocument, true );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePart::slotActionRemove()
{
    KURL currDocument;
    if (urlFocusedDocument( currDocument ))
    {
        m_impl->remove( currDocument );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePart::slotActionRemoveSticky()
{
    KURL currDocument;
    if (urlFocusedDocument( currDocument ))
    {
        m_impl->removeStickyFlag( currDocument );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePart::slotActionLog()
{
    KURL currDocument;
    if (urlFocusedDocument( currDocument ))
    {
        m_impl->log( currDocument );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePart::slotActionDiff()
{
    KURL currDocument;
    if (urlFocusedDocument( currDocument ))
    {
        m_impl->diff( currDocument );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePart::slotActionTag()
{
    KURL currDocument;
    if (urlFocusedDocument( currDocument ))
    {
        m_impl->tag( currDocument );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePart::slotActionUnTag()
{
    KURL currDocument;
    if (urlFocusedDocument( currDocument ))
    {
        m_impl->unTag( currDocument );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePart::slotActionAddToIgnoreList()
{
    KURL currDocument;
    if (urlFocusedDocument( currDocument ))
    {
        m_impl->addToIgnoreList( currDocument );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePart::slotActionRemoveFromIgnoreList()
{
    KURL currDocument;
    if (urlFocusedDocument( currDocument ))
    {
        m_impl->removeFromIgnoreList( currDocument );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePart::slotCommit()
{
    m_impl->commit( m_urls );
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePart::slotUpdate()
{
    m_impl->update( m_urls );
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePart::slotAdd()
{
    m_impl->add( m_urls, false );
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePart::slotAddBinary()
{
    m_impl->add( m_urls, true );
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePart::slotRemove()
{
    m_impl->remove( m_urls );
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePart::slotRemoveSticky()
{
    m_impl->removeStickyFlag( m_urls );
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePart::slotLog()
{
    m_impl->log( m_urls );
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePart::slotDiff()
{
    m_impl->diff( m_urls );
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePart::slotTag()
{
    m_impl->tag( m_urls );
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePart::slotUnTag()
{
    m_impl->unTag( m_urls );
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePart::slotAddToIgnoreList()
{
    m_impl->addToIgnoreList( m_urls );
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePart::slotRemoveFromIgnoreList()
{
    m_impl->removeFromIgnoreList( m_urls );
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePart::slotStopButtonClicked( KDevPlugin* which )
{
    if ( which != 0 && which != this )
        return;

    m_impl->flushJobs();
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePart::slotAddFilesToProject( const QStringList &filesToAdd )
{
    kdDebug( 9006 ) << "====> CvsServicePart::slotAddFilesToProject(const QStringList &)" << endl;

    int s = KMessageBox::questionYesNo( 0,
        i18n("Do you want to be added to CVS repository too?"),
        i18n("CVS - New Files Added to Project"),
        KStdGuiItem::yes(),
        KStdGuiItem::no(),
        i18n("askWhenAddingNewFiles") );
    if (s == KMessageBox::Yes)
    {
        kdDebug( 9006 ) << "Adding these files: " << filesToAdd.join( ", " ) << endl;

        m_urls = KURL::List( prependToStringList( project()->projectDirectory(), filesToAdd ) );
        URLUtil::dump( m_urls );
        m_impl->add( m_urls );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePart::slotRemovedFilesFromProject(const QStringList &fileToRemove)
{
    kdDebug( 9006 ) << "====> CvsServicePart::slotRemovedFilesFromProject( const QStringList &)" << endl;

    int s = KMessageBox::questionYesNo( 0,
        i18n("Do you want them to be removed from CVS repository too?\nWarning: They will be removed from disk too!"),
        i18n("CVS - Files Removed From Project"),
        KStdGuiItem::yes(),
        KStdGuiItem::no(),
        i18n("askWhenRemovingFiles") );
    if (s == KMessageBox::Yes)
    {
        kdDebug( 9006 ) << "Removing these files: " << fileToRemove.join( ", " ) << endl;

        m_urls = KURL::List( prependToStringList( project()->projectDirectory(), fileToRemove ) );
        URLUtil::dump( m_urls );
        m_impl->remove( m_urls );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePart::slotProjectOpened()
{
    kdDebug(9006) << "CvsServicePart::slotProjectOpened() here!" << endl;

    // Avoid bothering the user if this project has no support for CVS
    CVSDir cvsdir( project()->projectDirectory() );
    if (!cvsdir.isValid())
    {
        kdDebug(9006) << "Project has no CVS Support: too bad!! :-(" << endl;
        return;
    }

    CvsOptions *options = CvsOptions::instance();

    // If createNewProject() has set this var then we have to get it.
    if (g_projectWasJustCreated)
    {
        options->save( project() );
        g_projectWasJustCreated = false;
    }
    options->load( project() );

    // When files are added to project they may be added to/removed from repository too
    connect( project(), SIGNAL(addedFilesToProject(const QStringList&)), this, SLOT(slotAddFilesToProject(const QStringList &)) );
    connect( project(), SIGNAL(removedFilesFromProject(const QStringList&)), this, SLOT(slotRemovedFilesFromProject(const QStringList &)) );
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePart::slotProjectClosed()
{
    kdDebug(9006) << "CvsServicePart::slotProjectClosed() here!" << endl;

    // Avoid bothering the user if this project has no support for CVS
    CVSDir cvsdir( project()->projectDirectory() );
    if (!cvsdir.isValid())
    {
        kdDebug(9006) << "Project had no CVS Support: too bad!! :-(" << endl;
        return;
    }

    CvsOptions *options = CvsOptions::instance();
    options->save( project() );
    delete options;

    // We don't have a project anymore ...
    disconnect( project(), SIGNAL(addedFilesToProject(const QStringList&)), this, SLOT(slotAddFilesToProject(const QStringList &)) );
    disconnect( project(), SIGNAL(removedFilesFromProject(const QStringList&)), this, SLOT(slotRemovedFilesFromProject(const QStringList &)) );
}

#include "cvspart.moc"


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

#include <kparts/part.h>
#include <kdevpartcontroller.h>
#include <kgenericfactory.h>

#include <qdir.h>
#include <qpopupmenu.h>

#include "kdevcore.h"
#include "kdevmakefrontend.h"
#include "kdevdifffrontend.h"
#include "kdevappfrontend.h"
#include "domutil.h"
#include "kdevmainwindow.h"
#include "kdevproject.h"
#include "urlutil.h"

#include "logform.h"
#include "cvsform.h"
#include "commitdlg.h"
#include "checkoutdialog.h"
#include "tagdialog.h"

//#include "changelog.h"
//#include "cvsutils.h"
#include "cvspart.h"
#include "cvsprocesswidget.h"
#include "cvsoptions.h"
#include "cvsoptionswidget.h"

#include "cvsserviceimpl.h"

#include <dcopref.h>
#include <repository_stub.h>
#include <cvsservice_stub.h>
#include <cvsjob_stub.h>

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

//
bool g_projectWasJustCreated = false;

///////////////////////////////////////////////////////////////////////////////
// Plugin factory
///////////////////////////////////////////////////////////////////////////////

typedef KGenericFactory<CvsPart> CvsFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevcvsservice, CvsFactory( "kdevcvsservice" ) );

///////////////////////////////////////////////////////////////////////////////
// class CvsPart
///////////////////////////////////////////////////////////////////////////////

CvsPart::CvsPart( QObject *parent, const char *name, const QStringList & )
    : KDevVersionControl( "KDevCvsServicePart", "kdevcvsservicepart", parent,
        name ? name : "CvsService" ),
    actionCommit( 0 ), actionDiff( 0 ), actionLog( 0 ), actionAdd( 0 ),
    actionAddBinary( 0 ), actionRemove( 0 ), actionUpdate( 0 ),
    actionRevert( 0 ),
    actionAddToIgnoreList( 0 ), actionRemoveFromIgnoreList( 0 ),
    actionTag( 0 ), actionUnTag( 0 ),
    actionLogin( 0), actionLogout( 0 ),
    m_impl( 0 )
{
    setInstance( CvsFactory::instance() );

    setXMLFile( "kdevcvsservicepart.rc" );

    init();
}

///////////////////////////////////////////////////////////////////////////////

CvsPart::~CvsPart()
{
    delete m_cvsConfigurationForm;
    delete m_impl;
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::init()
{
    setupActions();

    // Here we change the implementation
    // @fixme factory pattern here? :-/
    m_impl = new CvsServiceImpl( this );
    connect( m_impl, SIGNAL(checkoutFinished(QString)), SIGNAL(finishedFetching(QString)) );

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

    mainWindow()->embedOutputView( m_impl->processWidget(), i18n("CvsService"), i18n("cvs output") );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::setupActions()
{
/*
    KAction * action = new KAction( i18n("CVS Repository"), 0,
        this, SLOT(slotCheckOut()), actionCollection(), "cvsservice_checkout" );
    action->setStatusText( i18n("Check-out from an existing CVS repository") );
*/
    actionCommit = new KAction( i18n("&Commit"), 0, this,
        SLOT(slotActionCommit()), actionCollection(), "cvsservice_commit" );

    actionDiff = new KAction( i18n("&Difference Between Revisions"), 0, this, SLOT(slotActionDiff()),
        actionCollection(), "cvsservice_diff" );

    actionLog = new KAction( i18n("Generate &log"), 0, this, SLOT(slotActionLog()),
        actionCollection(), "cvsservice_log" );

    actionAdd = new KAction( i18n("&Add to Repository"), 0, this, SLOT(slotActionAdd()),
        actionCollection(), "cvsservice_add" );

    actionAddBinary = new KAction( i18n("Add to Repository as &Binary"), 0, this,
        SLOT(slotActionAddBinary()), actionCollection(), "cvsservice_add_bin" );

    actionRemove = new KAction( i18n("&Remove From Repository"), 0, this,
        SLOT(slotActionRemove()), actionCollection(), "cvsservice_remove" );

    actionUpdate = new KAction( i18n("&Update Local Copy to Most Recent Release"), 0, this,
        SLOT(slotActionUpdate()), actionCollection(), "cvsservice_update" );

    actionRevert = new KAction( i18n("R&evert to Previous Release"), 0,
        this, SLOT(slotActionRevert()), actionCollection(),
        "cvsservice_revert" );

    actionTag = new KAction( i18n("&Tag/Branch These File(s)"), 0,
        this, SLOT(slotActionTag()), actionCollection(),
        "cvsservice_tag" );

    actionUnTag = new KAction( i18n("Re&move Tag From These File(s)"), 0,
        this, SLOT(slotActionUnTag()), actionCollection(),
        "cvsservice_untag" );

    actionAddToIgnoreList = new KAction(
        i18n("&Ignore This File(s) in CVS Operations"), 0,
        this, SLOT(slotActionAddToIgnoreList()), actionCollection(),
        "cvsservice_ignore" );

    actionRemoveFromIgnoreList = new KAction(
        i18n("Do &Not Ignore This File in CVS Operations"), 0,
        this, SLOT(slotActionRemoveFromIgnoreList()), actionCollection(),
        "cvsservice_donot_ignore" );

    actionLogin = new KAction( i18n("&Login Into Server"), 0, this,
        SLOT(slotActionLogin()), actionCollection(), "cvsservice_login" );

    actionLogout = new KAction( i18n("L&ogout From Server"), 0, this,
        SLOT(slotActionLogout()), actionCollection(), "cvsservice_logout" );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::fetchFromRepository()
{
    m_impl->checkout();
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::createNewProject( const QString &dirName )
{
    kdDebug( 9000 ) << "====> CvsPart::createNewProject( const QString& )" << endl;

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

void CvsPart::projectConfigWidget( KDialogBase *dlg )
{
    QVBox *vbox = dlg->addVBoxPage( i18n("CvsService") );
    CvsOptionsWidget *w = new CvsOptionsWidget( (QWidget *)vbox, "cvs config widget" );
    connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
}

///////////////////////////////////////////////////////////////////////////////

QWidget* CvsPart::newProjectWidget( QWidget *parent )
{
    m_cvsConfigurationForm = new CvsForm( parent, "cvsform" );
    return m_cvsConfigurationForm;
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::contextMenu( QPopupMenu *popup, const Context *context )
{
    if (context->hasType( Context::FileContext ))
    {
        kdDebug(9000) << "contextMenu()" << endl;

        const FileContext *fcontext = static_cast<const FileContext*>( context );

        // THis stuff should end up into prepareOperation()
        m_urls = fcontext->urls();
        URLUtil::dump( m_urls );
        // FIXME: Here we currently avoid context menu on document because there is no document
        // selected and we'll need to connect slotAction*() instead of these.
        if (m_urls.count() <= 0)
            return;

        KPopupMenu *subMenu = new KPopupMenu( popup );

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
        subMenu->insertItem( actionUpdate->text(), this, SLOT(slotUpdate()) );
        subMenu->insertItem( actionRevert->text(), this, SLOT(slotRevert()) );

        subMenu->insertSeparator();
        subMenu->insertItem( actionAddToIgnoreList->text(), this, SLOT(slotAddToIgnoreList()) );
        subMenu->insertItem( actionRemoveFromIgnoreList->text(), this, SLOT(slotRemoveFromIgnoreList()) );

        // Now insert in parent menu
        popup->insertItem( i18n("CvsService"), subMenu );
    }
}

///////////////////////////////////////////////////////////////////////////////

bool CvsPart::urlFocusedDocument( KURL &url )
{
    kdDebug(9000) << "CvsPartImpl::retrieveUrlFocusedDocument() here!" << endl;
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
            kdDebug(9027) << "Cannot handle non-local files!" << endl;
        }
    }
   return false;
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotActionLogin()
{
    m_impl->login();
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotActionLogout()
{
    m_impl->logout();
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotActionCommit()
{
    KURL currDocument;
    if (urlFocusedDocument( currDocument ))
    {
        m_impl->commit( currDocument );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotActionUpdate()
{
    KURL currDocument;
    if (urlFocusedDocument( currDocument ))
    {
        m_impl->update( currDocument );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotActionAdd()
{
    KURL currDocument;
    if (urlFocusedDocument( currDocument ))
    {
        m_impl->add( currDocument, false );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotActionAddBinary()
{
    KURL currDocument;
    if (urlFocusedDocument( currDocument ))
    {
        m_impl->add( currDocument, true );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotActionRemove()
{
    KURL currDocument;
    if (urlFocusedDocument( currDocument ))
    {
        m_impl->remove( currDocument );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotActionRevert()
{
    KURL currDocument;
    if (urlFocusedDocument( currDocument ))
    {
        m_impl->revert( currDocument );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotActionLog()
{
    KURL currDocument;
    if (urlFocusedDocument( currDocument ))
    {
        m_impl->log( currDocument );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotActionDiff()
{
    KURL currDocument;
    if (urlFocusedDocument( currDocument ))
    {
        m_impl->diff( currDocument );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotActionTag()
{
    KURL currDocument;
    if (urlFocusedDocument( currDocument ))
    {
        m_impl->tag( currDocument );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotActionUnTag()
{
    KURL currDocument;
    if (urlFocusedDocument( currDocument ))
    {
        m_impl->unTag( currDocument );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotActionAddToIgnoreList()
{
    KURL currDocument;
    if (urlFocusedDocument( currDocument ))
    {
        m_impl->addToIgnoreList( currDocument );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotActionRemoveFromIgnoreList()
{
    KURL currDocument;
    if (urlFocusedDocument( currDocument ))
    {
        m_impl->removeFromIgnoreList( currDocument );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotCommit()
{
    m_impl->commit( m_urls );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotUpdate()
{
    m_impl->update( m_urls );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotAdd()
{
    m_impl->add( m_urls, false );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotAddBinary()
{
    m_impl->add( m_urls, true );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotRemove()
{
    m_impl->remove( m_urls );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotRevert()
{
    m_impl->revert( m_urls );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotLog()
{
    m_impl->log( m_urls );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotDiff()
{
    m_impl->diff( m_urls );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotTag()
{
    m_impl->tag( m_urls );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotUnTag()
{
    m_impl->unTag( m_urls );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotAddToIgnoreList()
{
    m_impl->addToIgnoreList( m_urls );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotRemoveFromIgnoreList()
{
    m_impl->removeFromIgnoreList( m_urls );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotStopButtonClicked( KDevPlugin* which )
{
    if ( which != 0 && which != this )
        return;

    m_impl->processWidget()->cancelJob();
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotAddFilesToProject( const QStringList &filesToAdd )
{
    kdDebug( 9000 ) << "====> CvsPart::slotAddFilesToProject(const QStringList &)" << endl;

    int s = KMessageBox::questionYesNo( 0,
        i18n("Do you want to be added to CVS repository too?"),
        i18n("CVS - New Files Added to Project"),
        KStdGuiItem::yes(),
        KStdGuiItem::no(),
        i18n("askWhenAddingNewFiles") );
    if (s == KMessageBox::Yes)
    {
        kdDebug( 9000 ) << "Adding these files: " << filesToAdd.join( ", " ) << endl;

        m_urls = KURL::List( prependToStringList( project()->projectDirectory(), filesToAdd ) );
        URLUtil::dump( m_urls );
        m_impl->add( m_urls );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotRemovedFilesFromProject(const QStringList &fileToRemove)
{
    kdDebug( 9000 ) << "====> CvsPart::slotRemovedFilesFromProject( const QStringList &)" << endl;

    int s = KMessageBox::questionYesNo( 0,
        i18n("Do you want them to be removed from CVS repository too?\nWarning: They will be removed from disk too!"),
        i18n("CVS - Files Removed From Project"),
        KStdGuiItem::yes(),
        KStdGuiItem::no(),
        i18n("askWhenRemovingFiles") );
    if (s == KMessageBox::Yes)
    {
        kdDebug( 9000 ) << "Removing these files: " << fileToRemove.join( ", " ) << endl;

        m_urls = KURL::List( prependToStringList( project()->projectDirectory(), fileToRemove ) );
        URLUtil::dump( m_urls );
        m_impl->remove( m_urls );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotProjectOpened()
{
    kdDebug(9000) << "CvsPart::slotProjectOpened() here!" << endl;

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

void CvsPart::slotProjectClosed()
{
    kdDebug(9000) << "CvsPart::slotProjectClosed() here!" << endl;

    CvsOptions *options = CvsOptions::instance();
    options->save( project() );
    delete options;
}

#include "cvspart.moc"


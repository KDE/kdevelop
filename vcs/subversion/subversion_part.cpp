/* Copyright (C) 2003
	 Mickael Marchand <marchand@kde.org>

	 This program is free software; you can redistribute it and/or
	 modify it under the terms of the GNU General Public
	 License as published by the Free Software Foundation; either
	 version 2 of the License, or (at your option) any later version.

	 This program is distributed in the hope that it will be useful,
	 but WITHOUT ANY WARRANTY; without even the implied warranty of
	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	 General Public License for more details.

	 You should have received a copy of the GNU General Public License
	 along with this program; see the file COPYING.  If not, write to
	 the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
	 Boston, MA 02111-1307, USA.
	 */

#include <qwhatsthis.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kaction.h>
#include <kpopupmenu.h>

#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "subversion_core.h"
#include "subversion_part.h"
#include "subversion_options_widget.h"
#include "subversion_project.h"
#include "urlutil.h"
#include <qvbox.h>
#include <kdialogbase.h>
#include "svn_co.h"
#include <kapplication.h>
#include <kurlrequester.h>
#include <klineedit.h>
#include <kio/job.h>

using namespace KIO;

typedef KGenericFactory<subversionPart> subversionFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevsubversion, subversionFactory( "kdevsubversion" ) );

bool g_projectWasJustCreated = false;

subversionPart::subversionPart(QObject *parent, const char *name, const QStringList& )
  : KDevVersionControl("subversion", "subversion", parent, name ? name : "Subversion" )
{
  setInstance(subversionFactory::instance());
  setXMLFile("kdevpart_subversion.rc");

	m_impl = new subversionCore( this );

	m_impl->processWidget()->setIcon( SmallIcon("db") );

	setupActions();

	connect( m_impl, SIGNAL(checkoutFinished(QString)), 
			SIGNAL(finishedFetching(QString)) );
	connect( m_impl, SIGNAL(fileStateChanged(const VCSFileInfoList&)), 
			SIGNAL(fileStateChanged(const VCSFileInfoList& ) ) );

	// Context menu
	connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
			this, SLOT(contextMenu(QPopupMenu *, const Context *)) );
	connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
			this, SLOT(projectConfigWidget(KDialogBase*)) );
	connect( core(), SIGNAL(stopButtonClicked(KDevPlugin*)),
			this, SLOT(slotStopButtonClicked(KDevPlugin*)) );

  mainWindow()->embedOutputView( m_impl->processWidget(), "Subversion", i18n( "Subversion messages" ) );
	setVersionControl( this );
}

subversionPart::~subversionPart()
{
	delete m_impl;
	delete m_projWidget;
}

void subversionPart::setupActions() {
    actionCommit = new KAction( i18n("&Commit to repository"), 0, this,
        SLOT(slotActionCommit()), actionCollection(), "subversion_commit" );
    actionCommit->setToolTip( i18n("Commit file(s)") );
    actionCommit->setWhatsThis( i18n("Commit file(s)<p>Commit file to repository if modified") );

/*    actionDiff = new KAction( i18n("&Difference between revisions"), 0, this, SLOT(slotActionDiff()),
        actionCollection(), "subversion_diff" );
    actionDiff->setToolTip( i18n("Build difference") );
    actionDiff->setWhatsThis( i18n("Build difference<p>Build difference between releases") );
*/
    actionAdd = new KAction( i18n("&Add to repository"), 0, this, SLOT(slotActionAdd()),
        actionCollection(), "subversion_add" );
    actionAdd->setToolTip( i18n("Add file to repository") );
    actionAdd->setWhatsThis( i18n("Add file <p>Add file to repository") );

    actionRemove = new KAction( i18n("&Remove from repository"), 0, this,
        SLOT(slotActionRemove()), actionCollection(), "subversion_remove" );
    actionRemove->setToolTip( i18n("Remove from repository") );
    actionRemove->setWhatsThis( i18n("Remove from repository<p>Remove file(s) from repository") );

    actionUpdate = new KAction( i18n("&Update"), 0, this,
        SLOT(slotActionUpdate()), actionCollection(), "subversion_update" );
    actionUpdate->setToolTip( i18n("Update") );
    actionUpdate->setWhatsThis( i18n("Update<p>Update file(s) from repository") );

    actionAddToIgnoreList = new KAction( i18n("&Ignore in Subversion operations"), 0,
        this, SLOT(slotActionAddToIgnoreList()), actionCollection(), "subversion_ignore" );
    actionAddToIgnoreList->setToolTip( i18n("Ignore in Subversion operations") );
    actionAddToIgnoreList->setWhatsThis( i18n("Ignore in Subversion operations<p>Ignore file(s)") );

    actionRemoveFromIgnoreList = new KAction( i18n("Do &not ignore in Subversion operations"), 0,
        this, SLOT(slotActionRemoveFromIgnoreList()), actionCollection(), "subversion_donot_ignore" );
    actionRemoveFromIgnoreList->setToolTip( i18n("Do not ignore in Subversion operations") );
    actionRemoveFromIgnoreList->setWhatsThis( i18n("Do not ignore in Subversion operations<p>Do not ignore file(s)") );
}

QWidget* subversionPart::newProjectWidget( QWidget* parent ) {
	m_projWidget = new subversionProjectWidget(parent,"projectwidget");
	return m_projWidget;
}

void subversionPart::projectConfigWidget( KDialogBase *dlg ) {
	QVBox *vbox = dlg->addVBoxPage( i18n("Subversion") );
	subversionOptionsWidget *w = new subversionOptionsWidget( (QWidget *)vbox, "svn config widget" );
	connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
}

void subversionPart::createNewProject( const QString& dirname ) {
	if ( !m_projWidget ) return;
	
	// @fixme: actually there is no way to inform that a _new_ ("just created")
	// project has been opened because projectOpened() is emitted after the project
	// has been created :-/ So the only way to inform that slotProjectOpened() to not
	// load default settings (overriding the CvsOptions instance is to set this flag
	// here ...
	g_projectWasJustCreated = true;

/*	m_impl->createNewProject( dirName,
			m_confWidget->cvsRsh(), m_cvsConfigurationForm->location(),
			m_confWidget->message(), m_cvsConfigurationForm->module(),
			m_confWidget->vendor(), m_cvsConfigurationForm->release(),
			m_confWidget->mustInitRoot()
			);*/

}

void subversionPart::fetchFromRepository() {
	svn_co checkoutDlg;

	if ( checkoutDlg.exec() == QDialog::Accepted ) {
		//checkout :)
		QByteArray parms;
		QDataStream s( parms, IO_WriteOnly );
		KURL servURL ( checkoutDlg.serverURL->url() );
		int cmd = 1;
		int rev = -1;
		s << cmd << servURL << KURL( checkoutDlg.localDir->url() + "/" + checkoutDlg.newDir->text() ) << rev << QString( "HEAD" );
		servURL.setProtocol( "svn+" + servURL.protocol() ); //make sure it starts with "svn"
		SimpleJob * job = KIO::special(servURL,parms, true);
		connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotResult( KIO::Job * ) ) );
	}
}

void subversionPart::slotResult( KIO::Job * job ) {
	if ( job->error() )
		job->showErrorDialog( 0L );
}

KDevVCSFileInfoProvider * subversionPart::fileInfoProvider() const {

}

bool subversionPart::isValidDirectory( const QString& dirPath) {

}

void subversionPart::contextMenu( QPopupMenu *popup, const Context *context ) {
    kdDebug() << "contextMenu()" << endl;
    if (context->hasType( Context::FileContext ) ||
        context->hasType( Context::EditorContext ))
    {

        if (context->hasType( Context::FileContext ))
        {
            kdDebug() << "Requested for a FileContext" << endl;
            const FileContext *fcontext = static_cast<const FileContext*>( context );
            m_urls = fcontext->urls();
        }
        else
        {
            kdDebug() << "Requested for an EditorContext" << endl;
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
/*        if (m_urls.count() == 1)
        {
            subMenu->insertItem( actionDiff->text(), this, SLOT(slotDiff()) );
            subMenu->insertItem( actionLog->text(), this, SLOT(slotLog()) );
        }*/
        subMenu->insertItem( actionAdd->text(), this, SLOT(slotAdd()) );
        subMenu->insertItem( actionRemove->text(), this, SLOT(slotRemove()) );

        subMenu->insertSeparator();
        subMenu->insertItem( actionUpdate->text(), this, SLOT(slotUpdate()) );

        subMenu->insertSeparator();
        subMenu->insertItem( actionAddToIgnoreList->text(), this, SLOT(slotAddToIgnoreList()) );
        subMenu->insertItem( actionRemoveFromIgnoreList->text(), this, SLOT(slotRemoveFromIgnoreList()) );

        // Now insert in parent menu
        popup->insertItem( i18n("Subversion"), subMenu );
    }
}


#include "subversion_part.moc"

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
#include "subversionoptionswidgetimpl.h"
#include "subversionprojectwidget.h"
#include "urlutil.h"
#include <qvbox.h>
#include <kdialogbase.h>
#include <kparts/part.h>
#include <kdevpartcontroller.h>
#include <kdevproject.h>
#include <domutil.h>
#include <kurlrequester.h>
#include <qradiobutton.h>
#include <kdebug.h>
#include <qwidget.h>

typedef KGenericFactory<subversionPart> subversionFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevsubversion, subversionFactory( "kdevsubversion" ) );

//bool g_projectWasJustCreated = false;

subversionPart::subversionPart(QObject *parent, const char *name, const QStringList& )
	: KDevVersionControl("KDevsubversionPart", "kdevsubversionpart", parent, name ? name : "Subversion" ) {
	setInstance(subversionFactory::instance());
	setXMLFile("kdevpart_subversion.rc");
	m_projWidget = 0;

	m_impl = new subversionCore( this );

	m_checkout_recurse = true;
	m_update_recurse = true;
	m_switch_recurse = true;
	m_add_recurse = true;
	m_remove_force = true;
	m_commit_recurse = true;
	m_diff_recurse = true;
	m_merge_recurse = true;
	m_merge_overwrite = true;
	m_relocate_recurse = true;
	m_revert_recurse = true;
	m_resolve_recurse = true;
	m_move_force = true;
	m_propset_recurse = true;
	m_propget_recurse = true;
	m_proplist_recurse = true;

	//m_impl->processWidget()->setIcon( SmallIcon("db") );

	setupActions();

	connect( m_impl, SIGNAL(checkoutFinished(QString)), SIGNAL(finishedFetching(QString)) );

	// Context menu
	connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)), this, SLOT(contextMenu(QPopupMenu *, const Context *)) );
	connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)), this, SLOT(projectConfigWidget(KDialogBase*)) );
	connect( core(), SIGNAL(stopButtonClicked(KDevPlugin*)), this, SLOT(slotStopButtonClicked(KDevPlugin*)) );
	connect( core(), SIGNAL(projectOpened()), this, SLOT(slotProjectOpened()) );
	connect( core(), SIGNAL(projectClosed()), this, SLOT(slotProjectClosed()) );

	mainWindow()->embedOutputView( (QWidget*)m_impl->processWidget(), i18n( "Subversion" ), i18n( "Subversion messages" ) );
	setVersionControl( this );
}

subversionPart::~subversionPart() {
	if ( m_projWidget ){
		delete (subversionProjectWidget*) m_projWidget;
		m_projWidget = 0;
	}
	delete m_impl;
}

void subversionPart::setupActions() {
	actionCommit = new KAction( i18n("&Commit to Repository"), 0, this,
			SLOT(slotActionCommit()), actionCollection(), "subversion_commit" );
	actionCommit->setToolTip( i18n("Commit file(s)") );
	actionCommit->setWhatsThis( i18n("Commit file(s)<p>Commit file to repository if modified") );

	/*    actionDiff = new KAction( i18n("&Difference between revisions"), 0, this, SLOT(slotActionDiff()),
				actionCollection(), "subversion_diff" );
				actionDiff->setToolTip( i18n("Build difference") );
				actionDiff->setWhatsThis( i18n("Build difference<p>Build difference between releases") );
				*/
	actionAdd = new KAction( i18n("&Add to Repository"), 0, this, SLOT(slotActionAdd()),
			actionCollection(), "subversion_add" );
	actionAdd->setToolTip( i18n("Add file to repository") );
	actionAdd->setWhatsThis( i18n("Add file <p>Add file to repository") );

	actionRemove = new KAction( i18n("&Remove From Repository"), 0, this,
			SLOT(slotActionRemove()), actionCollection(), "subversion_remove" );
	actionRemove->setToolTip( i18n("Remove from repository") );
	actionRemove->setWhatsThis( i18n("Remove from repository<p>Remove file(s) from repository") );

	actionUpdate = new KAction( i18n("&Update"), 0, this,
			SLOT(slotActionUpdate()), actionCollection(), "subversion_update" );
	actionUpdate->setToolTip( i18n("Update") );
	actionUpdate->setWhatsThis( i18n("Update<p>Update file(s) from repository") );

	actionAddToIgnoreList = new KAction( i18n("&Ignore in Subversion Operations"), 0,
			this, SLOT(slotActionAddToIgnoreList()), actionCollection(), "subversion_ignore" );
	actionAddToIgnoreList->setToolTip( i18n("Ignore in Subversion operations") );
	actionAddToIgnoreList->setWhatsThis( i18n("Ignore in Subversion operations<p>Ignore file(s)") );

	actionRemoveFromIgnoreList = new KAction( i18n("Do &Not Ignore in Subversion Operations"), 0,
			this, SLOT(slotActionRemoveFromIgnoreList()), actionCollection(), "subversion_donot_ignore" );
	actionRemoveFromIgnoreList->setToolTip( i18n("Do not ignore in Subversion operations") );
	actionRemoveFromIgnoreList->setWhatsThis( i18n("Do not ignore in Subversion operations<p>Do not ignore file(s)") );
}

QWidget* subversionPart::newProjectWidget( QWidget* parent ) {
	if ( !m_projWidget )
		m_projWidget = new subversionProjectWidget(parent,"projectwidget");
	return m_projWidget;
}

void subversionPart::projectConfigWidget( KDialogBase *dlg ) {
	QVBox *vbox = dlg->addVBoxPage( i18n("Subversion") );
	subversionOptionsWidgetImpl *w = new subversionOptionsWidgetImpl( (QWidget *)vbox, this, "svn config widget" );
	connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
}

void subversionPart::createNewProject( const QString& dirname ) {
	if ( !m_projWidget ) return;

	m_impl->createNewProject( dirname, m_projWidget->importURL->url(), m_projWidget->yes->isChecked() );

}

void subversionPart::fetchFromRepository() {
	m_impl->checkout();
}

KDevVCSFileInfoProvider * subversionPart::fileInfoProvider() const {
	return 0;
}

bool subversionPart::isValidDirectory( const QString& dirPath) {
	return true;
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

bool subversionPart::urlFocusedDocument( KURL &url ) {
	KParts::ReadOnlyPart *part = dynamic_cast<KParts::ReadOnlyPart*>( partController()->activePart() );
	if ( part ) {
		if (part->url().isLocalFile() ) {
			url = part->url();
			return true;
		}
	}
	return false;
}

void subversionPart::slotActionUpdate() {
	kdDebug() << "subversion: slotActionUpdate()" << endl;
	KURL doc;
	if (urlFocusedDocument( doc )) {
		m_impl->update( doc );
	}
}

void subversionPart::slotUpdate() {
	m_impl->update (m_urls);
}

void subversionPart::slotActionCommit() {
	kdDebug() << "subversion: slotActionCommit()" << endl;
	KURL doc;
	if (urlFocusedDocument( doc )) {
		m_impl->commit( doc );
	}
}

void subversionPart::slotCommit() {
	m_impl->commit (m_urls);
}

void subversionPart::slotProjectOpened() {
	kdDebug() << "subversion :projectOpened" << endl;
/*	if ( g_projectWasJustCreated ) {
		//saveOptions();
		g_projectWasJustCreated = false;
	}	*/
	//loadOptions();
	//FIXME
	//connect( project(), SIGNAL(addedFilesToProject(const QStringList&)), this, SLOT(slotAddFilesToProject(const QStringList &)) );
	//connect( project(), SIGNAL(removedFilesFromProject(const QStringList&)), this, SLOT(slotRemovedFilesFromProject(const QStringList &)) );
}

void subversionPart::slotProjectClosed() {
	kdDebug() << "subversion :projectClosed" << endl;
	//saveOptions();
	//FIXME
	//disconnect( project(), SIGNAL(addedFilesToProject(const QStringList&)), this, SLOT(slotAddFilesToProject(const QStringList &)) );
	//disconnect( project(), SIGNAL(removedFilesFromProject(const QStringList&)), this, SLOT(slotRemovedFilesFromProject(const QStringList &)) );
}

void subversionPart::savePartialProjectSession(QDomElement* dom) {
	kdDebug() << "subversion : savePartialProjectSession" << endl;
	QDomDocument doc = dom->ownerDocument();
	QDomElement svn = doc.createElement( "subversion" );
	svn.setAttribute("recursecheckout", m_checkout_recurse);
	svn.setAttribute("recurseupdate", m_update_recurse);
	svn.setAttribute("recurseswitch", m_switch_recurse);
	svn.setAttribute("recurseadd", m_add_recurse);
	svn.setAttribute("forceremove",m_remove_force);
	svn.setAttribute("recursecommit",m_commit_recurse);
	svn.setAttribute("recursediff", m_diff_recurse);
	svn.setAttribute("recursemerge", m_merge_recurse);
	svn.setAttribute("forcemerge", m_merge_overwrite);
	svn.setAttribute("recurserelocate", m_relocate_recurse);
	svn.setAttribute("recurserevert", m_revert_recurse);
	svn.setAttribute("recurseresolve", m_resolve_recurse);
	svn.setAttribute("forcemove", m_move_force);
	svn.setAttribute("recursepropset", m_propset_recurse);
	svn.setAttribute("recursepropget", m_propget_recurse);
	svn.setAttribute("recurseproplist", m_proplist_recurse);
	svn.setAttribute( "base", base.url() );
	dom->appendChild( svn );
}

void subversionPart::restorePartialProjectSession(const QDomElement* dom) {
	kdDebug() << "subversion : restorePartialProjectSession" << endl;
	QDomElement svn = dom->namedItem("subversion").toElement();

	m_checkout_recurse = svn.attribute( "recursecheckout", "1" ).toInt();
	m_update_recurse = svn.attribute("recurseupdate","1").toInt();
	m_switch_recurse = svn.attribute("recurseswitch","1").toInt();
	m_add_recurse = svn.attribute("recurseadd","1").toInt();
	m_remove_force = svn.attribute("forceremove","1").toInt();
	m_commit_recurse = svn.attribute("recursecommit","1").toInt();
	m_diff_recurse = svn.attribute("recursediff","1").toInt();
	m_merge_recurse = svn.attribute("recursemerge","1").toInt();
	m_merge_overwrite = svn.attribute("forcemerge","1").toInt();
	m_relocate_recurse = svn.attribute("recurserelocate","1").toInt();
	m_revert_recurse = svn.attribute("recurserevert","1").toInt();
	m_move_force = svn.attribute("forcemove","1").toInt();
	m_resolve_recurse = svn.attribute("recurseresolve","1").toInt();
	m_propset_recurse = svn.attribute("recursepropset","1").toInt();
	m_propget_recurse = svn.attribute("recursepropget","1").toInt();
	m_proplist_recurse = svn.attribute("recurseproplist","1").toInt();
	base = svn.attribute( "base", "" );
}

#include "subversion_part.moc"

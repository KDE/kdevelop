/**
	 Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>

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
#include <kdevgenericfactory.h>
#include <kaction.h>
#include <kpopupmenu.h>

#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "subversion_core.h"
#include "subversion_part.h"
#include "subversion_widget.h"
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
#include <kdevplugininfo.h>

static const KDevPluginInfo data("kdevsubversion");

typedef KDevGenericFactory<subversionPart> subversionFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevsubversion, subversionFactory( data ) )

//bool g_projectWasJustCreated = false;

subversionPart::subversionPart(QObject *parent, const char *name, const QStringList& )
	: KDevVersionControl(&data, parent, name ? name : "Subversion" ) {
	setInstance(subversionFactory::instance());
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

	m_impl->processWidget()->setCaption(i18n( "Subversion Output" ));
	mainWindow()->embedOutputView( (QWidget*)m_impl->processWidget(), i18n( "Subversion" ), i18n( "Subversion messages" ) );
    QWhatsThis::add((QWidget*)m_impl->processWidget(), i18n("<b>Subversion</b><p>Subversion operations window."));
}

subversionPart::~subversionPart() {
	if ( m_projWidget ){
		delete (subversionProjectWidget*) m_projWidget;
		m_projWidget = 0;
	}
	delete m_impl;
}

void subversionPart::setupActions() {
	actionCommit = new KAction( i18n("&Commit to Repository"), 0, this, SLOT(slotActionCommit()), actionCollection(), "subversion_commit" );
	actionCommit->setToolTip( i18n("Commit file(s)") );
	actionCommit->setWhatsThis( i18n("<b>Commit file(s)</b><p>Commits file to repository if modified.") );

	/*    actionDiff = new KAction( i18n("&Difference Between Revisions"), 0, this, SLOT(slotActionDiff()),
				actionCollection(), "subversion_diff" );
				actionDiff->setToolTip( i18n("Build difference") );
				actionDiff->setWhatsThis( i18n("<b>Build difference</b><p>Builds difference between releases.") );
				*/
	actionAdd = new KAction( i18n("&Add to Repository"), 0, this, SLOT(slotActionAdd()), actionCollection(), "subversion_add" );
	actionAdd->setToolTip( i18n("Add file to repository") );
	actionAdd->setWhatsThis( i18n("<b>Add file to repository</b><p>Adds file to repository.") );

	actionRemove = new KAction( i18n("&Remove From Repository"), 0, this, SLOT(slotActionDel()), actionCollection(), "subversion_remove" );
	actionRemove->setToolTip( i18n("Remove from repository") );
	actionRemove->setWhatsThis( i18n("<b>Remove from repository</b><p>Removes file(s) from repository.") );

	actionUpdate = new KAction( i18n("&Update"), 0, this, SLOT(slotActionUpdate()), actionCollection(), "subversion_update" );
	actionUpdate->setToolTip( i18n("Update") );
	actionUpdate->setWhatsThis( i18n("<b>Update</b><p>Updates file(s) from repository.") );

	actionRevert = new KAction( i18n("&Revert"), 0, this, SLOT(slotActionRevert()), actionCollection(), "subversion_revert" );
	actionRevert->setToolTip( i18n("Revert") );
	actionRevert->setWhatsThis( i18n("<b>Revert</b><p>Undo local changes.") );

	actionAddToIgnoreList = new KAction( i18n("&Ignore in Subversion Operations"), 0,
			this, SLOT(slotActionAddToIgnoreList()), actionCollection(), "subversion_ignore" );
	actionAddToIgnoreList->setToolTip( i18n("Ignore in Subversion operations") );
	actionAddToIgnoreList->setWhatsThis( i18n("<b>Ignore in Subversion operations</b><p>Ignores file(s).") );

	actionRemoveFromIgnoreList = new KAction( i18n("Do &Not Ignore in Subversion Operations"), 0,
			this, SLOT(slotActionRemoveFromIgnoreList()), actionCollection(), "subversion_donot_ignore" );
	actionRemoveFromIgnoreList->setToolTip( i18n("Do not ignore in Subversion operations") );
	actionRemoveFromIgnoreList->setWhatsThis( i18n("<b>Do not ignore in Subversion operations</b><p>Do not ignore file(s).") );

	actionResolve = new KAction( i18n("Re&solve conflicting state"), 0,
			this, SLOT(slotActionResolve()), actionCollection(), "subversion_resolve" );
	actionResolve->setToolTip( i18n("Resolve the conflicting state of a file after a merge") );
	actionResolve->setWhatsThis( i18n("<b>Resolve the conflicting state</b><p>Remove the conflict state that can be set on a file after a merge failed.") );
}

QWidget* subversionPart::newProjectWidget( QWidget* parent ) {
	if ( !m_projWidget )
		m_projWidget = new subversionProjectWidget(parent,"projectwidget");
	return m_projWidget;
}

void subversionPart::projectConfigWidget( KDialogBase *dlg ) {
	QVBox *vbox = dlg->addVBoxPage( i18n("Subversion"), i18n("Subversion"), BarIcon( info()->icon(), KIcon::SizeMedium) );
	subversionOptionsWidgetImpl *w = new subversionOptionsWidgetImpl( (QWidget *)vbox, this, "svn config widget" );
	connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
}

void subversionPart::createNewProject( const QString& dirname ) {
	if ( !m_projWidget ) return;

	m_impl->createNewProject( dirname, KURL( m_projWidget->importURL->url() ), m_projWidget->yes->isChecked() );

}

bool subversionPart::fetchFromRepository() {
	m_impl->checkout();
        return true;
}

KDevVCSFileInfoProvider * subversionPart::fileInfoProvider() const {
	return m_impl->fileInfoProvider();
}

void subversionPart::contextMenu( QPopupMenu *popup, const Context *context ) {
// If the current project doesn't support SVN, we don't
// want to confuse the user with a SVN popup menu.
if(!project() || !isValidDirectory(project()->projectDirectory()))
  return;

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
                if (context->hasType( Context::FileContext ))
		    popup->insertSeparator();

		int id = subMenu->insertItem( actionCommit->text(), this, SLOT(slotCommit()) );
		// CvsService let to do log and diff operations only on one file (or directory) at time
		/*        if (m_urls.count() == 1)
							{
							subMenu->insertItem( actionDiff->text(), this, SLOT(slotDiff()) );
							subMenu->insertItem( actionLog->text(), this, SLOT(slotLog()) );
							}*/
        subMenu->setWhatsThis(id, i18n("<b>Commit file(s)</b><p>Commits file to repository if modified."));
		id = subMenu->insertItem( actionAdd->text(), this, SLOT(slotAdd()) );
        subMenu->setWhatsThis(id, i18n("<b>Add file to repository</b><p>Adds file to repository."));
		id = subMenu->insertItem( actionRemove->text(), this, SLOT(slotDel()) );
        subMenu->setWhatsThis(id, i18n("<b>Remove from repository</b><p>Removes file(s) from repository."));

		subMenu->insertSeparator();
		id = subMenu->insertItem( actionUpdate->text(), this, SLOT(slotUpdate()) );
        subMenu->setWhatsThis(id, i18n("<b>Update</b><p>Updates file(s) from repository."));
		id = subMenu->insertItem( actionRevert->text(), this, SLOT(slotRevert()) );
		subMenu->setWhatsThis(id, i18n("<b>Revert</b><p>Undo local changes.") );
		id = subMenu->insertItem( actionResolve->text(), this, SLOT(slotResolve()) );
		subMenu->setWhatsThis(id, i18n("<b>Resolve</b><p>Resolve conflicting state.") );

		subMenu->insertSeparator();
		id = subMenu->insertItem( actionAddToIgnoreList->text(), this, SLOT(slotAddToIgnoreList()) );
        subMenu->setWhatsThis(id, i18n("<b>Ignore in Subversion operations</b><p>Ignores file(s)."));
		id = subMenu->insertItem( actionRemoveFromIgnoreList->text(), this, SLOT(slotRemoveFromIgnoreList()) );
        subMenu->setWhatsThis(id, i18n("<b>Do not ignore in Subversion operations</b><p>Do not ignore file(s)."));

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

void subversionPart::slotActionResolve() {
	kdDebug() << "subversion: slotActionResolve()" << endl;
	KURL doc;
	if (urlFocusedDocument( doc )) {
		m_impl->resolve( doc );
	}
}

void subversionPart::slotResolve() {
	m_impl->resolve (m_urls);
}

void subversionPart::slotActionCommit() {
	kdDebug() << "subversion: slotActionCommit()" << endl;
	KURL doc;
	if (urlFocusedDocument( doc )) {
		m_impl->commit( doc );
	}
}

void subversionPart::slotActionAdd() {
	kdDebug() << "subversion: slotActionAdd()" << endl;
	KURL doc;
	if (urlFocusedDocument( doc )) {
		m_impl->add( doc );
	}
}

void subversionPart::slotActionDel() {
	kdDebug() << "subversion: slotActionDel()" << endl;
	KURL doc;
	if (urlFocusedDocument( doc )) {
		m_impl->del( doc );
	}
}

void subversionPart::slotActionRevert() {
	kdDebug() << "subversion: slotActionRevert()" << endl;
	KURL doc;
	if (urlFocusedDocument( doc )) {
		m_impl->revert( doc );
	}
}

void subversionPart::slotCommit() {
	m_impl->commit (m_urls);
}

void subversionPart::slotAdd() {
	m_impl->add (m_urls);
}

void subversionPart::slotDel() {
	m_impl->del (m_urls);
}

void subversionPart::slotRevert() {
	m_impl->revert (m_urls);
}

void subversionPart::slotProjectOpened() {
	kdDebug() << "subversion :projectOpened" << endl;
/*	if ( g_projectWasJustCreated ) {
		//saveOptions();
		g_projectWasJustCreated = false;
	}	*/
	//loadOptions();
	/// \FIXME slots
	//connect( project(), SIGNAL(addedFilesToProject(const QStringList&)), this, SLOT(slotAddFilesToProject(const QStringList &)) );
	//connect( project(), SIGNAL(removedFilesFromProject(const QStringList&)), this, SLOT(slotRemovedFilesFromProject(const QStringList &)) );
}

void subversionPart::slotProjectClosed() {
	kdDebug() << "subversion :projectClosed" << endl;
	//saveOptions();
	/// \FIXME slots
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

bool subversionPart::isValidDirectory( const QString &dirPath) const {
    QString svn = "/.svn/";
    QDir svndir( dirPath + svn );
    QString entriesFileName = dirPath + svn + "entries";

	kdDebug() << "dirpath " << dirPath+"/.svn/" << svndir.exists() << endl;
	kdDebug() << "entries " << entriesFileName << QFile::exists( entriesFileName ) << endl;
    return svndir.exists() &&
        QFile::exists( entriesFileName );
}

#include "subversion_part.moc"

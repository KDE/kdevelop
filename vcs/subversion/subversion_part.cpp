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
#include "subversion_part.h"

#include <qwhatsthis.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdevgenericfactory.h>
#include <kaction.h>
#include <kpopupmenu.h>

#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "subversion_core.h"
#include "subversion_widget.h"
#include "subversionprojectwidget.h"
#include "subversion_fileinfo.h"
#include "subversion_global.h"
#include "kdevversioncontrol.h"
#include "svn_fileselectdlg_commit.h"
#include "svn_logviewwidget.h"
#include "svn_switchwidget.h"
#include "svn_copywidget.h"
#include "svn_mergewidget.h"

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

#include <kmessagebox.h>

using namespace SvnGlobal;

static const KDevPluginInfo data("kdevsubversion");

typedef KDevGenericFactory<subversionPart> subversionFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevsubversion, subversionFactory( data ) )

//bool g_projectWasJustCreated = false;

subversionPart::subversionPart(QObject *parent, const char *name, const QStringList& )
	: KDevVersionControl(&data, parent, name ? name : "Subversion" ) {
	setInstance(subversionFactory::instance());
	m_projWidget = 0;

	m_impl = new subversionCore( this );

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
	actionCommit = new KAction( i18n("&Commit to Repository..."), 0, this, SLOT(slotActionCommit()), actionCollection(), "subversion_commit" );
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
	
	actionLog = new KAction( i18n("Show logs..."), 0, this, SLOT(slotLog()), actionCollection(), "subversion_log" );
	actionBlame = new KAction( i18n("Blame..."), 0, this, SLOT(slotBlame()), actionCollection(), "subversion_blame");

	actionRemove = new KAction( i18n("&Remove From Repository"), 0, this, SLOT(slotActionDel()), actionCollection(), "subversion_remove" );
	actionRemove->setToolTip( i18n("Remove from repository") );
	actionRemove->setWhatsThis( i18n("<b>Remove from repository</b><p>Removes file(s) from repository.") );

	actionUpdate = new KAction( i18n("&Update"), 0, this, SLOT(slotActionUpdate()), actionCollection(), "subversion_update" );
	actionUpdate->setToolTip( i18n("Update") );
	actionUpdate->setWhatsThis( i18n("<b>Update</b><p>Updates file(s) from repository.") );

	actionDiffLocal = new KAction( i18n("&Diff to BASE"), 0, this, SLOT(slotActionDiffLocal()), actionCollection(), "subversion_diff_local" );
	actionDiffLocal->setToolTip( i18n("Diff to BASE") );
	actionDiffLocal->setWhatsThis( i18n("<b>Diff to disk</b><p>Diff current file to the BASE checked out copy.") );

	actionDiffHead = new KAction( i18n("&Diff to HEAD"), 0, this, SLOT(slotActionDiffLocal()), actionCollection(), "subversion_diff_head" );
	actionDiffHead->setToolTip( i18n("Diff to HEAD") );
	actionDiffHead->setWhatsThis( i18n("<b>Diff HEAD</b><p>Diff the current file to HEAD in svn.") );


	actionRevert = new KAction( i18n("&Revert"), 0, this, SLOT(slotActionRevert()), actionCollection(), "subversion_revert" );
	actionRevert->setToolTip( i18n("Revert") );
	actionRevert->setWhatsThis( i18n("<b>Revert</b><p>Undo local changes.") );

	/*
	actionAddToIgnoreList = new KAction( i18n("&Ignore in Subversion Operations"), 0,
			this, SLOT(slotActionAddToIgnoreList()), actionCollection(), "subversion_ignore" );
	actionAddToIgnoreList->setToolTip( i18n("Ignore in Subversion operations") );
	actionAddToIgnoreList->setWhatsThis( i18n("<b>Ignore in Subversion operations</b><p>Ignores file(s).") );

	actionRemoveFromIgnoreList = new KAction( i18n("Do &Not Ignore in Subversion Operations"), 0,
			this, SLOT(slotActionRemoveFromIgnoreList()), actionCollection(), "subversion_donot_ignore" );
	actionRemoveFromIgnoreList->setToolTip( i18n("Do not ignore in Subversion operations") );
	actionRemoveFromIgnoreList->setWhatsThis( i18n("<b>Do not ignore in Subversion operations</b><p>Do not ignore file(s).") );
	*/
	actionResolve = new KAction( i18n("Re&solve Conflicting State"), 0,
			this, SLOT(slotActionResolve()), actionCollection(), "subversion_resolve" );
	actionResolve->setToolTip( i18n("Resolve the conflicting state of a file after a merge") );
	actionResolve->setWhatsThis( i18n("<b>Resolve the conflicting state</b><p>Remove the conflict state that can be set on a file after a merge failed.") );
	actionSwitch = new KAction( i18n("Switch this working copy to URL.."), 0,
			this, SLOT(slotSwitch()), actionCollection(), "subversion_switch" );
	// warn slogCopy(), slotMerge only works on context menu. There is no main-menu action
	actionCopy = new KAction( i18n("Copy this working copy to URL.."), 0,
								this, SLOT(slotCopy()), actionCollection(), "subversion_copy" );
	actionMerge = new KAction( i18n("Merge difference to working copy"), 0,
							  this, SLOT(slotMerge()), actionCollection(), "subversion_merge" );
}

QWidget* subversionPart::newProjectWidget( QWidget* parent ) {
	if ( !m_projWidget )
		m_projWidget = new subversionProjectWidget(parent,"projectwidget");
	return m_projWidget;
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
  //no project, no subversion. Don't test on projectDirectory() here. If the user wants this project to have subversion support
  //give it to him. e.g. for out of root subprojects like with qmake
if(!project())
  return;

	kdDebug(9036) << "contextMenu()" << endl;
	if (context->hasType( Context::FileContext ) ||
			context->hasType( Context::EditorContext ))
	{

		if (context->hasType( Context::FileContext ))
		{
			kdDebug(9036) << "Requested for a FileContext" << endl;
			const FileContext *fcontext = static_cast<const FileContext*>( context );
			m_urls = fcontext->urls();
		}
		else
		{
			kdDebug(9036) << "Requested for an EditorContext" << endl;
			const EditorContext *editorContext = static_cast<const EditorContext*>( context );
			m_urls = editorContext->url();
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
		id = subMenu->insertItem( actionLog->text(), this, SLOT(slotLog()) );
		subMenu->setWhatsThis(id, i18n("<b>Show logs..</b><p>View Logs"));
		id = subMenu->insertItem( actionBlame->text(), this, SLOT(slotBlame()) );
		subMenu->setWhatsThis(id, i18n("<b>Blame 0:HEAD </b><p>Show Annotate"));
		
		subMenu->insertSeparator();
		id = subMenu->insertItem( actionDiffLocal->text(), this, SLOT(slotDiffLocal()) );
		subMenu->setWhatsThis(id, i18n("<b>Diff</b><p>Diff file to local disk."));

		id = subMenu->insertItem( actionDiffHead->text(), this, SLOT(slotDiffHead()) );
		subMenu->setWhatsThis(id, i18n("<b>Diff</b><p>Diff file to repository."));

		id = subMenu->insertItem( actionUpdate->text(), this, SLOT(slotUpdate()) );
        subMenu->setWhatsThis(id, i18n("<b>Update</b><p>Updates file(s) from repository."));
		id = subMenu->insertItem( actionRevert->text(), this, SLOT(slotRevert()) );
		subMenu->setWhatsThis(id, i18n("<b>Revert</b><p>Undo local changes.") );
		id = subMenu->insertItem( actionResolve->text(), this, SLOT(slotResolve()) );
		subMenu->setWhatsThis(id, i18n("<b>Resolve</b><p>Resolve conflicting state.") );
		id = subMenu->insertItem( actionSwitch->text(), this, SLOT(slotSwitch()) );
		subMenu->setWhatsThis(id, i18n("<b>Switch</b><p>Switch working tree.") );
		id = subMenu->insertItem( actionCopy->text(), this, SLOT(slotCopy()) );
		subMenu->setWhatsThis(id, i18n("<b>Copy</b><p>Copy from/between path/URLs") );
		id = subMenu->insertItem( actionMerge->text(), this, SLOT(slotMerge()) );
		subMenu->setWhatsThis(id, i18n("<b>Merge</b><p>Merge difference to working copy") );

		/*
		subMenu->insertSeparator();
		id = subMenu->insertItem( actionAddToIgnoreList->text(), this, SLOT(slotAddToIgnoreList()) );
        subMenu->setWhatsThis(id, i18n("<b>Ignore in Subversion operations</b><p>Ignores file(s)."));
		id = subMenu->insertItem( actionRemoveFromIgnoreList->text(), this, SLOT(slotRemoveFromIgnoreList()) );
        subMenu->setWhatsThis(id, i18n("<b>Do not ignore in Subversion operations</b><p>Do not ignore file(s)."));
*/
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
	kdDebug(9036) << "subversion: slotActionUpdate()" << endl;
	KURL doc;
	if (urlFocusedDocument( doc )) {
		m_impl->update( doc );
	}
}

void subversionPart::slotUpdate() {
	m_impl->update (m_urls);
}

void subversionPart::slotActionResolve() {
	kdDebug(9036) << "subversion: slotActionResolve()" << endl;
	KURL doc;
	if (urlFocusedDocument( doc )) {
		m_impl->resolve( doc );
	}
}

void subversionPart::slotResolve() {
	m_impl->resolve (m_urls);
}

void subversionPart::slotSwitch()
{
	if( m_urls.count() > 1 ){
		KMessageBox::error( (QWidget*)project()->mainWindow()->main(),
							i18n("Please select only one item for subversion switch") );
		return;
	}
	if( m_urls.count() < 1 ) return;
	
	// retrieve repository info from local-copy metadata which will be displayed in dialog box
	KURL wcPath = m_urls.first();
	QMap< KURL, SvnGlobal::SvnInfoHolder> holderMap;
	SvnGlobal::SvnInfoHolder holder;
			
	m_impl->clientInfo( wcPath, false, holderMap );
	QValueList< SvnGlobal::SvnInfoHolder > holderList = holderMap.values();
	holder = holderList.first();
	// invoke dialog box
	SvnSwitchDlg dlg( &holder, wcPath.path(), (QWidget*)project()->mainWindow()->main() );
	
	if( dlg.exec() != QDialog::Accepted ){
		return;
	}
	// check target url's validity
	KURL repositUrl = KURL( dlg.destUrl() );
	if( !repositUrl.isValid() ){
		KMessageBox::error( (QWidget*)project()->mainWindow()->main(),
							 i18n("The destination URL is invalid") );
		return;
	}
	// call core
	if( dlg.switchOnly() )
		m_impl->switchTree( wcPath, repositUrl, -1, "HEAD", dlg.recursive() );
	else if( dlg.relocation() )
		m_impl->switchRelocate( wcPath, dlg.currentUrl(), repositUrl, dlg.recursive() );
	else
		KMessageBox::error( (QWidget*)project()->mainWindow()->main(),
							i18n("Fail to conduct subversion switch. No action was selected") );
}

void subversionPart::slotCopy()
{
	// error check
	if( m_urls.count() > 1 ){
		KMessageBox::error( (QWidget*)project()->mainWindow()->main(),
							 i18n("Please select only one item for subversion switch") );
		return;
	}
	if( m_urls.count() < 1 ) return;

	// retrieve repository info from local-copy metadata which will be displayed in dialog box
	KURL wcPath = m_urls.first();
	QMap< KURL, SvnGlobal::SvnInfoHolder> holderMap;
	SvnGlobal::SvnInfoHolder holder;
	m_impl->clientInfo( wcPath, false, holderMap );
	QValueList< SvnGlobal::SvnInfoHolder > holderList = holderMap.values();
	holder = holderList.first();
	// start input dialog
	SvnCopyDialog dlg( wcPath.prettyURL(),
					   &holder,
					   (QWidget*)project()->mainWindow()->main());
	
	if( dlg.exec() != QDialog::Accepted )
		return;
	// retrieve user input
	KURL srcUrl = dlg.sourceUrl();
    int rev = dlg.revision();
    QString revKind = dlg.revKind();
    KURL dest = dlg.destUrl();

	kdDebug(9036) << " SRC: " << srcUrl << " DEST: " << dest << " Revnum: " << rev << " RevKind: " << revKind << endl;

	m_impl->svnCopy( srcUrl, rev, revKind, dest );
}

void subversionPart::slotMerge()
{
	// error check
	if( m_urls.count() > 1 ){
		KMessageBox::error( (QWidget*)project()->mainWindow()->main(),
							 i18n("Please select only one item for subversion merge") );
		return;
	}
	if( m_urls.count() < 1 ) return;

	KURL wcTarget= m_urls.first();
	SvnMergeDialog dlg( wcTarget, (QWidget*)project()->mainWindow()->main() );
	if( dlg.exec() != QDialog::Accepted ) return;

	KURL src1 = dlg.source1();
	SvnRevision rev1 = dlg.rev1();
	KURL src2 = dlg.source2();
	SvnRevision rev2 = dlg.rev2();

	m_impl->merge( src1, rev1.revNum, rev1.revKind, src2, rev2.revNum, rev2.revKind, wcTarget,
				   dlg.recurse(), dlg.ignoreAncestry(), dlg.force(), dlg.dryRun() );
}

void subversionPart::slotActionCommit() {
	kdDebug(9036) << "subversion: slotActionCommit()" << endl;
	KURL doc;
	if (urlFocusedDocument( doc )) {
		m_impl->commit( doc, true, true );
	}
}

void subversionPart::slotActionAdd() {
	kdDebug(9036) << "subversion: slotActionAdd()" << endl;
	KURL doc;
	if (urlFocusedDocument( doc )) {
		m_impl->add( doc );
	}
}

void subversionPart::slotActionDel() {
	kdDebug(9036) << "subversion: slotActionDel()" << endl;
	KURL doc;
	if (urlFocusedDocument( doc )) {
		m_impl->del( doc );
	}
}

void subversionPart::slotActionRevert() {
	kdDebug(9036) << "subversion: slotActionRevert()" << endl;
	KURL doc;
	if (urlFocusedDocument( doc )) {
		m_impl->revert( doc );
	}
}

void subversionPart::slotActionDiffLocal() {
	kdDebug(9036) << "subversion: slotActionDiffLocal()" << endl;
	KURL doc;
	if (urlFocusedDocument( doc )) {
		m_impl->diff( doc, "BASE" );
	}
}
void subversionPart::slotActionDiffHead() {
	kdDebug(9036) << "subversion: slotActionDiffHead()" << endl;
	KURL doc;
	if (urlFocusedDocument( doc )) {
		m_impl->diff( doc, "HEAD" );
	}
}
void subversionPart::slotCommit()
{
	SVNFileSelectDlgCommit dialog( m_urls, this, 0 );
	if( dialog.exec() == QDialog::Accepted ){
		KURL::List tobeCommittedUrls = dialog.checkedUrls();
		bool recursive = dialog.recursive();
		bool keepLocks = dialog.keepLocks();
		m_impl->commit(tobeCommittedUrls, recursive, keepLocks );
	}
}
void subversionPart::slotAdd() {
	m_impl->add( m_urls );
}

void subversionPart::slotLog()
{
	if (m_urls.count() > 1){
		KMessageBox::error( (QWidget*)project()->mainWindow()->main(),
							i18n("Please select only one item for subversion log") );
		return;
	}
	SvnLogViewOptionDlg dlg;
	if( dlg.exec() ){
		int revstart = dlg.revstart();
		QString revkindstart = dlg.revKindStart();
		int revend = dlg.revend();
		QString revkindend = dlg.revKindEnd();
		bool strictNode = dlg.strictNode();
		m_impl->svnLog (m_urls, revstart, revkindstart, revend, revkindend, true/*changedPath*/, strictNode);
	} else{
		return;
	}
}
void subversionPart::slotBlame()
{
	if (m_urls.count() > 1){
		KMessageBox::error( (QWidget*)project()->mainWindow()->main(),
							 i18n("Please select only one item to see annotate") );
		return;
	}
	if (m_urls.count() < 1){
		KMessageBox::error( (QWidget*)project()->mainWindow()->main(),
							 i18n("Select file to see blame") );
		return;
	}
	KURL url = m_urls.first();
	m_impl->blame(url, SvnGlobal::path_to_reposit, 0, "", -1, "BASE");
}

void subversionPart::slotDel() {
	m_impl->del (m_urls);
}

// note: currently diffAsync does not support merging. But svncore::diff()
// cannot be invoked on directory, while diffAsync can.
void subversionPart::slotDiffLocal() {
// 	m_impl->diff (m_urls, "BASE");
	if( m_urls.count() < 1 ){
		// Impossible to reach here but..
		KMessageBox::error( (QWidget*)project()->mainWindow()->main(),
							 i18n("Select file or directory to see diff") );
		return;
	}
	m_impl->diffAsync( *(m_urls.begin()), *(m_urls.begin()), -1, "BASE", -1, "WORKING", true );
}
void subversionPart::slotDiffHead() {
// 	m_impl->diff (m_urls, "HEAD");
	if( m_urls.count() < 1 ){
		// Impossible to reach here but..
		KMessageBox::error( (QWidget*)project()->mainWindow()->main(),
							 i18n("Select file or directory to see diff") );
		return;
	}
	m_impl->diffAsync( *(m_urls.begin()), *(m_urls.begin()), -1, "WORKING", -1, "HEAD", true );
}

void subversionPart::slotRevert() {
	m_impl->revert (m_urls);
}

void subversionPart::slotProjectOpened() {
	kdDebug(9036) << "subversion :projectOpened" << endl;
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
	kdDebug(9036) << "subversion :projectClosed" << endl;
	//saveOptions();
	/// \FIXME slots
	//disconnect( project(), SIGNAL(addedFilesToProject(const QStringList&)), this, SLOT(slotAddFilesToProject(const QStringList &)) );
	//disconnect( project(), SIGNAL(removedFilesFromProject(const QStringList&)), this, SLOT(slotRemovedFilesFromProject(const QStringList &)) );
}

void subversionPart::savePartialProjectSession(QDomElement* dom) {
	kdDebug(9036) << "subversion : savePartialProjectSession" << endl;
	QDomDocument doc = dom->ownerDocument();
	QDomElement svn = doc.createElement( "subversion" );
	svn.setAttribute( "base", base.url() );
	dom->appendChild( svn );
}

void subversionPart::restorePartialProjectSession(const QDomElement* dom) {
	kdDebug(9036) << "subversion : restorePartialProjectSession" << endl;
	QDomElement svn = dom->namedItem("subversion").toElement();
	base = svn.attribute( "base", "" );
}

bool subversionPart::isValidDirectory( const QString &dirPath) const {
    QString svn = "/.svn/";
    QDir svndir( dirPath + svn );
    QString entriesFileName = dirPath + svn + "entries";

	kdDebug(9036) << "dirpath " << dirPath+"/.svn/" << " exists:" << svndir.exists() << endl;
	kdDebug(9036) << "entries " << entriesFileName << " exists:" << QFile::exists( entriesFileName ) << endl;
    return svndir.exists() &&
        QFile::exists( entriesFileName );
}

#include "subversion_part.moc"

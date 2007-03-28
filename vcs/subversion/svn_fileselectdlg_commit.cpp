/*
 *  Copyright (C) 2007 Dukju Ahn (dukjuahn@gmail.com)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */


#include "svn_fileselectdlg_commit.h"
#include "subversion_fileinfo.h"
#include "subversion_part.h"
#include <kurl.h>
#include <qstring.h>
#include <qlistview.h>
#include <qfileinfo.h>
#include <qcheckbox.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include "kdevmainwindow.h"
#include <klocale.h>

#include <kdevproject.h>

SVNFileSelectDlgCommit::SVNFileSelectDlgCommit( KURL::List &urls, subversionPart *part, QWidget* parent)
	:SvnCommitDlgBase( parent, "svnfileselectcommitdlg", true )
	,m_part(part)
{
	this->setCaption(i18n("Select Files to Commit"));
	listView()->clear();
	listView()->setColumnText(0, i18n("select") );  //col 0
	listView()->addColumn( i18n("status") );        //col 1
	listView()->addColumn( i18n("URL to commit") ); //col 2
	listView()->setColumnWidthMode( 2, QListView::Maximum );
	listView()->setSorting( 2, true );
	recursiveChk->setChecked(false);
	keepLocksChk->setChecked(false);
	
	
	const VCSFileInfoMap *vcsMap;
	VCSFileInfo     vcsInfo;
	KURL::List tobeCommittedUrls;

	for( QValueListConstIterator<KURL> it = urls.begin(); it != urls.end(); ++it ){
		KURL oneUrl(*it);
		QFileInfo fileInfo(oneUrl.path());
		//fileInfo.convertToAbs();
		
		if (fileInfo.isFile()){
			KURL base_url( part->project()->projectDirectory()+"/" );
			QString dirPath = KURL::relativeURL( base_url, fileInfo.dirPath(TRUE) );
			vcsMap = ((SVNFileInfoProvider*)part->fileInfoProvider()) ->
                    statusExt(dirPath, false/*repository access*/, true/*recurse*/, false/*getall*/, true/*noIgnore*/);
			vcsInfo = (*vcsMap)[fileInfo.fileName()];
			if( vcsInfo.state == VCSFileInfo::Added || vcsInfo.state == VCSFileInfo::Modified ||
				vcsInfo.state == VCSFileInfo::Deleted || vcsInfo.state == VCSFileInfo::Replaced ){
				
				this->insertItem( VCSFileInfo::state2String( vcsInfo.state ), oneUrl );
// 				tobeCommittedUrls.push_back(oneUrl);
// 				kdDebug(9036) << "slotCommit() : added AS FILE: " << oneUrl.prettyURL() << endl;
				
			}
			else{
				kdDebug(9036) << "slotCommit() @ FileCase ignoring " << oneUrl.prettyURL() << endl;
			}
		}
		else if (fileInfo.isDir()){
			KURL base_url( part->project()->projectDirectory()+"/" );
			QString dirPath = KURL::relativeURL( base_url, fileInfo.absFilePath() );
            vcsMap = ((SVNFileInfoProvider*)part->fileInfoProvider()) ->
                    statusExt(dirPath, false/*repository access*/, true/*recurse*/, false/*getall*/, true/*noIgnore*/);
			for (VCSFileInfoMap::ConstIterator it=vcsMap->begin(); it!=vcsMap->end(); ++it){
				
				vcsInfo = it.data();
//				QString absPathStr( fileInfo.absFilePath() + "/" + it.key() );
				QString absPathStr( fileInfo.filePath() + "/" + it.key() );
				KURL urlFromStatus( absPathStr );
				if( vcsInfo.state == VCSFileInfo::Added || vcsInfo.state == VCSFileInfo::Modified ||
                    vcsInfo.state == VCSFileInfo::Deleted || vcsInfo.state == VCSFileInfo::Replaced){

					this->insertItem( VCSFileInfo::state2String( vcsInfo.state ), urlFromStatus );
					
// 					tobeCommittedUrls.push_back( urlFromStatus );
// 					kdDebug(9036) << "slotCommit() @ DirCase adding " << urlFromStatus.prettyURL() << endl;
				}
				else{
					kdDebug(9036) << "slotCommit() @ DirCase ignoring " << urlFromStatus.prettyURL() << endl;
				}
				
			}
		}
		else if ( !fileInfo.exists() ){
			// maybe deleted files
			this->insertItem( VCSFileInfo::state2String( VCSFileInfo::Deleted ), oneUrl );
		}
	}
	
}
SVNFileSelectDlgCommit::~SVNFileSelectDlgCommit()
{
}

int SVNFileSelectDlgCommit::exec()
{
	if (listView()->childCount() <= 0){
		//TODO if klauncher fails, this spot is also reached. We should show correct error message to user
		KMessageBox::information( (QWidget*)m_part->project()->mainWindow()->main(), i18n("No added/modified/deleted file(s) to commit") );
		return QDialog::Rejected;
	}
	else{
		return SvnCommitDlgBase::exec();
	}
}
void SVNFileSelectDlgCommit::insertItem( QString status, KURL url )
{
	QCheckListItem *item = new QCheckListItem( listView(), "", QCheckListItem::CheckBox );
	item->setText( 1, status );
	item->setText( 2, url.path() );
	item->setOn(true);
}
KURL::List SVNFileSelectDlgCommit::checkedUrls()
{
	QPtrList<QListViewItem> lst;
	QListViewItemIterator it( listView() );
	KURL::List tobeCommittedUrls;
	
	for ( ; it.current() ; ++it ) {
		if ( ((QCheckListItem*)(it.current()))->isOn() ){
			KURL tmpurl( it.current()->text(2) );
			tobeCommittedUrls.push_back( tmpurl );
		}
	}
	return tobeCommittedUrls;
    
}
QListView* SVNFileSelectDlgCommit::listView()
{
	return listView1;
}

bool SVNFileSelectDlgCommit::recursive()
{
	return recursiveChk->isChecked();
}
bool SVNFileSelectDlgCommit::keepLocks()
{
	return keepLocksChk->isChecked();
}

#include "svn_fileselectdlg_commit.moc"


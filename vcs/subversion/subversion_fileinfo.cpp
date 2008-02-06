/**
	 Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>

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

#include "subversion_fileinfo.h"
#include "subversion_core.h"
#include <kdebug.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <kdevproject.h>
#include <unistd.h>
#include <kapplication.h>
#include <kdevmainwindow.h>
#include <kmainwindow.h>
#include <qregexp.h>

#include <kio/netaccess.h>
#include <klocale.h>

SVNFileInfoProvider::SVNFileInfoProvider(subversionPart *parent, const char *name)
    : KDevVCSFileInfoProvider( parent, "svnfileinfoprovider" ),
	m_cachedDirEntries( 0 ), m_recursiveDirEntries(0) {
	Q_UNUSED(name);
	m_part = parent;
}

SVNFileInfoProvider::~SVNFileInfoProvider() {
	delete m_cachedDirEntries;
	m_cachedDirEntries = 0;
	delete m_recursiveDirEntries;
 	m_recursiveDirEntries = 0;
}

//synchronous
const VCSFileInfoMap *SVNFileInfoProvider::status( const QString &dirPath ) {
	if ( !m_cachedDirEntries )
		m_cachedDirEntries = new VCSFileInfoMap;
//	return m_cachedDirEntries;

	kdDebug(9036) << "svn provider : status " << dirPath << endl;

	if ( dirPath != m_previousDirPath ) {
		m_previousDirPath = dirPath;
		KURL servURL = "kdevsvn+http://fakeserver_this_is_normal_behavior/";
		QByteArray parms;
		QDataStream s( parms, IO_WriteOnly );
		int cmd = 9;
		QString rPath = projectDirectory( );
		rPath += QDir::separator() + dirPath;
		kdDebug(9036) << "DIR : " << rPath << " " << KURL( QFileInfo( rPath ).absFilePath() ) << endl;

// 		s << cmd << KURL( QFileInfo( rPath ).absFilePath() ) << true << true; //original line

		// Dukju Ahn: if checkRepos is set, status() accesses remote repository,
		// which causes significant delaym_owner especially when network speed is not fast enough.
		// Of course, the user cannot get information about the out-of-dateness of his local copy.
		s << cmd << KURL( QFileInfo( rPath ).absFilePath() ) << false/*checkRepos*/ << false /*fullRecurse*/;

		KIO::SimpleJob *job2 = KIO::special(servURL, parms, false);
		job2->setWindow( m_part->mainWindow()->main() );


        QMap<QString,QString> ma;

		KIO::NetAccess::synchronousRun(job2, m_part->mainWindow()->main(), 0, 0, &ma );

		QValueList<QString> keys = ma.keys();
		qHeapSort( keys );
		QValueList<QString>::Iterator begin = keys.begin(), end = keys.end(), it;

		QString path;
		int text_status = 0, prop_status = 0, repos_text_status = 0, repos_prop_status = 0;
		long int rev = 0;
		int curIdx, lastIdx;

		QRegExp rx( "([0-9]*)(.*)" );
		for ( it = begin; it != end; ) {
			kdDebug(9036) << "METADATA : " << *it << ":" << ma[ *it ] << endl;
			if ( rx.search( *it ) == -1 ) return m_cachedDirEntries; // something is wrong ! :)
            /* if some notification comes here, consume these notification metadatas */
            if ( rx.cap( 2 ) == "action" ){
                curIdx = lastIdx = rx.cap( 1 ).toInt();
                while ( curIdx == lastIdx ){
                    ++it;
                    if ( it == end ) break;
                    if ( rx.search( *it ) == -1 ) continue; // something is wrong
                    curIdx = rx.cap( 1 ).toInt();
                }
                continue;
            }
			curIdx = lastIdx = rx.cap( 1 ).toInt();
			while ( curIdx == lastIdx ) {
				if ( rx.cap( 2 ) == "path" )
					path = ma[ *it ];
				else if ( rx.cap( 2	 ) == "text" )
					text_status = ma[ *it ].toInt();
				else if ( rx.cap( 2	 ) == "prop" )
					prop_status = ma[ *it ].toInt();
				else if ( rx.cap( 2	 ) == "reptxt" )
					repos_text_status = ma[ *it ].toInt();
				else if ( rx.cap( 2	 ) == "repprop" )
					repos_prop_status = ma[ *it ].toInt();
				else if ( rx.cap( 2	 ) == "rev" )
					rev = ma[ *it ].toLong();
				++it;
                if ( it == end )
                    break;
				if ( rx.search( *it ) == -1 ) break; // something is wrong ! :)
				curIdx = rx.cap( 1 ).toInt();
			}
			slotStatus(path, text_status, prop_status, repos_text_status, repos_prop_status, rev);
		}
	}
	kdDebug(9036) << " Returning VcsFileInfoMap. provider::status() finished " << endl;
    return m_cachedDirEntries;
}

bool SVNFileInfoProvider::requestStatus( const QString &dirPath, void *callerData, bool recursive, bool checkRepos ) {
	kdDebug(9036) << "##################################################################################### svn provider : request status" << endl;
    m_savedCallerData = callerData;
    // Flush old cache
    if (m_cachedDirEntries)
    {
        delete m_cachedDirEntries;
        m_cachedDirEntries = 0;
        m_previousDirPath = dirPath;
    }

	QByteArray parms;
	QDataStream s( parms, IO_WriteOnly );
	int cmd = 9;
	QString rPath = projectDirectory( );
	rPath += QDir::separator() + dirPath;
    
    if( ! m_part->isValidDirectory( rPath ) ){
        return false;
    }
    
	kdDebug(9036) << "DIR : " << rPath << " " << QFileInfo( rPath ).absFilePath() << endl;
	s << cmd << KURL( QFileInfo( rPath ).absFilePath() ) << checkRepos << recursive;
	KURL servURL = "kdevsvn+http://fakeserver_this_is_normal_behavior/";
	job = KIO::special(servURL, parms, false);
	connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotResult( KIO::Job * ) ) );
	if( checkRepos )
		m_part->svncore()->initProcessDlg( job, dirPath, i18n("Subversion File/Directory Status") );
    return true;
}

void SVNFileInfoProvider::slotResult( KIO::Job *j ) {
	if ( j->error() )
		j->showErrorDialog( m_part->mainWindow()->main() );

	KIO::MetaData ma = j->metaData();
	QValueList<QString> keys = ma.keys();
	qHeapSort( keys );
	QValueList<QString>::Iterator begin = keys.begin(), end = keys.end(), it;

	QString path;
	int text_status = 0, prop_status = 0, repos_text_status = 0, repos_prop_status = 0;
	long int rev = 0;
	int curIdx, lastIdx;

	QRegExp rx( "([0-9]*)(.*)" );
	for ( it = begin; it != end; ) {
		kdDebug(9036) << "METADATA : " << *it << ":" << ma[ *it ] << endl;
		if ( rx.search( *it ) == -1 ) return; // something is wrong ! :)
        /* if some notification comes here, consume these notification metadatas */
        if ( rx.cap( 2 ) == "action" ){
            curIdx = lastIdx = rx.cap( 1 ).toInt();
            while ( curIdx == lastIdx ){
                ++it;
                if ( it == end ) break;
                if ( rx.search( *it ) == -1 ) continue; // something is wrong
                curIdx = rx.cap( 1 ).toInt();
            }
            continue;
        }
		curIdx = lastIdx = rx.cap( 1 ).toInt();
		while ( curIdx == lastIdx ) {
			if ( rx.cap( 2 ) == "path" )
				path = ma[ *it ];
			else if ( rx.cap( 2	 ) == "text" )
				text_status = ma[ *it ].toInt();
			else if ( rx.cap( 2	 ) == "prop" )
				prop_status = ma[ *it ].toInt();
			else if ( rx.cap( 2	 ) == "reptxt" )
				repos_text_status = ma[ *it ].toInt();
			else if ( rx.cap( 2	 ) == "repprop" )
				repos_prop_status = ma[ *it ].toInt();
			else if ( rx.cap( 2	 ) == "rev" )
				rev = ma[ *it ].toLong();
			++it;
            if ( it == end )
                break;
			if ( rx.search( *it ) == -1 ) break; // something is wrong ! :)
			curIdx = rx.cap( 1 ).toInt();
		}
		slotStatus(path, text_status, prop_status, repos_text_status, repos_prop_status, rev);
	}

	if ( m_cachedDirEntries )
		emit statusReady(*m_cachedDirEntries, m_savedCallerData);
}

void SVNFileInfoProvider::slotStatus( const QString& path,int text_status, int prop_status,int repos_text_status, int repos_prop_status, long int rev) {
// 	kdDebug(9036) << "##################################################################################### svn provider : slotstatus"
// 		<< " path " << path << " text_status " << text_status << " prop_status " << prop_status << " repos_text_status " << repos_text_status
// 		<< " repos_prop_status " << repos_prop_status << " rev " << rev
// 		<< endl;

	if ( !m_cachedDirEntries )
		m_cachedDirEntries = new VCSFileInfoMap;

	QString wRev = QString::number( rev ); //work rev
	QString rRev = QString::number( rev );// repo rev
	VCSFileInfo::FileState state = VCSFileInfo::Unknown;

	switch ( text_status ) {
		case 1:
			break;
		case 2:
			break;
		case 3:
			state = VCSFileInfo::Uptodate;
			break;
		case 4:
			state = VCSFileInfo::Added;
			break;
		case 5:
			break;
		case 6: //deleted
			state = VCSFileInfo::Deleted;
			break;
		case 7: //replaced
            state = VCSFileInfo::Replaced;
			break;
		case 8: //modified
			state = VCSFileInfo::Modified;
			break;
		case 9: //merged
			break;
		case 10: //conflicted
			state = VCSFileInfo::Conflict;
			break;
		case 11: //ignored
			break;
		case 12: //obstructed
			break;
		case 13: //external
			break;
		case 14: //incomplete
			break;
	}
	switch( prop_status ) {
		case 8:
			state = VCSFileInfo::Modified;
			break;
	}
	switch ( repos_text_status ) {
		case 1:
			break;
		case 2:
			break;
		case 3:
			break;
		case 4:
			break;
		case 5:
			break;
		case 6: //deleted
			break;
		case 7: //replaced
			break;
		case 8: //modified
			state = VCSFileInfo::NeedsPatch;
			break;
		case 9: //merged
			break;
		case 10: //conflicted
			break;
		case 11: //ignored
			break;
		case 12: //obstructed
			break;
		case 13: //external
			break;
		case 14: //incomplete
			break;
	}

	VCSFileInfo info(QFileInfo( path ).fileName(),wRev,rRev,state);
	kdDebug(9036) << "Inserting " << info.toString() << endl;
	m_cachedDirEntries->insert( QFileInfo( path ).fileName(), info);
}

QString SVNFileInfoProvider::projectDirectory() const {
	return owner()->project()->projectDirectory();
}

const VCSFileInfoMap *SVNFileInfoProvider::statusExt( const QString &dirPath,
	bool checkRepos, bool fullRecurse, bool getAll, bool noIgnore )
{
	if ( !m_recursiveDirEntries )
		m_recursiveDirEntries = new VCSFileInfoMap;

// 	if ( dirPath != m_recursivePreviousDirPath ) {
	m_recursiveDirEntries->clear();
	m_recursivePreviousDirPath = dirPath;
	KURL servURL = "kdevsvn+http://fakeserver_this_is_normal_behavior/";
	QByteArray parms;
	QDataStream s( parms, IO_WriteOnly );
	int cmd = 109;
	QString rPath = projectDirectory( );
	rPath += QDir::separator() + dirPath;
	kdDebug(9036) << "DIR : " << rPath << " " << KURL( QFileInfo( rPath ).absFilePath() ) << endl;
	s << cmd << checkRepos << fullRecurse << getAll << noIgnore << -1 << "WORKING" << KURL( QFileInfo( rPath ).absFilePath() );
	KIO::SimpleJob *job2 = KIO::special(servURL, parms, false);
	job2->setWindow( m_part->mainWindow()->main() );


	QMap<QString,QString> ma;
	KIO::NetAccess::synchronousRun(job2, m_part->mainWindow()->main(), 0, 0, &ma );

	QValueList<QString> keys = ma.keys();
	qHeapSort( keys );
	QValueList<QString>::Iterator begin = keys.begin(), end = keys.end(), it;

	QString path;
	int text_status = 0, prop_status = 0, repos_text_status = 0, repos_prop_status = 0;
	long int rev = 0;
	int curIdx, lastIdx;

	QRegExp rx( "([0-9]*)(.*)" );
	for ( it = begin; it != end; ) {
		kdDebug(9036) << "METADATA : " << *it << ":" << ma[ *it ] << endl;
		if ( rx.search( *it ) == -1 ) return m_recursiveDirEntries; // something is wrong ! :)
		/* if some notification comes here, consume these notification metadatas */
		if ( rx.cap( 2 ) == "action" ){
			curIdx = lastIdx = rx.cap( 1 ).toInt();
			while ( curIdx == lastIdx ){
				++it;
				if ( it == end ) break;
				if ( rx.search( *it ) == -1 ) continue; // something is wrong
				curIdx = rx.cap( 1 ).toInt();
			}
			continue;
		}
		/* get properties */
		curIdx = lastIdx = rx.cap( 1 ).toInt();
		while ( curIdx == lastIdx ) {
			if ( rx.cap( 2 ) == "path" )
				path = ma[ *it ];
			else if ( rx.cap( 2	 ) == "text" )
				text_status = ma[ *it ].toInt();
			else if ( rx.cap( 2	 ) == "prop" )
				prop_status = ma[ *it ].toInt();
			else if ( rx.cap( 2	 ) == "reptxt" )
				repos_text_status = ma[ *it ].toInt();
			else if ( rx.cap( 2	 ) == "repprop" )
				repos_prop_status = ma[ *it ].toInt();
			else if ( rx.cap( 2	 ) == "rev" )
				rev = ma[ *it ].toLong();
			++it;
			if ( it == end )
				break;
			if ( rx.search( *it ) == -1 ) break; // something is wrong ! :)
			curIdx = rx.cap( 1 ).toInt();
		}
		slotStatusExt(dirPath, path, text_status, prop_status, repos_text_status, repos_prop_status, rev);
	}
// 	}

	return m_recursiveDirEntries;
}

void SVNFileInfoProvider::slotStatusExt(
	const QString& reqPath, const QString& path,int text_status, int prop_status,int repos_text_status, int repos_prop_status, long int rev)
{

	if ( !m_recursiveDirEntries )
		m_recursiveDirEntries = new VCSFileInfoMap;

	QString wRev = QString::number( rev ); //work rev
	QString rRev = QString::number( rev );// repo rev
	VCSFileInfo::FileState state = VCSFileInfo::Unknown;

	switch ( text_status ) {
		case 1: // does not exist
			break;
		case 2: // unversioned
			break;
		case 3:
			state = VCSFileInfo::Uptodate;
			break;
		case 4:
			state = VCSFileInfo::Added;
			break;
		case 5: // missing
			break;
		case 6: //deleted
			state = VCSFileInfo::Deleted;
			break;
		case 7: //replaced
			state = VCSFileInfo::Replaced;
			break;
		case 8: //modified
			state = VCSFileInfo::Modified;
			break;
		case 9: //merged
			break;
		case 10: //conflicted
			state = VCSFileInfo::Conflict;
			break;
		case 11: //ignored
			break;
		case 12: //obstructed
			break;
		case 13: //external
			break;
		case 14: //incomplete
			break;
	}
	switch( prop_status ) {
		case 8:
			state = VCSFileInfo::Modified;
			break;
	}
	switch ( repos_text_status ) {
		case 1:
			break;
		case 2:
			break;
		case 3:
			break;
		case 4:
			break;
		case 5:
			break;
		case 6: //deleted
			break;
		case 7: //replaced
			break;
		case 8: //modified
			state = VCSFileInfo::NeedsPatch;
			break;
		case 9: //merged
			break;
		case 10: //conflicted
			break;
		case 11: //ignored
			break;
		case 12: //obstructed
			break;
		case 13: //external
			break;
		case 14: //incomplete
			break;
	}

	QString relativeReqPath;
	if (reqPath == "./"){
		// case of project top directory
		QString reqAbsPath = projectDirectory();

        if( path == reqAbsPath ){
            //key of VCSInfo is project directory itself. So it is set to .
            relativeReqPath = ".";
        }
        else{
            relativeReqPath = path.right( path.length() - reqAbsPath.length() - 1 );
        }
	}
	else {
		QString reqAbsPath = projectDirectory() + QDir::separator() + reqPath;
		relativeReqPath = path.right( path.length() - reqAbsPath.length() - 1 );

		if (relativeReqPath == reqAbsPath){
			// case of requested directory itself.
			relativeReqPath = ".";
		}
	}

	VCSFileInfo info(relativeReqPath, wRev, rRev, state);
	m_recursiveDirEntries->insert( relativeReqPath, info );

// 	VCSFileInfo info(QFileInfo( path ).fileName(),wRev,rRev,state);
 	kdDebug(9036) << "Inserting " << info.toString() << endl;
// 	m_recursiveDirEntries->insert( QFileInfo( path ).fileName(), info);
}

#include "subversion_fileinfo.moc"


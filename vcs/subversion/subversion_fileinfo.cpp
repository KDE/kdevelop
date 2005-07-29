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
	 the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
	 Boston, MA 02110-1301, USA.
	 */

#include "subversion_fileinfo.h"
#include <kdebug.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <kdevproject.h>
#include <unistd.h>
#include <kapplication.h>
#include <kdevmainwindow.h>
#include <kmainwindow.h>
#include <qregexp.h>

#if KDE_VERSION <= KDE_MAKE_VERSION(3,3,90)
#include "../compat/netaccess/netaccess.h"
#else
#include <kio/netaccess.h>
#endif

SVNFileInfoProvider::SVNFileInfoProvider(subversionPart *parent, const char *name)
    : KDevVCSFileInfoProvider( parent, "svnfileinfoprovider" ),
	m_cachedDirEntries( 0 ) {
  Q_UNUSED(name);
	m_part = parent;
}

SVNFileInfoProvider::~SVNFileInfoProvider() {
	delete m_cachedDirEntries;
}

//synchronous
const VCSFileInfoMap *SVNFileInfoProvider::status( const QString &dirPath ) {
	if ( !m_cachedDirEntries )
		m_cachedDirEntries = new VCSFileInfoMap;
//	return m_cachedDirEntries;

	kdDebug(9036) << "##################################################################################### svn provider : status " << dirPath << endl;

	if ( dirPath != m_previousDirPath ) {
		m_previousDirPath = dirPath;
		KURL servURL = "svn+http://fakeserver_this_is_normal_behavior/";
		QByteArray parms;
		QDataStream s( parms, IO_WriteOnly );
		int cmd = 9;
		QString rPath = projectDirectory( );
		rPath += QDir::separator() + dirPath;
		kdDebug(9036) << "DIR : " << rPath << " " << KURL( QFileInfo( rPath ).absFilePath() ) << endl;
		s << cmd << KURL( QFileInfo( rPath ).absFilePath() ) << true << true;
		KIO::SimpleJob *job2 = KIO::special(servURL, parms, false);
		job2->setWindow( m_part->mainWindow()->main() );


		QMap<QString,QString> ma;
#if KDE_VERSION <= KDE_MAKE_VERSION(3,3,90)
		KIO_COMPAT::NetAccess::synchronousRun(job2, m_part->mainWindow()->main(), 0, 0, &ma );
#else
		KIO::NetAccess::synchronousRun(job2, m_part->mainWindow()->main(), 0, 0, &ma );
#endif

		QValueList<QString> keys = ma.keys();
		qHeapSort( keys );
		QValueList<QString>::Iterator begin = keys.begin(), end = keys.end(), it;

		QString path;
		int text_status, prop_status, repos_text_status, repos_prop_status;
		long int rev;
		int curIdx, lastIdx;

		QRegExp rx( "([0-9]*)(.*)" );
		for ( it = begin; it != end; ) {
			kdDebug(9036) << "METADATA : " << *it << ":" << ma[ *it ] << endl;
			if ( rx.search( *it ) == -1 ) return m_cachedDirEntries; // something is wrong ! :)
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
				if ( rx.search( *it ) == -1 ) break; // something is wrong ! :)
				curIdx = rx.cap( 1 ).toInt(); 
			}
			slotStatus(path, text_status, prop_status, repos_text_status, repos_prop_status, rev);
		}
	}

    return m_cachedDirEntries;
}

bool SVNFileInfoProvider::requestStatus( const QString &dirPath, void *callerData ) {
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
	kdDebug(9036) << "DIR : " << rPath << " " << QFileInfo( rPath ).absFilePath() << endl;
	s << cmd << KURL( QFileInfo( rPath ).absFilePath() ) << true << true;
	KURL servURL = "svn+http://fakeserver_this_is_normal_behavior/";
	job = KIO::special(servURL, parms, false);
	job->setWindow( m_part->mainWindow()->main() );
	connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotResult( KIO::Job * ) ) );

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
	int text_status, prop_status, repos_text_status, repos_prop_status;
	long int rev;
	int curIdx, lastIdx;

	QRegExp rx( "([0-9]*)(.*)" );
	for ( it = begin; it != end; ) {
		kdDebug(9036) << "METADATA : " << *it << ":" << ma[ *it ] << endl;
		if ( rx.search( *it ) == -1 ) return; // something is wrong ! :)
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
			if ( rx.search( *it ) == -1 ) break; // something is wrong ! :)
			curIdx = rx.cap( 1 ).toInt(); 
		}
		slotStatus(path, text_status, prop_status, repos_text_status, repos_prop_status, rev);
	}

	if ( m_cachedDirEntries )
		emit statusReady(*m_cachedDirEntries, m_savedCallerData);
}

void SVNFileInfoProvider::slotStatus( const QString& path,int text_status, int prop_status,int repos_text_status, int repos_prop_status, long int rev) {
	kdDebug(9036) << "##################################################################################### svn provider : slotstatus" 
		<< " path " << path << " text_status " << text_status << " prop_status " << prop_status << " repos_text_status " << repos_text_status
		<< " repos_prop_status " << repos_prop_status << " rev " << rev
		<< endl;

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
			break;
		case 7: //replaced
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


#include "subversion_fileinfo.moc"

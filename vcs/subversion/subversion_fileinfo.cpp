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
#include <kdebug.h>
#include <kio/netaccess.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <kdevproject.h>
#include <unistd.h>
#include <kapplication.h>

SVNFileInfoProvider::SVNFileInfoProvider(subversionPart *parent, const char *name)
    : KDevVCSFileInfoProvider( parent, "svnfileinfoprovider" ),
	m_cachedDirEntries( 0 ) {
	if ( !connectDCOPSignal("kded","ksvnd","subversionStatus(QString,int,int,int,int,long int)", "slotStatus(QString,int,int,int,int,long int)", false) )
		kdWarning() << "Could not connect to KDED DCOP signal, subversion file status monitoring will not work ! " << endl;
	m_part = parent;
}

SVNFileInfoProvider::~SVNFileInfoProvider() {
/*	if ( job )
		job->kill();*/
	delete m_cachedDirEntries;
}

//synchronous
const VCSFileInfoMap *SVNFileInfoProvider::status( const QString &dirPath ) const {
	if ( !m_cachedDirEntries )
		m_cachedDirEntries = new VCSFileInfoMap;
	return m_cachedDirEntries;
	/*
	kdDebug() << "##################################################################################### svn provider : status " << dirPath << endl;

	if ( dirPath != m_previousDirPath ) {
		m_previousDirPath = dirPath;
		KURL servURL = "svn+http://fakeserver_this_is_normal_behavior/";
		QByteArray parms;
		QDataStream s( parms, IO_WriteOnly );
		int cmd = 9;
		QString rPath = projectDirectory( );
		rPath += QDir::separator() + dirPath;
		kdDebug() << "DIR : " << rPath << " " << KURL( QFileInfo( rPath ).absFilePath() ) << endl;
		s << cmd << QFileInfo( rPath ).absFilePath();
		KIO::SimpleJob *job2 = KIO::special(servURL, parms, false);
		//	job->setWindow( m_part->mainWindow()->main() );
		//	connect( job2, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotResult( KIO::Job * ) ) );

		KIO::NetAccess::synchronousRun(job2, 0);
	}

    return m_cachedDirEntries;*/
}

bool SVNFileInfoProvider::requestStatus( const QString &dirPath, void *callerData ) {
	kdDebug() << "##################################################################################### svn provider : request status" << endl;
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
	kdDebug() << "DIR : " << rPath << " " << QFileInfo( rPath ).absFilePath() << endl;
	s << cmd << KURL( QFileInfo( rPath ).absFilePath() );
	KURL servURL = "svn+http://fakeserver_this_is_normal_behavior/";
	job = KIO::special(servURL, parms, false);
//	job->setWindow( m_part->mainWindow()->main() );
	connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotResult( KIO::Job * ) ) );

    return true;
}

void SVNFileInfoProvider::slotResult( KIO::Job *j ) {
	if ( j->error() )
		j->showErrorDialog( /*m_part->mainWindow()->main()*/ );

	if ( m_cachedDirEntries )
		emit statusReady(*m_cachedDirEntries, m_savedCallerData);
}

void SVNFileInfoProvider::slotStatus( const QString& path,int text_status, int prop_status,int repos_text_status, int repos_prop_status, long int rev) {
	kdDebug() << "##################################################################################### svn provider : slotstatus" 
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
	
	VCSFileInfo info(QFileInfo( path ).fileName(),wRev,rRev,state);
	kdDebug() << "Inserting " << info.toString() << endl;
	m_cachedDirEntries->insert( QFileInfo( path ).fileName(), info);
}

QString SVNFileInfoProvider::projectDirectory() const {
	return owner()->project()->projectDirectory();
}


#include "subversion_fileinfo.moc"

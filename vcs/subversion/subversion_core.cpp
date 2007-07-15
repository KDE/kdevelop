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

#include <kparts/part.h>
#include <kdevcore.h>
#include <kdevproject.h>
#include "subversion_part.h"
#include "subversion_core.h"
#include "subversion_widget.h"
#include "svn_blamewidget.h"
#include "svn_logviewwidget.h"
#include "subversiondiff.h"
#include <kdevmainwindow.h>
#include "svn_co.h"
#include <kurlrequester.h>
#include <klineedit.h>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <kio/netaccess.h>
#include <kdebug.h>
#include <kmainwindow.h>
#include <kapplication.h>
#include <dcopclient.h>
#include <ktempfile.h>
#include <kprocess.h>
#include <kstandarddirs.h>
#include <qtextcodec.h>
#include <qtextstream.h>
#include <qtextbrowser.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <qregexp.h>

#include <kapplication.h>
#include <kinstance.h>
#include <kaboutdata.h>

using namespace KIO;
using namespace SvnGlobal;

subversionCore::subversionCore(subversionPart *part)
// 	: QObject(NULL, "subversion core"), DCOPObject("subversion") {
	: QObject(NULL, "subversion core") {
		m_part = part;
		m_widget = new subversionWidget(part, 0 , "subversionprocesswidget");
// 		m_logViewWidget = new SvnLogViewWidget( part, 0 );
// 		m_part->mainWindow()->embedOutputView( m_logViewWidget, i18n( "Subversion Log" ), i18n( "Subversion Log" ) );
//		if ( ! connectDCOPSignal("kded", "ksvnd", "subversionNotify(QString,int,int,QString,int,int,long int,QString)", "notification(QString,int,int,QString,int,int,long int,QString)", false ) )
//			kdWarning() << "Failed to connect to kded dcop signal ! Notifications won't work..." << endl;

        m_fileInfoProvider = new SVNFileInfoProvider( part );
		diffTmpDir = new KTempDir();
		diffTmpDir->setAutoDelete(true);
}

subversionCore::~subversionCore() {
	if ( processWidget() ) {
		m_part->mainWindow()->removeView( processWidget() );
		delete processWidget();
	}
// 	if( m_logViewWidget ){
// 		m_part->mainWindow()->removeView( m_logViewWidget );
// 		delete m_logViewWidget;
// 	}
	delete diffTmpDir;
	//FIXME delete m_fileInfoProvider here?
}

KDevVCSFileInfoProvider *subversionCore::fileInfoProvider() const {
    return m_fileInfoProvider;
}

//not used anymore
// void subversionCore::notification( const QString& path, int action, int kind, const QString& mime_type, int content_state ,int prop_state ,long int revision, const QString& userstring ) {
// 	kdDebug(9036) << "Subversion Notification : "
// 		<< "path : " << path
// 		<< "action: " << action
// 		<< "kind : " << kind
// 		<< "mime_type : " << mime_type
// 		<< "content_state : " << content_state
// 		<< "prop_state : " << prop_state
// 		<< "revision : " << revision
// 		<< "userstring : " << userstring
// 		<< endl;
// 	if ( !userstring.isEmpty() ) {
// 		m_part->mainWindow()->raiseView(processWidget());
// 		processWidget()->append( userstring );
// 	}
// }

subversionWidget *subversionCore::processWidget() const {
// SvnLogViewWidget* subversionCore::processWidget() const {
// 	return processWidget();
// 	return m_logViewWidget;
    return m_widget;
}

void subversionCore::resolve( const KURL::List& list ) {
	KURL servURL = m_part->baseURL();
	if ( servURL.isEmpty() ) servURL="kdevsvn+svn://blah/";
	if ( ! servURL.protocol().startsWith( "kdevsvn+" ) ) {
		servURL.setProtocol( "kdevsvn+" + servURL.protocol() ); //make sure it starts with "svn"
	}
	kdDebug(9036) << "servURL : " << servURL.prettyURL() << endl;
	for ( QValueListConstIterator<KURL> it = list.begin(); it != list.end() ; ++it ) {
		kdDebug(9036) << "resolving: " << (*it).prettyURL() << endl;
		QByteArray parms;
		QDataStream s( parms, IO_WriteOnly );
		int cmd = 11;
		bool recurse = true;
		s << cmd << *it << recurse;
		SimpleJob * job = KIO::special(servURL, parms, true);
		job->setWindow( m_part->mainWindow()->main() );
		connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotResult( KIO::Job * ) ) );
	}
}

void subversionCore::update( const KURL::List& list ) {
	KURL servURL = "kdevsvn+svn://blah/";
	kdDebug(9036) << "Updating. servURL : " << servURL.prettyURL() << endl;
	
	QByteArray parms;
	QDataStream s( parms, IO_WriteOnly );
	int cmd = 2;
	int rev = -1;
	s << cmd << list << rev << QString( "HEAD" );
	
	SimpleJob * job = KIO::special(servURL, parms, false);
	connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotResult( KIO::Job * ) ) );
	initProcessDlg( (KIO::Job*)job, i18n("Subversion Update") , i18n("Subversion Update") );
}

void subversionCore::diff( const KURL::List& list, const QString& where){
	kdDebug(9036) << "diff " << list << endl;
	KURL servURL = "kdevsvn+svn://this_is_a_fake_URL_and_this_is_normal/";
	for ( QValueListConstIterator<KURL> it = list.begin(); it != list.end() ; ++it ) {
		QByteArray parms;
		QDataStream s( parms, IO_WriteOnly );
		int cmd = 13;
		kdDebug(9036) << "diffing : " << (*it).prettyURL() << endl;
		int rev1=-1;
		int rev2=-1;
		QString revkind1 = where;
		QString revkind2 = "WORKING";
		s << cmd << *it << *it << rev1 << revkind1 << rev2 << revkind2 << true ;
		KIO::SimpleJob * job = KIO::special(servURL, parms, true);
		connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotResult( KIO::Job * ) ) );
		KIO::NetAccess::synchronousRun( job, 0 );
		if ( diffresult.count() > 0 ) {
			//check kompare is available
			if ( !KStandardDirs::findExe( "kompare" ).isNull() ) {
				if (!KStandardDirs::findExe("patch").isNull()){
					// we have patch - so can merge
					KTempDir tmpDir = KTempDir(diffTmpDir->name());
					KTempFile tmpPatch = KTempFile(tmpDir.name());

					// write the patch
					QTextStream *stream = tmpPatch.textStream();
					stream->setCodec( QTextCodec::codecForName( "utf8" ) );
					for ( QStringList::Iterator it2 = diffresult.begin();it2 != diffresult.end() ; ++it2 ) {
						( *stream ) << ( *it2 ) << "\n";
					}
					tmpPatch.close();

					QString ourCopy = tmpDir.name()+(*it).fileName();

					KProcess copy;
					copy << "cp" << (*it).prettyURL(0,KURL::StripFileProtocol) <<  tmpDir.name();
					copy.start(KProcess::Block);

					KProcess patch;
					patch.setWorkingDirectory(tmpDir.name());
					patch << "patch" << "-R" << ourCopy << tmpPatch.name();
					patch.start(KProcess::Block, KProcess::All);

					KProcess *p = new KProcess;
					*p << "kompare" << ourCopy << (*it).prettyURL();
					p->start();
				}
				else{
					// only diff
					KTempFile *tmp = new KTempFile;
					tmp->setAutoDelete(true);
					QTextStream *stream = tmp->textStream();
					stream->setCodec( QTextCodec::codecForName( "utf8" ) );
					for ( QStringList::Iterator it2 = diffresult.begin();it2 != diffresult.end() ; ++it2 ) {
						( *stream ) << ( *it2 ) << "\n";
					}
					tmp->close();
					KProcess *p = new KProcess;
					*p << "kompare" << "-n" << "-o" << tmp->name();
					p->start();
				}
			} else { //else do it with message box
				Subversion_Diff df;
				for ( QStringList::Iterator it2 = diffresult.begin();it2 != diffresult.end() ; ++it2 ) {
					df.text->append( *it2 );
				}
				QFont f = df.font();
				f.setFixedPitch( true );
				df.text->setFont( f );
				df.exec();
			}
		}
		else{
			QString diffTo = i18n("the local disk checked out copy.");
			if ( where=="HEAD"){
				diffTo=i18n("the current svn HEAD version.");
			}
			KMessageBox::information( 0, i18n("No differences between the file and %1").arg(diffTo), i18n("No difference") );
		}
		diffresult.clear();
	}
}

void subversionCore::diffAsync( const KURL &pathOrUrl1, const KURL &pathOrUrl2,
							int rev1, QString revKind1, int rev2, QString revKind2,
							bool recurse, bool pegdiff )
{
	KURL servURL = "kdevsvn+svn://blah/";
	QByteArray parms;
	QDataStream s( parms, IO_WriteOnly );
	int cmd = 13;
	kdDebug(9036) << "diffing async : " << pathOrUrl1 << " and " << pathOrUrl2 << endl;
	s << cmd << pathOrUrl1 << pathOrUrl2 << rev1 << revKind1 << rev2 << revKind2 << recurse;
	s << pegdiff;
	KIO::SimpleJob * job = KIO::special(servURL, parms, false);
	connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotDiffResult( KIO::Job * ) ) );
	initProcessDlg( (KIO::Job*)job, pathOrUrl1.prettyURL(), pathOrUrl2.prettyURL() );
}

void subversionCore::commit( const KURL::List& list, bool recurse, bool keeplocks ) {
	KURL servURL = m_part->baseURL();
	if ( servURL.isEmpty() ) servURL="kdevsvn+svn://blah/";
	if ( ! servURL.protocol().startsWith( "kdevsvn+" ) ) {
		servURL.setProtocol( "kdevsvn+" + servURL.protocol() ); //make sure it starts with "svn"
	}
	kdDebug(9036) << "servURL : " << servURL.prettyURL() << endl;
	QByteArray parms;
	QDataStream s( parms, IO_WriteOnly );
	int cmd = 103;
 	s << cmd << recurse << keeplocks;
	for ( QValueListConstIterator<KURL> it = list.begin(); it != list.end() ; ++it ) {
		kdDebug(9036) << "adding to list: " << (*it).prettyURL() << endl;
		s << *it;
	}
	SimpleJob * job = KIO::special(servURL, parms, false);
	connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotResult( KIO::Job * ) ) );
	if( list.count() == 1 )
		initProcessDlg( (KIO::Job*)job, (*(list.begin())).prettyURL() , i18n("Commit to remote repository") );
	else if( list.count() > 1 )
		initProcessDlg( (KIO::Job*)job, i18n("From working copy") , i18n("Commit to remote repository") );
}
// Right now, only one item for each action.
void subversionCore::svnLog( const KURL::List& list,
		int revstart, QString revKindStart, int revend, QString revKindEnd,
		bool discorverChangedPath, bool strictNodeHistory )
{
	// ensure that part has repository information. This info is used to retrieve root repository URL
    if( m_part->m_prjInfoMap.count() < 1 )
        clientInfo( KURL(m_part->project()->projectDirectory()), false, m_part->m_prjInfoMap );
	KURL servURL = m_part->baseURL();
	if ( servURL.isEmpty() ) servURL="kdevsvn+svn://blah/";
	if ( ! servURL.protocol().startsWith( "kdevsvn+" ) ) {
		servURL.setProtocol( "kdevsvn+" + servURL.protocol() ); //make sure it starts with "svn"
	}
	kdDebug(9036) << "servURL : " << servURL.prettyURL() << endl;
	QByteArray parms;
	QDataStream s( parms, IO_WriteOnly );
	// prepare arguments
	int cmd = 4;
// 	int revstart = -1, revend = 0;
// 	QString revKindStart = "HEAD", revKindEnd = "";
// 	bool repositLog = true, discorverChangedPath = true, strictNodeHistory = true;
	s << cmd << revstart << revKindStart << revend << revKindEnd;
	s << discorverChangedPath << strictNodeHistory;
	for ( QValueListConstIterator<KURL> it = list.begin(); it != list.end() ; ++it ) {
		kdDebug(9036) << "svnCore: adding to list: " << (*it).prettyURL() << endl;
		s << *it;
	}
	SimpleJob * job = KIO::special(servURL, parms, false);
	connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotLogResult( KIO::Job * ) ) );
	//  progress info. LogView is allowed and meaninful only for one url in KDev3.4
	initProcessDlg( (KIO::Job*)job, (*(list.begin())).prettyURL() , i18n("Subversion Log View") );
}

void subversionCore::blame( const KURL &url, UrlMode mode, int revstart, QString revKindStart, int revend, QString revKindEnd )
{
	KURL servURL = m_part->baseURL();
	if ( servURL.isEmpty() ) servURL="kdevsvn+svn://blah/";
	if ( ! servURL.protocol().startsWith( "kdevsvn+" ) ) {
		servURL.setProtocol( "kdevsvn+" + servURL.protocol() ); //make sure it starts with "svn"
	}
	kdDebug(9036) << "servURL : " << servURL.prettyURL() << endl;
	QByteArray parms;
	QDataStream s( parms, IO_WriteOnly );
	// prepare arguments
	int cmd = 14;
	s << cmd << url << (int)mode ;
	s << revstart << revKindStart << revend << revKindEnd ;

	SimpleJob * job = KIO::special(servURL, parms, false);
	connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotBlameResult( KIO::Job * ) ) );
	initProcessDlg( (KIO::Job*)job, url.prettyURL() , i18n("Subversion Blame") );
}

void subversionCore::add( const KURL::List& list ) {
	
	KURL servURL = "kdevsvn+svn://blah/";
	kdDebug(9036) << "Deleting servURL : " << servURL.prettyURL() << endl;
	
	QByteArray parms;
	QDataStream s( parms, IO_WriteOnly );
	int cmd = 6;
	s << cmd << list;
	// add/delete/revert works on local copy. Don't need to show progress dialog
	SimpleJob * job = KIO::special(servURL, parms, false); 
	job->setWindow( m_part->mainWindow()->main() );
	connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotResult( KIO::Job * ) ) );
}

void subversionCore::del( const KURL::List& list ) {
	KURL servURL = "kdevsvn+svn://blah/";
	kdDebug(9036) << "Deleting servURL : " << servURL.prettyURL() << endl;
	
	QByteArray parms;
	QDataStream s( parms, IO_WriteOnly );
	int cmd = 7;
	s << cmd << list;
	// add/delete/revert works on local copy. Don't need to show progress dialog
	SimpleJob * job = KIO::special(servURL, parms, false);
	job->setWindow( m_part->mainWindow()->main() );
	connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotResult( KIO::Job * ) ) );
}

void subversionCore::revert( const KURL::List& list ) {
	KURL servURL = "kdevsvn+svn://blah/";
	kdDebug(9036) << "Reverting servURL : " << servURL.prettyURL() << endl;
	
	QByteArray parms;
	QDataStream s( parms, IO_WriteOnly );
	int cmd = 8;
	s << cmd << list;
	SimpleJob * job = KIO::special(servURL, parms, false);
	job->setWindow( m_part->mainWindow()->main() );
	connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotResult( KIO::Job * ) ) );
}

void subversionCore::checkout() {
	svn_co checkoutDlg;

	if ( checkoutDlg.exec() == QDialog::Accepted ) {
		//checkout :)
		QByteArray parms;
		QDataStream s( parms, IO_WriteOnly );
		KURL servURL ( checkoutDlg.serverURL->url() );
		wcPath = checkoutDlg.localDir->url() + "/" + checkoutDlg.newDir->text();
		int cmd = 1;
		int rev = -1;
		s << cmd << servURL << KURL( wcPath ) << rev << QString( "HEAD" );
		servURL.setProtocol( "kdevsvn+" + servURL.protocol() ); //make sure it starts with "svn"
		SimpleJob * job = KIO::special(servURL,parms, true);
		job->setWindow( m_part->mainWindow()->main() );
		connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotEndCheckout( KIO::Job * ) ) );
	}
}

void subversionCore::switchTree( const KURL &path, const KURL &repositUrl,
								int revNum, const QString &revKind, bool recurse )
{
	KURL servURL = "kdevsvn+svn://blah/";
	QByteArray parms;
	QDataStream s( parms, IO_WriteOnly );
	// prepare arguments
	int cmd = 12;
	s << cmd << path << repositUrl ;
	s << recurse << revNum << revKind;

	SimpleJob * job = KIO::special(servURL, parms, false);
	connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotResult( KIO::Job * ) ) );
	initProcessDlg( (KIO::Job*)job, repositUrl.prettyURL() , path.prettyURL() );
}

void subversionCore::switchRelocate( const KURL &path,
									 const KURL &currentUrl, const KURL &newUrl, bool recurse )
{
	KURL servURL = "kdevsvn+svn://blah/";
	QByteArray parms;
	QDataStream s( parms, IO_WriteOnly );
	// prepare arguments
	int cmd = 16;
	s << cmd << path << currentUrl << newUrl << recurse;

	SimpleJob * job = KIO::special(servURL, parms, false);
	connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotResult( KIO::Job * ) ) );
	// this doesn't contact repository
}

void subversionCore::svnCopy( const KURL &src, int srcRev, const QString &srcRevKind,
							  const KURL &dest )
{
	KURL servURL = "kdevsvn+svn://blah/";
	QByteArray parms;
	QDataStream s( parms, IO_WriteOnly );
	// prepare arguments
	int cmd = 17;
	s << cmd << src << srcRev << srcRevKind << dest;

	SimpleJob * job = KIO::special(servURL, parms, false);
	connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotResult( KIO::Job * ) ) );
	initProcessDlg( (KIO::Job*)job, src.prettyURL(), dest.prettyURL() );
}

void subversionCore::merge( const KURL &src1, int rev1, QString revKind1,
							const KURL &src2, int rev2, QString revKind2, const KURL &wc_path,
							bool recurse, bool ignore_ancestry, bool force, bool dry_run )
{
	KURL servURL = "kdevsvn+svn://blah/";
	QByteArray parms;
	QDataStream s( parms, IO_WriteOnly );
	// prepare arguments
	int cmd = 18;
	s << cmd << src1 << rev1 << revKind1 << src2 << rev2 << revKind2 << wc_path;
	s << recurse << ignore_ancestry << force << dry_run;

	SimpleJob * job = KIO::special(servURL, parms, false);
	connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotResult( KIO::Job * ) ) );
	initProcessDlg( (KIO::Job*)job, src1.prettyURL() + "\n" + src2.prettyURL() ,
					wc_path.prettyURL() );
}

bool subversionCore::clientInfo( KURL path_or_url, bool recurse, QMap< KURL, SvnInfoHolder> &holderMap )
{
    KURL servURL = "kdevsvn+svn://blah/";
    QByteArray parms;
    QDataStream s( parms, IO_WriteOnly );
    int cmd = 15;
    s << cmd << path_or_url << -1 << QString("UNSPECIFIED") << -1 << QString("UNSPECIFIED") << recurse;
    SimpleJob *job = KIO::special( servURL, parms, false );

    QMap<QString,QString> ma;
    KIO::NetAccess::synchronousRun(job, 0, 0, 0, &ma ); // synchronize

    QValueList<QString> keys = ma.keys();
    QValueList<QString>::Iterator begin = keys.begin(), end = keys.end(), it;
    int curIdx, lastIdx;
    QRegExp rx( "([0-9]*)(.*)" );
    
    for ( it = begin; it != end; /*++it*/) {
        kdDebug(9036) << "METADATA key: " << *it << " value: " << ma[ *it ] << endl;
        if ( rx.search( *it ) == -1 ) return false; // something is wrong ! :)
        curIdx = lastIdx = rx.cap( 1 ).toInt();
        SvnInfoHolder holder;
        
        while ( curIdx == lastIdx ) {
            if ( rx.cap( 2 ) == "PATH" )
                holder.path = KURL( ma[ *it ] );
            else if ( rx.cap( 2  ) == "URL" )
                holder.url = KURL( ma[*it] );
            else if ( rx.cap( 2  ) == "REV" )
                holder.rev= ma[ *it ].toInt();
            else if ( rx.cap( 2  ) == "KIND" )
                holder.kind = ma[ *it ].toInt();
            else if ( rx.cap( 2  ) == "REPOS_ROOT_URL" )
                holder.reposRootUrl = KURL( ma[*it] );
            else if ( rx.cap( 2  ) == "REPOS_UUID" )
                holder.reposUuid = ma[ *it ];
            
            ++it;
            if ( it == end )
                break;
            if ( rx.search( *it ) == -1 ) return false; // something is wrong ! :)
            curIdx = rx.cap( 1 ).toInt();
        }
        holderMap.insert( holder.path, holder );
    }
    return true;;
}
        
void subversionCore::slotEndCheckout( KIO::Job * job ) {
	if ( job->error() ) {
		job->showErrorDialog( m_part->mainWindow()->main() );
		emit checkoutFinished( QString::null );
	} else
		emit checkoutFinished(wcPath);
}

void subversionCore::slotResult( KIO::Job * job ) {
    if ( job->error() ){
        job->showErrorDialog( m_part->mainWindow()->main() );
        if( job->error() == ERR_CANNOT_LAUNCH_PROCESS )
            KMessageBox::error( m_part->mainWindow()->main(),
                                i18n("If you just have installed new version of KDevelop,"
                                     " and if the error message was unknown protocol kdevsvn+*,"
                                     " try to restart KDE"
                                    ) );
        return;
    }
    KIO::MetaData ma = job->metaData();
	QValueList<QString> keys = ma.keys();
	qHeapSort( keys );
	QValueList<QString>::Iterator begin = keys.begin(), end = keys.end(), it;

	for ( it = begin; it != end; ++it ) {
// 		kdDebug(9036) << "METADATA : " << *it << ":" << ma[ *it ] << endl;
		if ( ( *it ).endsWith( "string" ) ) {
			m_part->mainWindow()->raiseView(processWidget());
			processWidget()->append( ma[ *it ] );
		}
		//extra check to retrieve the diff output in case with run a diff command
		if ( ( *it ).endsWith( "diffresult" ) ) {
			diffresult << ma[ *it ];
		}
	}
}
void subversionCore::slotLogResult( KIO::Job * job )
{
	if ( job->error() ){
		job->showErrorDialog( m_part->mainWindow()->main() );
        if( job->error() == ERR_CANNOT_LAUNCH_PROCESS )
            KMessageBox::error( m_part->mainWindow()->main(),
                                i18n("If you just have installed new version of KDevelop,"
                                    " and if the error message was unknown protocol kdevsvn+*,"
                                    " try to restart KDE"
                                    ) );
		return;
	}

	QValueList<SvnLogHolder> holderList;

	KIO::MetaData ma = job->metaData();
	QValueList<QString> keys = ma.keys();
	QRegExp rx( "([0-9]*)(.*)" );
	int curIdx, lastIdx;
	QString requestedUrl;

	for (QValueList<QString>::Iterator it = keys.begin(); it != keys.end(); /*++it*/ ){
		if ( rx.search( *it ) == -1 ){
			kdDebug(9036) << " Exiting loop at line " << __LINE__ <<endl;
			return; // something is wrong ! :)
		}
		curIdx = lastIdx = rx.cap( 1 ).toInt();
		SvnLogHolder logHolder;
		while ( curIdx == lastIdx ) {
			kdDebug(9036) << "svn log MetaData: " << *it << ":" << ma[ *it ] << endl;

			if ( rx.cap( 2 ) == "author" )
				logHolder.author = ma[*it];
			else if ( rx.cap( 2	 ) == "date" )
				logHolder.date = ma[*it];
			else if ( rx.cap( 2	 ) == "logmsg" )
				logHolder.logMsg = ma[*it];
			else if ( rx.cap( 2	 ) == "pathlist" )
				logHolder.pathList = ma[*it];
			else if ( rx.cap( 2	 ) == "rev" )
				logHolder.rev = ma[*it];
			else if ( rx.cap( 2  ) == "requrl" )
				requestedUrl = ma[*it];

			++it;
			if ( it == keys.end() )
				break;
			if ( rx.search( *it ) == -1 ){
				kdDebug(9036) << " Exiting loop at line " << __LINE__ <<endl;
				break; // something is wrong ! :)
			}
			curIdx = rx.cap( 1 ).toInt();
		}//end of while
		holderList.append( logHolder );
	}
	processWidget()->showLogResult( &holderList, requestedUrl );
	m_part->mainWindow()->raiseView(processWidget());

}

void subversionCore::slotBlameResult( KIO::Job * job )
{
    if ( job->error() ){
        job->showErrorDialog( m_part->mainWindow()->main() );
        if( job->error() == ERR_CANNOT_LAUNCH_PROCESS )
            KMessageBox::error( m_part->mainWindow()->main(),
                                i18n("If you just have installed new version of KDevelop,"
                                     " and if the error message was unknown protocol kdevsvn+*,"
                                     " try to restart KDE"
                                    ) );
        return;
    }
	QValueList<SvnBlameHolder> blameList;

	KIO::MetaData ma = job->metaData();
	QValueList<QString> keys = ma.keys();
	QRegExp rx( "([0-9]*)(.*)" );
	int curIdx, lastIdx;

	for (QValueList<QString>::Iterator it = keys.begin(); it != keys.end(); /*++it*/ ){
		if ( rx.search( *it ) == -1 ){
			kdDebug(9036) << " Exiting loop at line " << __LINE__ <<endl;
			return; // something is wrong ! :)
		}

		// if metadata has action key, that means a notification for svn_wc_notify_blame_completed
		// Thus, consume this notification
		if ( rx.cap( 2 ) == "action" ){
			curIdx = lastIdx = rx.cap( 1 ).toInt();
			while ( curIdx == lastIdx ){
				++it;
				if ( it == keys.end() ) break;
				if ( rx.search( *it ) == -1 ) continue; // something is wrong
				curIdx = rx.cap( 1 ).toInt();
			}
			continue;
		}
		// get actual blame data
		curIdx = lastIdx = rx.cap( 1 ).toInt();
		SvnBlameHolder blameHolder;
		while ( curIdx == lastIdx ) {
			kdDebug(9036) << "svn blame MetaData: " << *it << ":" << ma[ *it ] << endl;

			if ( rx.cap( 2 ) == "LINE" )
				blameHolder.line= (ma[*it]).toInt();
			else if ( rx.cap( 2	 ) == "REV" )
				blameHolder.rev = (ma[*it]).toLongLong();
			else if ( rx.cap( 2	 ) == "AUTHOR" )
				blameHolder.author= ma[*it];
			else if ( rx.cap( 2	 ) == "DATE" )
				blameHolder.date= ma[*it];
			else if ( rx.cap( 2	 ) == "CONTENT" )
				blameHolder.content = ma[*it];

			++it;
			if ( it == keys.end() )
				break;
			if ( rx.search( *it ) == -1 ){
				kdDebug(9036) << " Exiting loop at line " << __LINE__ <<endl;
				break; // something is wrong ! :)
			}
			curIdx = rx.cap( 1 ).toInt();
		}//end of while
		blameList.append( blameHolder );
// 		blameList.insert( blameHolder.line, blameHolder );
	}
    processWidget()->showBlameResult( &blameList );
    m_part->mainWindow()->raiseView(processWidget());
}

void subversionCore::slotDiffResult( KIO::Job * job )
{
	if ( job->error() ){
		job->showErrorDialog( m_part->mainWindow()->main() );
		if( job->error() == ERR_CANNOT_LAUNCH_PROCESS )
			KMessageBox::error( m_part->mainWindow()->main(),
								i18n("If you just have installed new version of KDevelop,"
									" and if the error message was unknown protocol kdevsvn+*,"
									" try to restart KDE"
									) );
		return;
	}
	KIO::MetaData ma = job->metaData();
	QValueList<QString> keys = ma.keys();
	qHeapSort( keys );
	QValueList<QString>::Iterator begin = keys.begin(), end = keys.end(), it;
	QStringList diffList;
	
	for ( it = begin; it != end; ++it ) {
// 		kdDebug(9036) << "METADATA : " << *it << ":" << ma[ *it ] << endl;
		if ( ( *it ).endsWith( "diffresult" ) ) {
			diffList << ma[ *it ];
		}
	}

	if ( diffList.count() > 0 ) {
		//check kompare is available
		if ( !KStandardDirs::findExe( "kompare" ).isNull() ) {
			KTempFile *tmp = new KTempFile;
			tmp->setAutoDelete(true);
			QTextStream *stream = tmp->textStream();
			stream->setCodec( QTextCodec::codecForName( "utf8" ) );
			for ( QStringList::Iterator it2 = diffList.begin();it2 != diffList.end() ; ++it2 ) {
				( *stream ) << ( *it2 ) << "\n";
			}
			tmp->close();
			KProcess *p = new KProcess;
			*p << "kompare" << "-n" << "-o" << tmp->name();
			p->start();
			
		} else { //else do it with message box
			KMessageBox::information( NULL, i18n("You don't have kompare installed. We recommend you to install kompare to view difference graphically") + "\nhttp://www.caffeinated.me.uk/kompare/" , QString::null , "userDoesNotWantKompare" );
			Subversion_Diff df;
			for ( QStringList::Iterator it2 = diffList.begin();it2 != diffList.end() ; ++it2 ) {
				df.text->append( *it2 );
			}
			QFont f = df.font();
			f.setFixedPitch( true );
			df.text->setFont( f );
			df.exec();
		}
	}
	else{
		KMessageBox::information( 0, i18n("No subversion differences") );
	}
}

void subversionCore::initProcessDlg( KIO::Job *job, const QString &src, const QString &dest )
{
	SvnProgressDlg *progress = new SvnProgressDlg( true );
	progress->setSourceUrl( src );
	progress->setDestUrl( dest );
	progress->setJob( job );
	connect( job, SIGNAL( totalSize(KIO::Job*, KIO::filesize_t) ),
			 progress, SLOT( slotTotalSize (KIO::Job*, KIO::filesize_t) ) );
	connect( job, SIGNAL( processedSize(KIO::Job*, KIO::filesize_t) ),
			 progress, SLOT( slotProcessedSize(KIO::Job*, KIO::filesize_t) ) );
}

void subversionCore::createNewProject( const QString& // dirName
                                       , const KURL& // importURL
                                       , bool // init
                                       ) {

}

#include "subversion_core.moc"

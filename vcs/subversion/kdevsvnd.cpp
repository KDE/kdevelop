/*
    This file is part of the KDE Project

    Copyright (C) 2003, 2004 Mickael Marchand <marchand@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    version 2 as published by the Free Software Foundation.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this library; see the file COPYING. If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <kapplication.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <ktextedit.h>
#include <kpassdlg.h>
#include <qdir.h>
#include <qfile.h>

#include "config.h"

#include "kdevsvnd.h"

#include "kdevsvnd_widgets.h"
#include "commitdlg.h"

extern "C" {
    KDE_EXPORT KDEDModule *create_kdevsvnd(const QCString &name) {
       return new KDevSvnd(name);
    }
}

KDevSvnd::KDevSvnd(const QCString &name)
 : KDEDModule(name) {
}
KDevSvnd::~KDevSvnd()
{}

QString KDevSvnd::commitDialog(QString modifiedFiles) {
	CommitDlg commitDlg;
	commitDlg.setCaption(i18n("Enter Commit Log Message:"));
	commitDlg.listMessage->setText( modifiedFiles );
	int result = commitDlg.exec();
	if ( result == QDialog::Accepted ) {
		return commitDlg.textMessage->text();
	} else
		return QString::null;
}
int KDevSvnd::sslServerTrustPrompt( QString errmsg, QString hostname, QString fingerPrint, QString validfrom, QString validuntil, QString issuerName, QString ascii_cert )
{
	SvnSSLTrustPrompt dlg;
	dlg.setupCertInfo( hostname, fingerPrint, validfrom, validuntil, issuerName, ascii_cert );
	dlg.setupFailedReasonMsg( errmsg );
	int result = dlg.exec();
	if ( result == QDialog::Accepted ){
		return dlg.code();
	} else{
		return -1;
	}
}
QString KDevSvnd::sslCertFile()
{
	QString fileName = KFileDialog::getOpenFileName(QString::null,QString::null,0, i18n("Open SSL certificate file"));
	return fileName;
}
QCString KDevSvnd::sslPasswdDlg(QString promptMsg)
{
	QCString passwd;
	int ret = KPasswordDialog::getPassword( passwd,promptMsg );
	if( ret == KPasswordDialog::Accepted ){
		QCString retstr;
		retstr.setNum(1);
		return retstr + passwd;
	} else{
		QCString nullstr;
		nullstr.setNum(-1);
		return nullstr;
	}
}


// void KDevSvnd::registerMe(const QCString &app)
// {
// 	insert(app, "test", new TestObject(app));
//    // When 'app' unregisters with DCOP, the TestObject will get deleted.
// }

// bool KSvnd::AreAnyFilesInSvn( const KURL::List& wclist ) {
// 	for ( QValueListConstIterator<KURL> it = wclist.begin(); it != wclist.end() ; ++it ) {
// 		kdDebug( 9036 ) << "Checking file " << ( *it ) << endl;
// 		QDir bdir ( ( *it ).path() );
// 		if ( bdir.exists() && QFile::exists( ( *it ).path() + "/.svn/entries" ) ) {
// 			return true;
// 		} else if ( !bdir.exists() ) {
// 			if ( isFileInSvnEntries( ( *it ).fileName(), ( *it ).directory() + "/.svn/entries" ) || isFileInExternals ( ( *it ).fileName(), ( *it ).directory()+"/.svn/dir-props" ) )
// 				return true;
// 		}
// 	}
// 	return false;
// }
//
// bool KSvnd::AreAnyFilesNotInSvn( const KURL::List& wclist ) {
// 	for ( QValueListConstIterator<KURL> it = wclist.begin(); it != wclist.end() ; ++it ) {
// 		kdDebug( 9036 ) << "Checking file " << ( *it ) << endl;
// 		QDir bdir ( ( *it ).path() );
// 		if ( bdir.exists() && !QFile::exists( ( *it ).path() + "/.svn/entries" ) ) {
// 			return true;
// 		} else if ( !bdir.exists() ) {
// 			if ( !isFileInSvnEntries( ( *it ).fileName(),( *it ).directory() + "/.svn/entries" ) && !isFileInExternals ( ( *it ).fileName(), ( *it ).directory()+"/.svn/dir-props" ) )
// 				return true;
// 		}
// 	}
// 	return false;
// }
//
// bool KSvnd::AreAllFilesInSvn( const KURL::List& wclist ) {
// 	for ( QValueListConstIterator<KURL> it = wclist.begin(); it != wclist.end() ; ++it ) {
// 		kdDebug( 9036 ) << "Checking file " << ( *it ) << endl;
// 		QDir bdir ( ( *it ).path() );
// 		if ( bdir.exists() && !QFile::exists( ( *it ).path() + "/.svn/entries" ) ) {
// 			return false;
// 		} else if ( !bdir.exists() ) {
// 			if ( !isFileInSvnEntries( ( *it ).fileName(),( *it ).directory() + "/.svn/entries" ) && !isFileInExternals ( ( *it ).fileName(), ( *it ).directory()+"/.svn/dir-props" )  )
// 				return false;
// 		}
// 	}
// 	return true;
// }
//
// bool KSvnd::AreAllFilesNotInSvn( const KURL::List& wclist ) {
// 	for ( QValueListConstIterator<KURL> it = wclist.begin(); it != wclist.end() ; ++it ) {
// 		kdDebug( 9036 ) << "Checking file " << ( *it ) << endl;
// 		QDir bdir ( ( *it ).path() );
// 		if ( bdir.exists() && QFile::exists( ( *it ).path() + "/.svn/entries" ) ) {
// 			return false;
// 		} else if ( !bdir.exists() ) {
// 			if ( isFileInSvnEntries( ( *it ).fileName(),( *it ).directory() + "/.svn/entries" ) || isFileInExternals ( ( *it ).fileName(), ( *it ).directory()+"/.svn/dir-props" ) )
// 				return false;
// 		}
// 	}
// 	return true;
// }
//
// bool KSvnd::isFileInSvnEntries ( const QString filename, const QString entfile ) {
// 	QFile file( entfile );
// 	if ( file.open( IO_ReadOnly ) ) {
// 		QTextStream stream( &file );
// 		QString line;
// 		while ( !stream.atEnd() ) {
// 			line = stream.readLine().simplifyWhiteSpace();
// 			if ( line == "name=\""+ filename + "\"" ) {
// 				file.close();
// 				return true;
// 			}
// 		}
// 		file.close();
// 	}
// 	return false;
// }
//
// bool KSvnd::isFileInExternals ( const QString filename, const QString propfile ) {
// 	QFile file( propfile );
// 	if ( file.open( IO_ReadOnly ) ) {
// 		QTextStream stream( &file );
// 		QStringList line;
// 		while ( !stream.atEnd() )
// 			line << stream.readLine().simplifyWhiteSpace();
// 		for ( uint i = 0 ; i < line.count(); i++ ) {
// 			if ( line[ i ] == "K 13"  && line[ i+1 ] == "svn:externals" ) { //Key 13 : svn:externals
// 				//next line should be "V xx"
// 				if ( line [ i+2 ].startsWith( "V " ) ) {
// 					//ok browse the values now
// 					i+=2;
// 					while ( i < line.count() ) {
// 						if ( line[ i ].startsWith( filename+" " ) ) { //found it !
// 							file.close( );
// 							return true;
// 						} else if ( line[ i ].isEmpty() ) {
// 							file.close( );
// 							return false; //we are out of svn:externals now...
// 						}
// 						i++;
// 					}
// 				}
// 			}
// 		}
// 		file.close();
// 	}
// 	return false;
// }
//
// bool KSvnd::anyNotValidWorkingCopy( const KURL::List& wclist ) {
// 	bool result = true; //one negative match is enough
// 	for ( QValueListConstIterator<KURL> it = wclist.begin(); it != wclist.end() ; ++it ) {
// 		//exception for .svn dirs
// 		if ( ( *it ).path(-1).endsWith( "/.svn" ) )
// 			return true;
// 		//if is a directory check whether it contains a .svn/entries file
// 		QDir dir( ( *it ).path() );
// 		if ( dir.exists() ) { //it's a dir
// 			if ( !QFile::exists( ( *it ).path() + "/.svn/entries" ) )
// 				result = false;
// 		}
//
// 		//else check if ./.svn/entries exists
// 		if ( !QFile::exists( ( *it ).directory() + "/.svn/entries" ) )
// 			result = false;
// 	}
// 	return result;
// }
//
// bool KSvnd::anyValidWorkingCopy( const KURL::List& wclist ) {
// 	for ( QValueListConstIterator<KURL> it = wclist.begin(); it != wclist.end() ; ++it ) {
// 		//skip .svn dirs
// 		if ( ( *it ).path(-1).endsWith( "/.svn" ) )
// 			continue;
// 		//if is a directory check whether it contains a .svn/entries file
// 		QDir dir( ( *it ).path() );
// 		if ( dir.exists() ) { //it's a dir
// 			if ( QFile::exists( ( *it ).path() + "/.svn/entries" ) )
// 				return true;
// 		}
//
// 		//else check if ./.svn/entries exists
// 		if ( QFile::exists( ( *it ).directory() + "/.svn/entries" ) )
// 			return true;
// 	}
// 	return false;
// }
//
// int KSvnd::getStatus( const KURL::List& list ) {
// 	int result = 0;
// 	uint files = 0, folders = 0, parentsentries = 0, parentshavesvn = 0, subdirhavesvn = 0, external = 0;
// 	for ( QValueListConstIterator<KURL> it = list.begin(); it != list.end() ; ++it ) {
// 		if ( isFolder ( ( *it ) ) ) {
// 			folders++;
// 		} else {
// 			files++;
// 		}
// 		if ( isFileInSvnEntries ( (*it).filename(),( *it ).directory() + "/.svn/entries" ) ) { // normal subdir known in the working copy
// 			parentsentries++;
// 		} else if ( isFolder( *it ) ) { // other subfolders (either another module checkouted or an external, or something not known at all)
// 			if ( QFile::exists( ( *it ).path() + "/.svn/entries" ) )
// 				subdirhavesvn++;
// 			if ( isFileInExternals( (*it).filename(), ( *it ).directory() + "/.svn/dir-props" ) ) {
// 				external++;
// 			}
// 		}
// 		if ( ( isFolder( ( *it ) ) && QFile::exists( ( *it ).directory() + "../.svn/entries" ) ) || QFile::exists( ( *it ).directory() + "/.svn/entries" ) ) //parent has a .svn ?
// 			parentshavesvn++;
// 	}
// 	if ( files > 0 )
// 		result |= SomeAreFiles;
// 	if ( folders == list.count() ) {
// 		result |= AllAreFolders;
// 		result |= SomeAreFolders;
// 	}
// 	if ( folders > 0 )
// 		result |= SomeAreFolders;
// 	if ( parentsentries == list.count() ) {
// 		result |= AllAreInParentsEntries;
// 		result |= SomeAreInParentsEntries;
// 	} else if ( parentsentries != 0 )
// 		result |= SomeAreInParentsEntries;
// 	if ( parentshavesvn == list.count() ) {
// 		result |= AllParentsHaveSvn;
// 		result |= SomeParentsHaveSvn;
// 	} else if ( parentshavesvn > 0 )
// 		result |= SomeParentsHaveSvn;
// 	if ( subdirhavesvn == list.count() ) {
// 		result |= AllHaveSvn;
// 		result |= SomeHaveSvn;
// 	} else if ( subdirhavesvn > 0 )
// 		result |= SomeHaveSvn;
// 	if ( external == list.count() ) {
// 		result |= AllAreExternalToParent;
// 		result |= SomeAreExternalToParent;
// 	} else if ( external > 0 )
// 		result |= SomeAreExternalToParent;
//
// 	return result;
// }
//
// bool KSvnd::isFolder( const KURL& url ) {
// 	QDir d( url.path() );
// 	return d.exists();
// }
//
// QStringList KSvnd::getActionMenu ( const KURL::List &list ) {
// 	QStringList result;
// 	int listStatus = getStatus( list );
//
// 	if ( !(listStatus & SomeAreInParentsEntries) &&
// 	     !(listStatus & SomeAreExternalToParent) &&
// 	     !(listStatus & SomeHaveSvn)) {
// 		if( list.size() == 1 && listStatus & SomeAreFolders) {
// 			result << "Checkout";
// 			result << "Export";
// //			result << "CreateRepository";
// 			result << "Import";
// 		}
// 	} else if ( (listStatus & AllAreInParentsEntries) ) {
// 		result << "Diff";
// 		//In SVN
// //		result << "ShowLog";
// //		result << "CheckForModifications";
// //		result << "RevisionGraph";
// //		result << "_SEPARATOR_";
// //		result << "Update to revision..."
// 		result << "Rename";
// 		result << "Delete";
// 		if( listStatus & SomeAreFolders && !(listStatus & SomeAreFiles)) {
// 			result << "Revert";
// //			result << "Cleanup";
// 		}
// 		result << "_SEPARATOR_";
// //		result << "BranchTag";
// 		result << "Switch";
// 		result << "Merge";
// 		if( listStatus & SomeAreFolders && !(listStatus & SomeAreFiles)) {
// //			result << "Export";
// //			result << "Relocate";
// 			result << "_SEPARATOR_";
// 			result << "Add";
// 		}
// 		result << "_SEPARATOR_";
// 		if( listStatus & SomeAreFiles && !(listStatus & SomeAreFolders)) {
// 			result << "Blame";
// 		}
// 		result << "CreatePatch";
//
// 		if( list.size() == 1 && listStatus & SomeAreFolders) {
// //			result << "ApplyPatchToFolder";
// 		}
// 	}
// 	return result;
// }
//
// QStringList KSvnd::getTopLevelActionMenu ( const KURL::List &list ) {
// 	QStringList result;
// 	int listStatus = getStatus( list );
//
//
// 	if ( ( listStatus & AllParentsHaveSvn &&
// 			( ( listStatus & SomeAreExternalToParent ) || ( listStatus & SomeAreInParentsEntries ) )
// 				|| ( listStatus & SomeHaveSvn ) )
// 		) {
// 		result << "Update";
// 		result << "Commit";
// 	}
//
// 	return result;
// }
//
// #if 0
// void KSvnd::notify(const QString& path, int action, int kind, const QString& mime_type, int content_state, int prop_state, long int revision, const QString& userstring) {
// 	kdDebug(9036) << "KDED/Subversion : notify " << path << " action : " << action << " mime_type : " << mime_type << " content_state : " << content_state << " prop_state : " << prop_state << " revision : " << revision << " userstring : " << userstring << endl;
// 	QByteArray params;
//
// 	QDataStream stream(params, IO_WriteOnly);
// 	stream << path << action << kind << mime_type << content_state << prop_state << revision << userstring;
//
// 	emitDCOPSignal( "subversionNotify(QString,int,int,QString,int,int,long int,QString)", params );
// }
//
// void KSvnd::status(const QString& path, int text_status, int prop_status, int repos_text_status, int repos_prop_status, long int rev ) {
// 	kdDebug(9036) << "KDED/Subversion : status " << path << " " << text_status << " " << prop_status << " "
// 			<< repos_text_status << " " << repos_prop_status << " " << rev << endl;
// 	QByteArray params;
//
// 	QDataStream stream(params, IO_WriteOnly);
// 	stream << path << text_status << prop_status << repos_text_status << repos_prop_status << rev;
//
// 	emitDCOPSignal( "subversionStatus(QString,int,int,int,int,long int)", params );
// }
//
// void KSvnd::popupMessage( const QString& message ) {
// 	kdDebug(9036) << "KDED/Subversion : popupMessage" << message << endl;
// 	KMessageBox::information(0, message, i18n( "Subversion" ) );
// }
// #endif

#include "kdevsvnd.moc"

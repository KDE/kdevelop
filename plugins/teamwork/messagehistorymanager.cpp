/***************************************************************************
   copyright            : (C) 2006 by David Nolden
   email                : david.nolden.kdevelop@art-master.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <fstream>
#include <QDate>
#include <QMap>
#include <QTimer>
#include <kurl.h>
#include <kio/netaccess.h>
#include <klockfile.h>

#include "network/serialization.h"
#include "network/messageserialization.h"

#include "messagehistorymanager.h"
#include "kdevteamwork_messages.h"
#include "kdevteamwork_user.h"
#include "teamworkfoldermanager.h"


#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/level.hpp>

#include <QDir>
#include "kdevteamwork_client.h"
//#include <map>

using namespace Teamwork;

struct FileEntryHeader {
	int version;
	int messages;
	FileEntryHeader( int _messages=0 ) : version(1), messages(_messages) {
	}

	void read( istream& str ) {
		str >> version >> messages;
	}
	void write( ostream& str ) {
		str << version << endl << messages << endl;
	}
};

HistoryMessageDesc::HistoryMessageDesc() {
}

HistoryMessageDesc::HistoryMessageDesc( const HistoryMessagePointer& msg )
{
	message = msg;
	HistoryMessagePointer::Locked l = msg;
	if( l ) {
		isIncoming = l->info().isIncoming();
		
		UserPointer::Locked lu = userFromSession( l->info().session() );
		if( lu ) {
			user = lu->identity();
		} else {
		}
	} else {
	}
}

MessageHistoryManager::MessageHistoryManager( Teamwork::LoggerPointer logger ) : m_logger(logger) {
	m_pendingTimer = new QTimer();
	m_pendingTimer->start( 1000 );
	connect( m_pendingTimer, SIGNAL( timeout() ), this, SLOT( writePending() ) );
  m_lockFile = new KLockFile( lockFileName() );
}

MessageHistoryManager::~MessageHistoryManager() {
}

void MessageHistoryManager::addMessage( KDevTeamworkTextMessage* msg ) {
	HistoryMessageDesc m( msg );
	m_pending << m;
	emit newMessage( msg );
}

QList< HistoryMessagePointer > MessageHistoryManager::getMessages( const KDevTeamworkClientPointer& client, QDate from, QDate until, UserSet users  ) {
	writePending();
	QStringList interestingFiles;
	QList< HistoryMessageDesc > ret;
	try {
		KUrl kdir = directory();
		
		if( !kdir.isLocalFile() )
			throw QString( "directory \"%1\" is not local" ).arg( kdir.prettyUrl() );
		
		QDir dir( kdir.path() );
		if( !dir.isReadable() ) throw QString( "directory \"%1\" is not readable" ).arg( dir.path() );

		QStringList files = dir.entryList( QDir::Files );

		for( QStringList::iterator it = files.begin(); it != files.end(); ++it ) {
			QDate date = QDate::fromString( *it, Qt::ISODate );
			if( !date.isValid() ) continue;
			if( from.isValid() ) {
				if( until.isValid() ) {
					///from "from" until "until"
					if( date >= from && date <= until )
						interestingFiles << *it;
				} else {
					///from "from" until now
					if( date >= from )
						interestingFiles << *it;
				}
			} else {
				if( until.isValid() ) {
					///from begin until "until"
					if( date <= until )
						interestingFiles << *it;
				} else {
					///All messages
					interestingFiles << *it;
				}
			}
		}

		for( QStringList::iterator it = interestingFiles.begin(); it != interestingFiles.end(); ++it ) {
			KUrl fileUrl = kdir;
			fileUrl.addPath( *it );
			std::ifstream f( fileUrl.path().toLatin1(), ios_base::binary );
			if( !f.good() ) {
				err() << "could not open " << ~fileUrl.path();
			}
			while( !f.eof() ) {
				if( !f.good() ) {
					err() << "error while reading " << ~fileUrl.path();
					break;
				}

				FileEntryHeader header;
				header.read( f );
			
				//f.read( (char*)&header, sizeof( FileEntryHeader ) );
				if( f.eof() ) break;
			
				if( !f.good() ) {
					err() << "could not read a header from file \"" << ~*it << "\"";
					break;
				} else {
					out( Logger::Debug ) << "successfully read a header";
				}

				try  {
				///read the message from the archive
					boost::archive::xml_iarchive arch( f );
					QList<HistoryMessageDesc> messages;
					for( int a = 0; a < header.messages; a++ ) {
						HistoryMessageDesc msg;
						arch >> boost::serialization::make_nvp( "message", msg );
						if( users.empty() || users.contains( msg.user ) )
							ret << msg;
					}
				}
				catch( std::exception& exc ) {
					err() << "error while deserializing from \"" << ~*it << "\": " << exc.what();
					break;
				}
			}
		}

	}
	catch( QString str ) {
		err() << "error in MessageHistoryManager::getMessages: " << ~str;
	}

	return fillMessageUsers( ret, client );
}

struct WaitError {
  QString str;
  WaitError( const QString& stri ) : str( stri ) {
  }
};

void MessageHistoryManager::writePending()
{
	try
	{
		KUrl dir = directory();
		if( !dir.isLocalFile() ) throw QString( "the directory %1 is not local" ).arg( dir.prettyUrl() );
		
		m_pendingTimer->start( 1000 );
		QMap< QDate, QList<HistoryMessageDesc> > map;
		std::map< Teamwork::UniqueMessageId, HistoryGroupLocation > insertions;
		
		for( QList<HistoryMessageDesc>::iterator it = m_pending.begin(); it != m_pending.end(); ++it ) {
			HistoryMessagePointer::Locked l = it->message;
			if( l ) {
				QDate key( l->creationTime().date() );
				map[ key ] << *it;
			} else {
				err() << "could not lock a message for storing it into the history, the message will be lost";
			}
		}
	
		for( QMap< QDate, QList<HistoryMessageDesc> >::iterator it = map.begin(); it != map.end(); ++it ) {
			QDate date( it.key() );

			if( !date.isValid() ) {
				err() << "tried to store " << it->count() << " messages with invalid date";
				continue;
			}

      if( !m_lockFile->isLocked() ) {
        switch( m_lockFile->lock( KLockFile::NoBlockFlag | KLockFile::ForceFlag ) ) {
          case KLockFile::LockOK:
            break;
          case KLockFile::LockFail:
            throw WaitError( "locking the lockfile " + lockFileName() + " failed" );
            break;
          case KLockFile::LockError:
            throw "an error occured while locking the lockfile " + lockFileName();
            break;
          case KLockFile::LockStale:
            throw WaitError( "the lockfile " + lockFileName() + " is stale" );
        }
      }

			KUrl file = dir;
			QString localFileName = date.toString( Qt::ISODate );
			file.addPath( localFileName );
			std::string fileName = ~file.path();
			
			if( fileName.empty() ) {
				err() << "empty file-path for date " << ~date.toString( Qt::ISODate );
				continue;
			}

			std::ofstream f( fileName.c_str(), ios_base::binary | ios_base::app );

			if( !f.good() ) {
				err() << "could not open file \"" << fileName << "\" for read/write";
				continue;
			}
			f.seekp( 0, std::ios_base::end );

			FileEntryHeader header( it->count() );

			HistoryGroupLocation location;
			location.fileName = ~localFileName;
			location.offset = f.tellp();

			header.write( f );
			//f.write( (char*)&header, sizeof( FileEntryHeader ) );
			
			if( !f.good() ) {
				err() << "could not write header into file \"" << fileName << "\"";
				continue;
			}

			try  {
				///Store the messages into the archive
				boost::archive::xml_oarchive arch( f );
				for( QList<HistoryMessageDesc>::iterator it2 =  it->begin(); it2 != it->end(); ++it2 ) {
					arch << boost::serialization::make_nvp( "message", *it2 );

					HistoryMessagePointer::Locked l = it2->message;
					if( l ) {
						insertions[ l->info().uniqueId() ] = location;
					} else {
						err() << "could not lock a message while storing it into the history";
					}
				}
			}
			catch( std::exception& exc ) {
				err() << "error while serializing into \"" << fileName << "\": " << exc.what();
			}
		}

		///The index maps unique-message-ids to the files they can be found in(including offset to the FileEntryHeader)
		std::map<Teamwork::UniqueMessageId, HistoryGroupLocation> index;

		if( !insertions.empty() ) {
			try {
			readIndex( index  );
			}
			catch( QString str ) {
				out( Logger::Warning ) << "error in readIndex: " << str;
			}
	
			index.insert( insertions.begin(), insertions.end() );
	
			writeIndex( index );
		}
	

		m_pending.clear();
	}
  catch( WaitError err ) {
    out( Logger::Warning ) << "in MessageHistoryManager::writePending: " << err.str;
  }
	catch( QString str )
	{
		err() << "error in MessageHistoryManager::writePending: " << str;
		m_pending.clear();
	}

  if( m_lockFile->isLocked() )
    m_lockFile->unlock();
}

QString MessageHistoryManager::lockFileName() {
  KUrl file = directory();
  file.addPath( ".lock" );
  return file.path();
}

void MessageHistoryManager::readIndex( std::map<Teamwork::UniqueMessageId, HistoryGroupLocation>& index ) throw( QString) {
	KUrl ind = directory();
	ind.addPath( "index" );
	if( !ind.isLocalFile() ) throw  QString( "the index-file is not local" );
		
	std::string indexFile = ~ind.path();

	{
		std::ifstream f( indexFile.c_str(), ios_base::binary );
		if( !f.good() )  throw QString( "could not open index-file for reading" );
			
		try  {
			boost::archive::binary_iarchive arch( f );
			arch & index;
		}
		catch( std::exception& exc ) {
			err() << "error while deserializing the index \"" << indexFile << "\": " << exc.what();
			throw QString( "error in readIndex" );
		}
	}
}

void MessageHistoryManager::writeIndex( const std::map<Teamwork::UniqueMessageId, HistoryGroupLocation>& index ) throw( QString) {
	{
		KUrl ind = directory();
		ind.addPath( "index" );
		if( !ind.isLocalFile() ) throw  QString( "the index-file is not local" );

		std::string indexFile = ~ind.path();

		std::ofstream f( indexFile.c_str(), ios_base::binary );
		if( !f.good() )  throw QString( "could not open index-file for writing" );
			
		try  {
					///Store the messages into the archive
			boost::archive::binary_oarchive arch( f );
			arch & index;
		}
		catch( std::exception& exc ) {
			err() << "error while serializing the index \"" << indexFile << "\": " << exc.what();
			throw QString( "error in writeIndex" );
		}
	}
}

KUrl MessageHistoryManager::directory() throw(QString)
{
  KUrl ul = TeamworkFolderManager::absolute( "messages" );

	if( !KIO::NetAccess::exists( ul, true, 0 ) )
		KIO::NetAccess::mkdir( ul, 0 );

	if( !KIO::NetAccess::exists( ul, true, 0 ) )
		throw QString( "could not create messages-directory " ) + ul.path();

	return ul;
}

void MessageHistoryManager::readMessages( const QString& file, uint offset, QList<HistoryMessageDesc>& messages ) throw(QString)
{
	KUrl path = directory();
	path.addPath( file );
		
	if( !path.isLocalFile() ) throw QString( "message is not local" );
	if( !KIO::NetAccess::exists( path, true, 0 ) ) throw QString( "index does not exist" );
	
	std::ifstream f( path.path().toLatin1(), ios_base::binary );
	f.seekg( offset );
	if( !f.good() || f.eof() ) throw QString( "could not open file and seek to offset %1 in file %2" ).arg( offset ).arg( path.path() );

		FileEntryHeader header;

		header.read( f );
		if( !f.eof() ) {
			if( !f.good() || f.eof() )
				throw QString( "could not read a header from file \"%1\"" ).arg( path.path() );
	
			try  {
				boost::archive::xml_iarchive arch( f );
				for( int a = 0; a < header.messages; a++ ) {
					HistoryMessageDesc msg;
					arch >> boost::serialization::make_nvp( "message", msg );
					messages << msg;
				}
			}
			catch( std::exception& exc ) {
				err() << "error while deserializing from \"" << ~path.path() << "\": " << exc.what();
			}
		}
}

HistoryMessagePointer  MessageHistoryManager::getMessageFromId( Teamwork::UniqueMessageId id, const KDevTeamworkClientPointer& client ) {
	std::map<Teamwork::UniqueMessageId, HistoryGroupLocation> index;
	try {
	readIndex( index );
	std::map<Teamwork::UniqueMessageId, HistoryGroupLocation>::iterator it = index.find( id );
	if( it != index.end() ) {
		QList<HistoryMessageDesc> messages;
		readMessages( ~it->second.fileName, it->second.offset, messages );

		for( QList<HistoryMessageDesc>::iterator it = messages.begin(); it != messages.end(); ++it ) {
			HistoryMessagePointer::Locked l = it->message;
			if( l ){
				if( l->info().uniqueId() == id ) {
					return fillMessageUser( *it, client );
				}
			}
		}

		return HistoryMessagePointer();
	} else {
		return HistoryMessagePointer();
	}
	}
	catch( QString str ) {
		err() << "error in messageFromId: " << ~str;
		return HistoryMessagePointer();
	}
}

HistoryMessagePointer MessageHistoryManager::fillMessageUser( const HistoryMessageDesc& desc, const KDevTeamworkClientPointer& client ) {
	HistoryMessagePointer::Locked lmsg = desc.message;
	KDevTeamworkClientPointer::Locked l = client;
	if( !l ) {
		err() << "could not lock KDevTeamworkClient";
		return HistoryMessagePointer();
	}
	if( lmsg ) {
		lmsg->info().setUser( l->getUser( desc.user ) );
		return lmsg;
	} else {
		err() << "could not lock a message in fillMessageUsers";
	}

	return HistoryMessagePointer();
}

QList<HistoryMessagePointer> MessageHistoryManager::fillMessageUsers( const QList<HistoryMessageDesc>& messages, const KDevTeamworkClientPointer& client ) {
	QList<HistoryMessagePointer> ret;
	KDevTeamworkClientPointer::Locked l = client;

	if( l ) {
		foreach( const HistoryMessageDesc& desc, messages ) {
			HistoryMessagePointer::Locked lmsg = desc.message;
			if( lmsg ) {
				lmsg->info().setUser( l->getUser( desc.user ) );
				ret << lmsg;
			} else {
				err() << "could not lock a message in fillMessageUsers";
			}
		}
	} else {
		err() << "could not lock teamwork-client";
	}
	
	return ret;
}

Teamwork::LoggerPrinter MessageHistoryManager::out( Teamwork::Logger::Level level ) {
	Teamwork::LoggerPrinter ret( m_logger, level );
	ret << "in MessageHistoryManager: ";
	return ret;
}


Teamwork::LoggerPrinter MessageHistoryManager::err() {
	Teamwork::LoggerPrinter ret( m_logger, Teamwork::Logger::Error );
	ret << "Error in MessageHistoryManager: ";
	return ret;
}

BOOST_CLASS_IMPLEMENTATION(HistoryMessageDesc, boost::serialization::object_serializable)

#include "messagehistorymanager.moc"

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on

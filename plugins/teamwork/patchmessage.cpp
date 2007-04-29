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

#include "network/serialization.h"
#include <boost/serialization/list.hpp>
#include <boost/serialization/vector.hpp>
#include  <memory>

#include  "patchmessage.h"
#include <k3process.h>
#include <kurl.h>
#include <kmimetype.h>
#include <memory.h>
#include <kio/netaccess.h>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include "teamworkfoldermanager.h"

#include "kdevteamwork.h"

#include "kdevteamwork_helpers.h"

PatchData::PatchData( const LocalPatchSourcePointer& p, LoggerPointer logg ) : m_patch( p ), logger( logg ), deserialized( false ), finished( false ), isBinary_( false ) {
  projectDir = TeamworkFolderManager::workspaceDirectory();
}

LocalPatchSourcePointer PatchData::patch() {
  return m_patch;
}

PatchesListMessage::PatchesListMessage( InArchive& arch, const Teamwork::MessageInfo& info ) : Precursor( arch, info ) {
  serial( arch );
}

void PatchesListMessage::serialize( OutArchive& arch ) {
  Precursor::serialize( arch );
  serial( arch );
}

uint LocalPatchSource::patchDepth() const {
  QString cmd = ~applyCommand;
  int i = cmd.indexOf( "-p" );
  if( i == -1 ) return 0;
  if( i+1 >= cmd.length() ) return 0;
  QString f = cmd.mid( i+2 ).trimmed();
  if( f.isEmpty() ) return 0;
  f.truncate( 1 );
  bool b = false;
  uint ret = f.toUInt( &b );
  if( !b ) return 0;
  else
    return ret;
}

string LocalPatchSource::patchTool( bool reverse ) const {
  QString cmd;
  if( reverse )
    cmd = (~unApplyCommand).trimmed();

  if( cmd.isEmpty() )
    cmd = (~applyCommand).trimmed();

  if( cmd[0] == '-' ) return "patch";
  int firstWhite = cmd.indexOf( " " );
  QString ret;
  if( firstWhite == -1 )
    ret = cmd;
  else
    ret = cmd.mid( 0, firstWhite );

  ret = ret.trimmed();
  if( ret.isEmpty() )
    return "patch";
  else
    return ~ret;
}

string LocalPatchSource::patchParams( bool reverse ) const {
  
  QString cmd;
  bool normalCommand = false;
  if( reverse )
    cmd = (~unApplyCommand).trimmed();
  if( cmd.isEmpty() ) {
    cmd = (~applyCommand).trimmed();
    normalCommand = true;
  }
  if( cmd.isEmpty() )
    return string("-p0 --backup") + (reverse ? " --reverse" : "");
  else
    if( reverse && normalCommand )
      cmd += " --reverse";
  
  if( cmd.startsWith( ~patchTool(reverse) ) )
    return ~cmd.mid( patchTool(reverse).length() ).trimmed();
  else
    return ~cmd.trimmed();
}

/*
void LocalPatchSource::setMimeType( KMimeType::Ptr mimeType ) {
  QByteArray array;
  {
    QDataStream str( &array, QIODevice::WriteOnly );
    mimeType->save( str );
  }
  mimetype.resize( array.count() );
  memcpy( &( mimetype[ 0 ] ), array.data(), array.count() );
}

KMimeType::Ptr LocalPatchSource::getMimeType() {
  QByteArray array( &( mimetype[ 0 ] ), mimetype.size() );
  QDataStream str( &array, QIODevice::ReadOnly );
  return KMimeType::Ptr( new KMimeType( str, 0 ) );
}*/

class AbstractPatchArchiver {
  virtual void put( QByteArray& data );
  virtual void put( int i );
  virtual ~AbstractPatchArchiver() {
  };
  };
  /*
template<class Archive>
class PatchArchiver {
  Archive& arch;
  PatchArchiver( Archive& a ) : arch(a) {
  }
 
  virtual void put( QByteArray& data ) {
    arch & data;
  }
  
  virtual void put( int i ) {
    arch & i;
  }
  };
  */
void PatchData::transferData( KIO::Job* /*job*/, const QByteArray& data ) {
  std::vector<char> vec;
  vec.resize( data.size() );  ///a serialization-function for QByteArray should be written instead of moving the data through std::vector
  memcpy( &(vec[0]), data.data(), data.size() );
  EntryType v = Vector;
  *currentArchive << v;
  *currentArchive & vec; 
}

void PatchData::transferFinished() {
  finished = true;
}

void PatchData::transferCanceled() {
  errored = true;
}

//template<class Arch>
void PatchData::saveInternal( OutArchive& arch, const uint /*version*/ ) {
  auto_ptr<PatchDataReceiver> rec( new PatchDataReceiver( this ) );

  if ( !m_patch || deserialized )
    throw CannotReserialize(); ///This type of Message can only be serialized after being constructed with a local patch-source
  try {
    arch & m_patch;
    currentArchive = &arch;
    errored = false;
    finished = false;
    if ( !m_patch )
      throw "invalid patch-source";
    LocalPatchSourcePointer::Locked lpatch = m_patch;
    if ( !lpatch )
      throw "patch-source could not be locked";

    if ( !lpatch->filename.empty() ) {
      ///Load a file
      isBinary_ = true;
      EntryType v = BinaryHeader;
      arch << v;
      KUrl url( TeamworkFolderManager::absolute( ~lpatch->filename ) );
      log( Logger::Debug ) << "opening file for sending: " << ~url.prettyUrl();

      //(QWidget*)KDevApi::self()->mainWindow()->main()
      if ( !KIO::NetAccess::exists( url, true, 0 ) )
        throw ( "the file \"" + url.prettyUrl() + "\" seems not to exist, or is not accessible" );

      auto_ptr<KIO::TransferJob> transfer( KIO::get
                                             ( url, false, false ) );
      if ( !transfer.get() )
        throw ( "could not create transfer-job for reading " + url.prettyUrl() );
      //transfer->setWindow( (QWidget*)KDevApi::self()->mainWindow()->main() );
      QObject::connect( &( *transfer ), SIGNAL( data( KIO::Job *, const QByteArray & ) ), &( *rec ), SLOT( transferData( KIO::Job*, const QByteArray& ) ), Qt::DirectConnection );
      QObject::connect( &( *transfer ), SIGNAL( result( KJob * ) ), &( *rec ), SLOT( transferFinished( KJob* ) ), Qt::DirectConnection );
      if ( !transfer->exec() )
        errored = true;

      QString errorText;
      int error = transfer->error();
      if ( error )
        errorText = transfer->errorString();

      transfer->kill();

      if ( error )
        throw ( "failed reading the file \"" + url.prettyUrl() + "\" reason: \"" + errorText + "\"" );

      EntryType t = End;
      arch << t;

    } else if ( !lpatch->command.empty() ) {
      ///Execute a command and send it's output
      isBinary_ = false;
      EntryType v = TextHeader;
      arch << v;
      QStringList args = splitArgs( ~lpatch->command );
      {
        LoggerPrinter l = log() << "calling process: \"" << lpatch->command << "\" params: ";
        for ( QStringList::iterator it = args.begin(); it != args.end(); ++it )
          l << "\"" << ~( *it ) << "\"";
        l << " in folder " << ~projectDir.path();
      }

      auto_ptr<K3Process> proc( new K3Process() );
      QObject::connect( &( *proc ), SIGNAL( receivedStdout( K3Process*, char*, int ) ), &( *rec ), SLOT( receivedStdout( K3Process*, char*, int ) ), Qt::DirectConnection );

      proc->setPriority( K3Process::PrioLowest );
      proc->setWorkingDirectory( projectDir.path() );
      *proc << args;
      if ( !proc->start( K3Process::Block, K3Process::Stdout ) )
        throw "the process could not be started";

      if ( !proc->normalExit() )
        throw "process did not exit normally";
      if ( proc->exitStatus() != 0 )
        throw QString( "process returned with exit-status " + QString::number( proc->exitStatus() ) );

      log() << lpatch->command << ": successful";

      EntryType t = End;
      arch << t;
    } else {
      throw "the patch-data cannot be sent: missing command or url";
    }
  } catch ( const char * str ) {
    currentArchive = 0;
    throw NonFatalSerializationError( ( string( "PatchData::serialize: " ) + str ).c_str() );
  } catch( QString str ) {
    currentArchive = 0;
    throw NonFatalSerializationError( ( string( "PatchData::serialize: " ) + str.toUtf8().data() ).c_str() );
  }
  currentArchive = 0;
}

//template<class Arch>
void PatchData::load( InArchive& arch, const uint /*version*/ ) {
  deserialized = true;
  //cout << "got patch: ";

  arch & m_patch;

  EntryType t;
  arch >> t;
  if ( t == BinaryHeader ) {
    isBinary_ = true;
    while ( 1 ) {
      arch >> t;
      if ( t == Vector ) {
        vector<char> vec;
        arch & vec;
        //cout << str;
        m_data.append( QByteArray( &( vec[ 0 ] ), vec.size() ) );
      } else {
        if ( t != End )
          throw NonFatalSerializationError( "stream-error in PatchData(Text)" );
        else
          break;
      }
    }
  } else {
    isBinary_ = false;
    while ( 1 ) {
      arch >> t;
      if ( t == Text ) {
        string str;
        arch & str;
        //cout << str;
        m_data.append( str.c_str() );
      } else {
        if ( t != End )
          throw NonFatalSerializationError( "stream-error in PatchData(Text)" );
        else
          break;
      }
    }
  }
}

/*
template void PatchData::load( boost::archive::polymorphic_iarchive& arch, const uint );
 
template void PatchData::saveInternal( boost::archive::polymorphic_oarchive& arch, const uint );
 
template void PatchData::load( boost::archive::text_iarchive& arch, const uint );
 
template void PatchData::saveInternal( boost::archive::text_oarchive& arch, const uint );
 
template void PatchData::load( boost::archive::binary_iarchive& arch, const uint );
 
template void PatchData::saveInternal( boost::archive::binary_oarchive& arch, const uint );
*/

void PatchData::receivedStdout( K3Process */*proc*/, char *buffer, int /*buflen*/ ) {
  if ( !currentArchive || errored ) {
    if ( !errored ) {
      log() << "received unexpected stdout-data";
      errored = true;
    }
    return ;
  }
  EntryType t = Text;
  *currentArchive << t;
  string str( buffer );
  *currentArchive << str;
}

bool PatchData::isBinary() {
  return isBinary_;
}

const QByteArray& PatchData::data() {
  return m_data;
}

QStringList splitArgs( const QString& str ) {
  QStringList ret;
  QString current = str;
  int pos = 0;
  while ( ( pos = current.indexOf( ' ', pos ) ) != -1 ) {
    if ( current[ 0 ] == '"' ) {
      int end = current.indexOf( '"' );
      if ( end > pos )
        pos = end;
    }
    QString s = current.left( pos );
    if ( s.length() > 0 )
      ret << s;
    current = current.mid( pos + 1 );
    pos = 0;
  }
  if ( current.length() )
    ret << current;
  return ret;
}

PatchRequestData::PatchRequestData( const LocalPatchSourcePointer& id, KDevTeamwork* tw, RequestType req ) : request_( id ), requestType_( req ), stat( Waiting ), emitter( new SafeTeamworkEmitter( tw ) ) {
  LocalPatchSourcePointer::Locked l = id;
  if( l ) {
    ident_ = l->identity();
  }
}

/*
PatchRequestMessage::PatchRequestMessage( const Teamwork::MessageTypeSet& info, const LocalPatchSource::Identity& id, KDevTeamwork* tw, RequestType requestType
 ) : PatchRequestData( id ), Precursor( info ),  emitter( new SafeTeamworkEmitter(tw), m_requestType( requestType ) ) {
  }*/


PatchDataReceiver::PatchDataReceiver( PatchData* d ) : data( d ) {}
void PatchDataReceiver::receivedStdout( K3Process *proc, char *buffer, int buflen ) {
  data->receivedStdout( proc, buffer, buflen );
}

void PatchDataReceiver::transferData( KIO::Job* job, const QByteArray& array ) {
  data->transferData( job, array );
}

void PatchDataReceiver::transferFinished( KJob */*job*/ ) {
  data->transferFinished();
}

QString PatchRequestData::messageText() const {
  return "patch-request: " + ~patchDesc();
}


QIcon PatchRequestData::messageIcon() const {
  IconCache & cache( *IconCache::instance() );

  if ( !selfMessage() ->info().isIncoming() ) {
    return cache( "generalrequest_out" );
    if ( stat == Denied ) {
      return cache( "generalrequest_out_denied" );
    } else if ( stat == Accepted ) {
      return cache( "generalrequest_out_accepted" );
    } else if ( stat == Unknown ) {
      return cache( "unknown" );
    } else {
      return cache( "generalrequest_out" );
    }
  } else {
    if ( stat == Denied ) {
      return cache( "generalrequest_in_denied" );
    } else if ( stat == Accepted ) {
      return cache( "generalrequest_in_accepted" );
    } else if ( stat == Unknown ) {
      return cache( "unknown" );
    } else {
      return cache( "generalrequest_in" );
    }
  }
}

PatchRequestMessage* PatchRequestData::selfMessage() {
  return dynamic_cast<PatchRequestMessage*>( this );
}

const PatchRequestMessage* PatchRequestData::selfMessage() const {
  return dynamic_cast<const PatchRequestMessage*>( this );
}

MessageInterface::ReplyResult PatchRequestMessage::gotReply( const MessagePointer& p ) {
  KDevSystemMessagePointer::Locked lmessage = ( ( MessagePointer ) p ).cast<KDevSystemMessage>();

  if ( lmessage ) {
    switch ( lmessage->message() ) {
      case KDevSystemMessage::ActionFailed:
      stat = Failed;
      break;
      case KDevSystemMessage::ActionSuccessful:
      stat = Accepted;
      break;
      case KDevSystemMessage::ActionDenied:
      stat = Denied;
      break;
      default:
      stat = Unknown;
      break;
    }
  }

  if ( emitter.data() )
    emitter->updateMessageInfo( this );

  return ReplyResult();
}

PatchRequestData::~PatchRequestData() {
}

#include "patchmessage.moc"

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on

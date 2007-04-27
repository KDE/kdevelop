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

#include "filecollaborationsession.h"

Q_DECLARE_METATYPE( Teamwork::UserPointer );

#include <sstream>
#include <QAction>
#include <QMenu>
#include <QTimer>
#include <QStandardItemModel>
#include <QModelIndex>

#include "idocumentcontroller.h"
#include <idocument.h>

#include <krandom.h>
#include <ktexteditor/document.h>
#include <ktexteditor/cursor.h>

#include "network/messagesendhelper.h"
#include "network/messagetypeset.h"

#include "filecollaborationmanager.h"
#include "dynamictext/vectortimestamp.h"
#include "kdevteamwork_user.h"
#include "documentwrapper.h"
#include "patchesmanager.h"
#include "kdevteamwork_helpers.h"


#include "filesynchronizemessage.h"
#include "teamworkfoldermanager.h"

using namespace KDevelop;

CROSSMAP_KEY_EXTRACTOR( FileCollaborationPointer, FileCollaborationPointer, 0, value );
CROSSMAP_KEY_EXTRACTOR( FileCollaborationPointer, UserPointer, 0, value->user() );

CROSSMAP_KEY_EXTRACTOR( DocumentWrapperPointer, QString, 0, value->fileName() );
CROSSMAP_KEY_EXTRACTOR( DocumentWrapperPointer, uint, 0, value->id() );

FileCollaborationSession::FileCollaborationSession( QString name, CollabFileList files, FileCollaborationManager* manager, uint primaryIndex, CollabSessionId id ) :
    SafeLogger( manager->teamwork() ->logger(), "in FileCollaborationSession:" ),
    m_manager( manager ),
    m_allowSentDocuments( false ),
    m_isMasterSession( primaryIndex == 0 ),
    m_primaryIndex( primaryIndex ),
    m_indexCount( primaryIndex ? 0 : 1 ),
    m_wrapperIndexCount( primaryIndex ? 0 : 1 ),
    m_name( name ),
    m_dispatcher( *this ) {
  m_publishFileListTimer = new QTimer( this );
  connect( m_publishFileListTimer, SIGNAL( timeout() ), this, SLOT( slotPublishFileList() ) );
  m_publishFileListTimer->setSingleShot( true );
  m_stateChangeTimer = new QTimer( this );
  connect( m_stateChangeTimer, SIGNAL( timeout() ), this, SLOT( publishStateChange() ) );
  m_publishFileListTimer->setSingleShot( true );

  connect( m_manager, SIGNAL( fillCollaboratingUserMenu( QMenu*, const UserPointer& ) ), this, SLOT( slotFillCollaboratingUserMenu( QMenu*, const UserPointer& ) ) );

  m_removeUserAction = new QAction( IconCache::getIconStatic( "remove" ), "Remove User", this );
  connect( m_removeUserAction, SIGNAL( triggered( bool ) ), this, SLOT( removeUserAction() ) );

  if ( id != 0 )
    m_id = id;
  else {
    QString str = KRandom::randomString( 8 ).toLatin1();
    m_id = *( ( CollabSessionId* ) & ( str.toAscii().data() [ 0 ] ) ); ///Not a good way of generating a 64-bit random-number..
  }

  out( Logger::Debug ) << "starting session";

  for ( CollabFileList::iterator it = files.begin(); it != files.end(); ++it ) {
    addFileInternal( *it, true, m_isMasterSession );
  }

}

void FileCollaborationSession::slotConnectionResult( FileCollaboration* collab, bool result ) {
  if ( result && collab->invited() ) {
    for ( FileSet::Iterator it = m_files.values(); it; ++it ) {
      synchronizeFile( collab, *it );
    }
  }

  emit stateChanged( this );
}

void FileCollaborationSession::setAllowSentDocuments( bool allow ) {
  m_allowSentDocuments = allow;
}


bool FileCollaborationSession::synchronizeFile( const FileCollaborationPointer& collab, const DocumentWrapperPointer& wrapper ) {
  return wrapper->synchronize( collab->user() );
}

QStringList FileCollaborationSession::plainFileNames() const {
  QStringList ret;
  for ( FileSet::ValueMap::const_iterator it = m_files.begin(); it != m_files.end(); ++it )
    ret << ( *it ).second.value->fileName();
  return ret;
}

QList<uint> FileCollaborationSession::plainIds() const {
  QList<uint> ret;
  for ( FileSet::ValueMap::const_iterator it = m_files.begin(); it != m_files.end(); ++it )
    ret << ( *it ).second.value->id();
  return ret;
}

void FileCollaborationSession::removeUserAction() {
  try {
    QAction* act = qobject_cast<QAction*>( sender() );
    if( !act ) throw "no sender-action";

    QVariant v = act->data();
    UserPointer u = v.value<UserPointer>();
    if( !u ) throw "could not get user";

    FileCollaborationPointer collab = m_collaborations.value( u );
    if( !collab ) throw "no file-collaboration for the specified user";

    removeCollaboration( collab );
  } catch( const char* str ) {
    err() << "removeUserAction: " << str;
  } catch( const QString& str ) {
    err() << "removeUserAction: " << str;
  }
}

void FileCollaborationSession::removeCollaboration( const FileCollaborationPointer& collab ) {
  collab->close( "goodbye" );
  m_collaborations.remove( collab );
  emit stateChanged( this  );
}


CollabFileList FileCollaborationSession::fileNames() const {
  CollabFileList ret;
  for ( FileSet::ValueMap::const_iterator it = m_files.begin(); it != m_files.end(); ++it ) {
    ret.push_back( CollabFile( ( *it ).second.value->id(), ( *it ).second.value->fileName() ) );
  }
  return ret;
}

const FileCollaborationSession::FileSet& FileCollaborationSession::files() const {
  return m_files;
}


void FileCollaborationSession::fillContextMenu( int /*i*/, QMenu* menu ) {
  IDocument * d = KDevTeamwork::documentController() ->activeDocument();
  if ( d && d->textDocument() ) {
    KUrl u = TeamworkFolderManager::workspaceRelative( d->url().path() );

    if ( !m_files.values( u.path() ) )
      menu->addAction( "Add Current File", this, SLOT( addFile() ) );
  }
  QMenu* m = new QMenu( menu );
  m->setTitle( "Save Result" );
  m->addAction( "As Files", this, SLOT( saveAsFiles() ) );
  m->addAction( "As Patch", this, SLOT( saveAsPatch() ) );
  menu->addSeparator();
  FileSet::Iterator it = m_files.values();
  if ( m_isMasterSession && it ) {
    while ( it && ( *it ) ->dead() )
      ++it;
    if ( it ) {
      if ( ( *it ) ->disabled() )
        menu->addAction( "Enable Editing", this, SLOT( enableEditing() ) );
      else
        menu->addAction( "Disable Editing", this, SLOT( disableEditing() ) );
    }
  }
  m = new QMenu( menu );
  m->setTitle( "Remove" );
  m->addAction( "All Users", this, SLOT( stopSession() ) );
  m->addAction( "All Files", this, SLOT( removeAllFilesSession() ) );
  menu->addSeparator();
  menu->addAction( "Close", this, SLOT( closeSession() ) );
}

bool FileCollaborationSession::acceptMessage( const FileCollaborationRequestPointer& msg ) {
  try {
    FileCollaborationRequestPointer::Locked l = msg;
    if ( !l )
      throw "could not lock message";

    KDevTeamworkUserPointer u = l->info().user().cast<KDevTeamworkUser>();
    if ( !u )
      throw "message has no user";

    FileCollaborationPointer collab = newCollaboration( u, msg );
    if ( !collab )
      throw "could not create collaboration";

    return true;
  } catch ( const QString & str ) {
    err() << "error in acceptMessage: " << str;
    return false;
  } catch ( const char * str ) {
    err() << "error in acceptMessage: " << str;
    return false;
  }
  return false;
}

DocumentWrapperPointer FileCollaborationSession::getDocument( QString fileName ) {
  if ( ! m_files.values( fileName ) ) {
    out( Logger::Warning ) << "getDocument: Document-wrapper for a file not being collaborated was requested: " << fileName;
    return 0;
  }

  return m_files[ fileName ];
}

std::string FileCollaborationSession::logPrefix() {
  ostringstream str;
  str << "FileCollaborationSession(" << name() << ", " << id() << "): ";
  return str.str();
}

FileCollaborationPointer FileCollaborationSession::newCollaboration( const KDevTeamworkUserPointer & u, const FileCollaborationRequestPointer& request ) {
  try {
    FileCollaborationPointer ret = new FileCollaboration( u, this, request );
    connect( ret, SIGNAL( connectionResult( FileCollaboration*, bool ) ), this, SLOT( slotConnectionResult( FileCollaboration*, bool ) ) );
    m_collaborations.insert( ret );
    return ret;
  } catch ( const QString & str ) {
    err() << "error while creating file-collaboration: " << str;
    return 0;
  }
}

void FileCollaborationSession::updateTree( QModelIndex& i, QStandardItemModel* model ) {
  QMap<DocumentWrapperPointer, QPersistentModelIndex> positions;
  QMap<FileCollaborationPointer, QPersistentModelIndex> userPositions;
  try {
    if ( model->columnCount( i ) == 0 )
      model->insertColumn( 0, i );
    ///Locate all files that are already there
    for ( int a = model->rowCount( i ) - 1; a >= 0; a-- ) {
      QModelIndex ind = model->index( a, 0, i );
      QVariant v = model->data( ind, Qt::UserRole );
      if ( v.canConvert<CollaborationTreeActionPointer>() ) {
        StandardCollaborationTreeAction<DocumentWrapper>* d = dynamic_cast< StandardCollaborationTreeAction<DocumentWrapper>* >( v.value<CollaborationTreeActionPointer>().data() );
        if ( d && d->target ) {
          if ( m_files.values<DocumentWrapperPointer>( ( DocumentWrapper* ) d->target ) ) {
            positions[ ( DocumentWrapper* ) d->target ] = ind;
          } else {
            ///Remove the old item
            model->removeRow( a, i );
          }
        }
        FileCollaborationUserTreeAction * userAction = dynamic_cast< FileCollaborationUserTreeAction* >( v.value<CollaborationTreeActionPointer>().data() );
        if( userAction ) {
          if( userAction->target && m_collaborations.value( (FileCollaborationPointer)userAction->target ) ) {
            ///user is still collaborating
            userPositions[(FileCollaborationPointer)userAction->target] = ind;
          } else {
            ///User must be removed
            model->removeRow( a, i );
          }
        }
      } else {
        model->removeRow( a, i );
        err() << "unexepected entry-type in tree, removing";
      }
    }
    ///Add missing files
    for ( FileSet::ValueMap::iterator it = m_files.begin(); it != m_files.end(); ++it ) {
      if ( !positions.contains( ( *it ).second.value ) ) {
        model->insertRow( 0, i ); ///Documents are inserted at the top, users at the bottom.
        QModelIndex ind = model->index( 0, 0, i );
        ;
        if ( !ind.isValid() )
          throw "index-error";
        positions[ ( *it ).second.value ] = ind;
      }
    }

    ///Add missing users
    for( CollaborationSet::Iterator it = m_collaborations.values(); it; ++it ) {
      if ( !userPositions.contains( *it ) ) {
        int num = model->rowCount( i );
        model->insertRow( num, i );
        QModelIndex ind = model->index( num, 0, i );

        if ( !ind.isValid() )
          throw "index-error";
        userPositions[ *it ] = ind;
      }
    }

    ///Update documents
    for ( QMap<DocumentWrapperPointer, QPersistentModelIndex>::iterator it = positions.begin(); it != positions.end(); ++it ) {
      QModelIndex i( *it );
      const_cast<DocumentWrapperPointer&>( it.key() ) ->updateTree( i, model );
    }

    ///Update users
    for ( QMap<FileCollaborationPointer, QPersistentModelIndex>::iterator it = userPositions.begin(); it != userPositions.end(); ++it ) {
      QModelIndex i(*it);
      it.key()->updateTree( i , model );

    }

  } catch ( const char * str ) {
    err() << "in updateTree: " << str;
  } catch ( const QString & str ) {
    err() << "in updateTree: " << str;
  }
}

QIcon FileCollaborationSession::icon() const {
  if ( state() == Finished )
    return IconCache::getIconStatic( "collaboration_session_finished" );
  else {
    if ( !m_isMasterSession ) {
      CollaborationSet::Iterator it = m_collaborations.values();
      if ( !( *it ) ->connectionActive() )
        return IconCache::getIconStatic( "collaboration_session_connectionlost" );
    }
    return IconCache::getIconStatic( "collaboration_session" );
  }
}

bool FileCollaborationSession::isMasterSession() const {
  return m_isMasterSession;
}

uint FileCollaborationSession::primaryIndex() const {
  return m_primaryIndex;
}

FileCollaborationSession::State FileCollaborationSession::state() const {
  if ( !m_collaborations.empty() )
    return Running;
  return Finished;
}

bool FileCollaborationSession::removeFile( uint id ) {
  DocumentWrapperPointer wrapper = m_files[ id ];
  if ( !wrapper )
    return false;
  ///@todo ask if content should be saved or if the file should stay completely
  m_files.remove( wrapper );

  publishFileRemove( id );
  m_stateChangeTimer->start( 100 );
  return true;
}

bool FileCollaborationSession::killFile( uint id ) {
  DocumentWrapperPointer wrapper = m_files[ id ];
  if ( !wrapper )
    return false;
  ///@todo ask if content should be saved or if the file should stay completely
  wrapper->setDisabled( true );
  wrapper->setDead( true );

  m_stateChangeTimer->start( 100 );
  return true;
}

uint FileCollaborationSession::addFileInternal( const CollabFile& f, bool fromBuffer, bool readFile ) {
  try {
    if ( f.id > m_wrapperIndexCount )
      m_wrapperIndexCount = f.id + 1;
    QString file = f.file;
    /*if ( KDevCore::projectController() ->activeProject() ) {
      KUrl u( file );

      if ( u.isParentOf( KDevCore::projectController() ->activeProject() ->projectDirectory() ) ) {
        QString d = KDevCore::projectController() ->activeProject() ->projectDirectory().path();
        if( !d.endsWith( "/" ) ) d += "/";
        file = KUrl::relativeUrl( d, u );
      }
    }*/

    DocumentWrapperPointer p = new DocumentWrapper( f.id, m_manager->teamwork() ->logger(), file, m_primaryIndex, readFile, this, fromBuffer );
    connect( p.data(), SIGNAL( publishEdit( const VectorTimestamp&, const SimpleReplacement&, DocumentWrapper* ) ), this, SLOT( publishEdit( const VectorTimestamp&, const SimpleReplacement&, DocumentWrapper* ) ) );
    m_files.insert( p );
    m_publishFileListTimer->start( 100 );
    m_stateChangeTimer->start( 100 );
    return f.id;
  } catch ( const QString & str ) {
    err() << "failed to create document-wrapper for " << f.file << ": " << str;
    return 0;
  }
}

void FileCollaborationSession::saveAsFiles() {
  FileSet::Iterator it = m_files.values();
  while ( it ) {
    ( *it ) ->saveAsBufferFile();

    ++it;
  }
}

void FileCollaborationSession::saveAsPatch() {
  FileSet::Iterator it = m_files.values();
  QList<LocalPatchSourcePointer> patches;
  while ( it ) {

    LocalPatchSourcePointer p = ( *it ) ->saveAsPatch( false, false );

    ++it;
  }

  QString patchName = name();

  if ( !m_isMasterSession ) {
    CollaborationSet::Iterator it = m_collaborations.values();
    if ( ( *it ) ->user() )
      patchName += "@" + ~( *it ) ->user().unsafe() ->safeName() + "_" + QDateTime::currentDateTime().toString( Qt::ISODate );
  }

  LocalPatchSourcePointer p = manager() ->teamwork() ->patchesManager() ->merge( patchName, patches );
  if ( p ) {
    manager() ->teamwork() ->patchesManager() ->addPatch( p );
  } else {
    err() << "could not merge the file-patches to one";
  }
}
void FileCollaborationSession::addFile() {
  try {
    IDocument * document = KDevTeamwork::documentController() ->activeDocument();
    Q_ASSERT( document != 0 );
    Q_ASSERT( document->textDocument() != 0 );

    KUrl u = TeamworkFolderManager::workspaceRelative( document->url().path() );

    out( Logger::Debug ) << "adding " << u.path() << " to the session";

    uint index = allocateWrapperIndex();
    QString fileName = TeamworkFolderManager::relative( u.path() );

    if ( !addFileInternal( CollabFile( index, fileName ), true, true ) )
      throw "could not add file " + u.path();

    DocumentWrapperPointer wrapper = m_files[ index ];
    if ( !wrapper )
      throw "could not get document-wrapper for " + fileName;

    ///Synchronize the newly created file
    for ( CollaborationSet::Iterator it = m_collaborations.values(); it; ++it )
      synchronizeFile( *it, wrapper );


  } catch ( const QString & str ) {
    err() << "addFile(): " << str;
  } catch ( const char * str ) {
    err() << "addFile(): " << str;
  }
}

void FileCollaborationSession::stopSession() {
  out() << "stopping session, disconnecting all users";
  CollaborationSet::Iterator it = m_collaborations.values();
  while ( it ) {
    it.edit() ->data() ->close( "stop" );
    ++it;
  }
  m_collaborations.clear();

  emit stateChanged( this );
}

void FileCollaborationSession::disableEditing() {
  FileSet::Iterator it = m_files.values();
  while ( it ) {
    if ( !( *it ) ->dead() ) {
      ( *it ) ->setDisabled( true );
    }
    ++it;
  }
}

void FileCollaborationSession::enableEditing() {
  FileSet::Iterator it = m_files.values();
  while ( it ) {
    if ( !( *it ) ->dead() ) {
      ( *it ) ->setDisabled( false );
    }
    ++it;
  }
}

void FileCollaborationSession::removeAllFiles() {
  FileSet::Iterator it = m_files.values();
  while ( it ) {
    removeFile( ( *it ) ->id() );
    ++it;
  }
}

void FileCollaborationSession::closeSession() {
  manager() ->closeSession( this );
}

void FileCollaborationSession::publishStateChange() {
  emit stateChanged( this );
}

QAction* FileCollaborationSession::getRemoveUserAction( const UserPointer& user ) {
  QVariant v;
  v.setValue( user );
  m_removeUserAction->setData( v );
  return m_removeUserAction;
}

void FileCollaborationSession::publishFileRemove( uint id ) {
  CollabFileList files = fileNames();
  files.push_back( CollabFile( id, "" ) );
  publishFileList( files );
}

void FileCollaborationSession::slotPublishFileList() {
  publishFileList( fileNames() );
}

void FileCollaborationSession::publishFileList( const CollabFileList& files ) {
  for ( CollaborationSet::Iterator it = m_collaborations.values(); it; ++it ) {
    FileCollaborationPointer collab = *it;
    KDevTeamworkUserPointer::Locked l = collab->user();
    if ( l ) {
      if ( l->online().session() ) {
        globalMessageSendHelper().send<FileListMessage>( l->online().session().unsafe(), files, id() );
      }
    } else {
      err() << "publishFileList(..): could not lock user";
    }
  }
}

void FileCollaborationSession::publishEdit( const VectorTimestamp& state, const SimpleReplacement& replacement, DocumentWrapper* sender ) {
  for ( CollaborationSet::ValueMap::iterator it = m_collaborations.begin(); it != m_collaborations.end(); ++it ) {
    FileCollaborationPointer collab = ( *it ).second.value;
    if ( state.primaryIndex() != collab->primaryIndex() ) {
      KDevTeamworkUserPointer::Locked l = collab->user();
      if ( l ) {
        if ( l->online().session() ) {
          globalMessageSendHelper().send<FileEditMessage>( l->online().session().unsafe(), state, replacement, sender->id(), id() );
        }
      } else {
        err() << "publishEdit(..): could not lock user";
      }
    }
  }
}

void FileCollaborationSession::slotFillCollaboratingUserMenu( QMenu * /*menu*/, const UserPointer & /*user*/ ) {}

CollabSessionId FileCollaborationSession::id() const {
  return m_id;
}

QList< KDevTeamworkUserPointer > FileCollaborationSession::users() const {
  QList<KDevTeamworkUserPointer> ret;
  for ( CollaborationSet::ValueMap::const_iterator it = m_collaborations.begin(); it != m_collaborations.end(); ++it ) {
    ret << ( *it ).second.value->user();
  }
  return ret;
}

void FileCollaborationSession::inviteUser( const KDevTeamworkUserPointer & user ) {
  try {
    if ( !m_isMasterSession )
      throw "cannot invite user to a remote session";

    KDevTeamworkUserPointer::Locked lu = user;
    if ( !lu )
      throw "could not lock user";

    if ( !lu->online().session() )
      throw "no session to user " + ~lu->User::name();

    SessionPointer session = lu->online().session();

    FileCollaborationPointer collab = newCollaboration( user );

  } catch ( const char * str ) {
    err() << "inviteUser: " << str;
  } catch ( QString str ) {
    err() << "inviteUser: " << str;
  }
}

QString FileCollaborationSession::firstPeerName() {
  CollaborationSet::ValueMap::iterator it = m_collaborations.begin();
  if ( it != m_collaborations.end() ) {
    KDevTeamworkUserPointer::Locked l = ( *it ).second.value->user();
    if ( l ) {
      return ~l->User::name();
    } else {
      err() << "firstPeerUserName() could not lock user";
      return "";
    }
  }
  return "";
}

uint FileCollaborationSession::allocateIndex() {
  if ( m_primaryIndex != 0 ) {
    err() << "allocateIndex() called although session is not master";
  }
  return m_indexCount++;
}

uint FileCollaborationSession::allocateWrapperIndex() {
  /*if ( m_primaryIndex != 0 ) {
    err() << "allocateWrapperIndex() called although session is not master";
  }*/

  uint ret = 0;
  if ( m_isMasterSession )
    ret = m_wrapperIndexCount++;
  else
    ret = m_wrapperIndexCount++ + KRandom::random() % 10 + 5; ///Return a higher number to avoid conflicts
  if( m_files[ret] )
    return allocateWrapperIndex();
  else
    return ret;

}

void FileCollaborationSession::kickUser( const KDevTeamworkUserPointer & /*user*/ ) {}

QString FileCollaborationSession::name() const {
  return m_name;
}

FileCollaborationManager * FileCollaborationSession::manager() {
  return m_manager;
}
/*
void FileCollaborationSession::processMessage( const FileCollaborationMessagePointer& msg ) {
  try {
    FileCollaborationMessagePointer::Locked lmsg = msg;
    if ( !lmsg )
      throw "could not lock message";

    UserPointer u = lmsg->info().user();
    if ( !u )
      throw QString( "no user-information" );

    FileCollaborationPointer collab = m_collaborations[ u ];
    if ( !collab )
      throw ~( "got message from not involved user \"" + u.unsafe() ->safeName() + "\"" );

    collab->processMessage( msg );

  } catch ( QString str ) {
    err() << "could not process message: " << str;
  }
}*/
int FileCollaborationSession::receiveMessage( MessageInterface* /*msg*/ ) {
  err() << "got unknown message-type";
  return 1;
}

int FileCollaborationSession::receiveMessage( FileListMessage* msg ) {
  for ( CollabFileList::iterator it = msg->m_files.begin(); it != msg->m_files.end(); ++it ) {
    FileSet::Iterator doc = m_files.values( it->id );
    if ( doc ) {
      if ( it->file.isEmpty() ) {
        if( m_isMasterSession ) {
          out( Logger::Debug ) << "file " << it->file << " was tried to be removed from the session";
        } else {
          out( Logger::Debug ) << "file " << it->file << " has been removed from the session";
          killFile( it->id );
        }
      }
    } else {
      out( Logger::Warning ) << "unknown reference in file-list-message: " << it->id << " " << it->file;
      /*if ( !it->file.isEmpty() ) {
        ///Add the new file
        addFileInternal( *it, false, false );
      } else {
        out( Logger::Warning ) << "unknown referenced in file-list-message: " << it->id;
      }*/
    }
  }
  return 1;
}

int FileCollaborationSession::receiveMessage( FileCollaborationMessage* msg ) {
  switch ( msg->message() ) {
    case FileCollaborationMessageData::CloseSession:
      if ( !m_isMasterSession ) {
        out() << "finishing session";
        aboutToClose();
      } else {
        CollaborationSet::Iterator it = m_collaborations.values( msg->info().user() );
        if ( it ) {
          it.edit() ->data() ->close( "requested" );
          m_collaborations.remove( it );
        } else {
          err() << "got message from a user that is not part of the collaboration-session";
        }
      }
    default:
    out() << "got unhandled file-collaboration-message: " << msg->messageAsString();
  }
  return 1;
}

int FileCollaborationSession::receiveMessage( DocumentWrapperMessage* msg ) {
  CollaborationSet::Iterator sender = m_collaborations.values( msg->info().user() );
  if ( !sender ) {
    err() << "got FileSynchronize from a not collaborating user";
    return 0;
  }

  FileSet::Iterator it = m_files.values<uint>( msg->wrapperId() );
  if ( it ) {
    it.edit() ->data() ->processMessage( msg );
  } else {
    FileSynchronize* smsg = dynamic_cast<FileSynchronize*>( msg );
    if ( smsg ) {
      ///This has to be unterstood as an invitation to add a new document.
      if ( m_isMasterSession && !m_allowSentDocuments ) {
        globalMessageSendHelper().sendReply<KDevSystemMessage>( msg, KDevSystemMessage::ActionDenied );
        QString userName = "anonymous user";
        if ( msg->info().user() )
          userName = ~msg->info().user().unsafe() ->safeName();
        out( Logger::Warning ) << "got a file-synchronization from " << userName << " for \"" << smsg->fileName() << "\", but new files from collaborators are not allowed.";
        return 1;
      }
      if ( addFileInternal( CollabFile( smsg->wrapperId(), smsg->fileName() ), false, false ) ) {
        DocumentWrapperPointer wrapper = m_files[ smsg->wrapperId() ];
        if ( !wrapper ) {
          err() << "could not create wrapper";
          globalMessageSendHelper().sendReply<KDevSystemMessage>( msg, KDevSystemMessage::ActionFailed );
          return 1;
        }
        wrapper->processMessage( smsg );

        if ( m_isMasterSession ) {
          ///publish the new file to all other clients
          for ( CollaborationSet::Iterator it = m_collaborations.values(); it; ++it ) {
            if ( sender == it )
              continue;
            synchronizeFile( *it, wrapper );
          }
        }
      } else {
        err() << "failed to add file on synchronization: " << smsg->fileName();
        globalMessageSendHelper().sendReply<KDevSystemMessage>( msg, KDevSystemMessage::ActionFailed );
      }
    } else {
      out( Logger::Warning ) << "could not locate the correct document-wrapper for a message of type " << msg->name() << " wrapper-id: " << msg->wrapperId();
    }
  }
  return 1;
}

int FileCollaborationSession::processMessage( FileCollaborationMessage* msg ) {
  return m_dispatcher( msg );
}

void FileCollaborationSession::aboutToClose() {
  stopSession();
}

#include "filecollaborationsession.moc"


// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on

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

#include "filecollaborationmanager.h"
#include <QAction>
#include <QMenu>
#include <QTimer>
#include <QFileInfo>
#include <QModelIndex>
#include <QStandardItemModel>

#include <ktexteditor/document.h>
#include <ktexteditor/cursor.h>

#include "idocumentcontroller.h"

#include "network/messagesendhelper.h"
#include "network/messagetypeset.h"
#include "network/sessioninterface.h"

#include "dynamictext/verify.h"

#include "teamworkfwd.h"
#include "patchesmanager.h"
#include "kdevteamwork_user.h"
#include "kdevutils.h"
#include "documentwrapper.h"
#include "kdevteamwork_helpers.h"
#include "ui_kdevteamwork_filecollaborationsession.h"

using namespace Teamwork;

CROSSMAP_KEY_EXTRACTOR( FileCollaborationSessionPointer, QList<QString>, 0, value->plainFileNames() )
CROSSMAP_KEY_EXTRACTOR( FileCollaborationSessionPointer, QList<KDevTeamworkUserPointer>, 0, value->users() )
CROSSMAP_KEY_EXTRACTOR( FileCollaborationSessionPointer, SessionName, 0, value->name() )
CROSSMAP_KEY_EXTRACTOR( FileCollaborationSessionPointer, FileCollaborationSessionPointer, 0, value )
CROSSMAP_KEY_EXTRACTOR( FileCollaborationSessionPointer, CollabSessionId, 0, value->id() )

/**How file-collaboration should work:
 * Host:
 * - Once developer starts a collaboration-session on an arbitrary set of files, he can invite an arbitrary count of collaborating developers to join it.
 * - He can only host one collaboration-session at a time.
 * - Once the session is closed, he is asked whether he'd like to store the changes of the session as a patch to the patches-list(that way he may also unapply the changes if he saved them).
 * Client:
 * - A client gets an invitation and can accept it.
 * - The client can only be part of one collaboration-session at a time.
 * - Once the session is closed, he is asked whether the result of the session should be stored as a diff-file to the patches-list.(that way it may be applied to the local tree)
 *
 * Implementation: See DynamicText
 *
 * */

/**Algorithm for the FileCollaboration:
 * - Each FileCollaboration-message that is sent gets a personal sequence-number(which is like a timestamp, the "count of locally happened events")
 *   that can be used to see which state of the communication it was sent in(and can be used to resolve causalities)
 * - Each FileCollaboration-session has a vector containing all current sequence-numbers
 * - Each FileCollaboration-message is sent together with that vector, so conflicts may be resolved.
 * - Whenever the master-session sends a FileResynchronize-message, the local version of the slave-session is totally updated
 * */


Q_DECLARE_METATYPE( Teamwork::UserPointer )



FileCollaborationManager::FileCollaborationManager( CollaborationManager* manager ) :
    SafeLogger( manager->teamwork() ->logger(),
        "FileCollaborationManager: " ),
    m_manager( manager ),
m_dispatcher( *this ) {
  connect( m_manager, SIGNAL( fillCollaboratingUserMenu( QMenu*, const UserPointer& ) ), this, SLOT( slotFillCollaboratingUserMenu( QMenu*, const UserPointer& ) ) );
  m_startCollaborationSessionAction = new QAction( "Start File-Collaboration", this );
  connect( m_startCollaborationSessionAction, SIGNAL( triggered() ), this, SLOT( slotStartCollaborationSession() ) );
  connect( manager, SIGNAL( updateModel( QStandardItemModel* ) ), this, SLOT( updateCollaborationModel( QStandardItemModel* ) ) );

}

KDevTeamwork* FileCollaborationManager::teamwork() {
  return m_manager->teamwork();
}

const FileCollaborationManager::SessionSet& FileCollaborationManager::sessions() {
  return m_sessions;
}

FileCollaborationSession * FileCollaborationManager::startSession( const QString& name, CollabFileList files, uint primaryIndex, CollabSessionId id ) {
  FileCollaborationSessionPointer s = new FileCollaborationSession( name, files, this, primaryIndex, id );
  m_sessions.insert( s );

  connect( s.data(), SIGNAL( stateChanged( const FileCollaborationSessionPointer& ) ), this, SLOT( slotSessionStateChanged( const FileCollaborationSessionPointer& ) ) );

  return s;
}

void FileCollaborationManager::updateCollaborationModel( QStandardItemModel* model ) {

  ///Add/update sessions
  QMap< FileCollaborationSessionPointer, QPersistentModelIndex > sessions;

  for ( int r = model->rowCount() - 1; r >= 0 ; --r ) {
    QModelIndex i = model->index( r, 0 );
    if ( !i.isValid() )
      continue;
    QVariant v = i.data( Qt::UserRole );
    if ( v.canConvert<CollaborationTreeActionPointer>() ) {
      CollaborationTreeActionPointer action = v.value<CollaborationTreeActionPointer>();
      StandardCollaborationTreeAction<FileCollaborationSession> *session = dynamic_cast< StandardCollaborationTreeAction<FileCollaborationSession>* >( action.data() );
      if( session ) {
        if ( !session->target ) {
        model->removeRow( r );
      } else {
        sessions[ (FileCollaborationSession*)session->target ] = i;
      }
      }
    }
  }

  SessionSet::ValueMap::const_iterator it = m_sessions.begin();
  for ( ; it != m_sessions.end(); ++it ) {
    FileCollaborationSessionPointer session = ( *it ).second.value;
    QModelIndex i;
    if ( sessions.contains( session ) ) {
      i = sessions[ session ];
    } else {
      model->insertRow( 0 );
      i = model->index( 0, 0 );
      sessions[ session ] = QPersistentModelIndex( i );
      disconnect( session.data(), SIGNAL( stateChanged( const FileCollaborationSessionPointer& ) ), m_manager, SLOT( sessionStateChanged( const FileCollaborationSessionPointer& ) ) );
      connect( session.data(), SIGNAL( stateChanged( const FileCollaborationSessionPointer& ) ), m_manager, SLOT( sessionStateChanged( const FileCollaborationSessionPointer& ) ) );
    }

    QIcon icon = session->icon();
    model->setData( i, session->name(), Qt::DisplayRole );
    model->setData( i, icon, Qt::DecorationRole );

    /*QVariant
    model->setData( i, icon, Qt::DecorationRole );*/
    QVariant v;
    v.setValue<CollaborationTreeActionPointer>( new StandardCollaborationTreeAction<FileCollaborationSession>( session ) );
    model->setData( i, v, Qt::UserRole );

    session->updateTree( i, model );
  }
}

void FileCollaborationManager::slotFillCollaboratingUserMenu( QMenu * menu, const UserPointer & user ) {
  if ( m_sessions.empty() ) {
    QVariant v;
    v.setValue( user );
    m_startCollaborationSessionAction->setData( v );

    menu->addAction( m_startCollaborationSessionAction );
  }

  emit fillCollaboratingUserMenu( menu, user );
}



void FileCollaborationManager::slotStartCollaborationSession() {
  try {
    QAction * act = qobject_cast<QAction*>( sender() );
    if ( !act )
      throw "no action";

    QVariant v = act->data();
    if ( !v.canConvert<UserPointer>() )
      throw "wrong data";

    UserPointer::Locked lu = v.value<UserPointer>();
    if ( !lu )
      throw "could not lock user";

    CollabFileList files;

    files.push_back(CollabFile(0, currentDocumentPath() ) );

    Ui_NewFileCollaborationSession s;
    QDialog d;
    s.setupUi( &d );
    s.sessionName->setText( "Collaborate_on_" + QFileInfo( files.front().file ).baseName() );
    QString filesText = "Files:";
    for( CollabFileList::iterator it = files.begin(); it != files.end(); ++it ) {
      filesText += "\n" + it->file;
    }
    s.files->setText(  filesText );

    QString usersText = "Invite users:";
    usersText += "\n" + ~lu->name();
    s.users->setText(  usersText );

    if( d.exec() == QDialog::Accepted ) {
      QString name = s.sessionName->text();
      FileCollaborationSessionPointer p = startSession( name, files );

      p->setAllowSentDocuments( s.allowSentDocuments->isChecked() );
      p->inviteUser( lu.freeCast<KDevTeamworkUser>() );
    }
  } catch ( const char * str ) {
    err() << QString( "Error in slotStartCollaborationSession(): " ) + str;
  } catch ( QString str ) {
    err() << QString( "Error in slotStartCollaborationSession(): " ) + str;
  };
}

void FileCollaborationManager::processMessage( FileCollaborationMessagePointer msg ) {
  FileCollaborationMessagePointer::Locked l = msg;
  if ( l ) {
    if ( l->sessionId() == 0 || msg.cast<FileCollaborationRequest>() ) {
      m_dispatcher( l );
    } else {
      FileCollaborationSessionPointer s = m_sessions.value<CollabSessionId>( l->sessionId() );
      if ( s ) {
        s->processMessage( l.data() );
      } else {
        err() << "got message for unknown file-collaboration-session with id ~" << l->sessionId() <<", type: " << msg.unsafe()->name();
      }
    }
  } else {
    err() << "could not lock a FileCollaborationMessage";
  }
}

int FileCollaborationManager::receiveMessage( MessageInterface* msg ) {
  out( Logger::Warning ) << "got unknown message-type " << msg->name();
  return 0;
}

int FileCollaborationManager::receiveMessage( FileCollaborationRequest* msg ) {
  ///Since it is an AbstractGUIMessage, it can plug itself into the GUI and wait for an answer by the user.
  m_manager->teamwork() ->addMessageToList( msg );
  return 1;
}

int FileCollaborationManager::receiveMessage( FileCollaborationMessage* msg ) {
  SessionSet::Iterator it = m_sessions.values<CollabSessionId>( msg->sessionId() );
  if( it ) {
    return const_cast<FileCollaborationSession*>((*it).data())->processMessage( msg );
  } else {
    out( Logger::Warning ) << "got a FileCollaborationMessage of type " << msg->name() << " for a non-existent session: " << (uint)msg->sessionId();
    return 0;
  }
}

void FileCollaborationManager::slotSessionStateChanged( const FileCollaborationSessionPointer & session ) {
  m_sessions.update( session );
}

void FileCollaborationManager::denyCollaboration( const FileCollaborationRequestPointer& msg ) {
  FileCollaborationRequestPointer::Locked l = msg;
  if ( !l ) {
    err() << "denyCollaboration(): could not lock message";
    return;
  }

  std::pair< RequestMap::const_iterator, RequestMap::const_iterator > range = m_requests.equal_range( l->sessionId() );

  ///Deny all other requests for the same session
  bool ready = false;
  while( !ready ) {
    ready = true;
    std::pair< RequestMap::iterator, RequestMap::iterator > range = m_requests.equal_range( l->sessionId() );

    while( range.first != range.second ) {
      if( (*range.first).second == msg ) {
        m_requests.erase( range.first++ );
        continue;
      }
      FileCollaborationRequestPointer::Locked lm = (*range.first).second;
      if( lm ) {
        int cnt = m_requests.size();
        lm->denyCollaboration();
        if( m_requests.size() != (uint)cnt ) {
          ready = false;
          break;
        }
      }
      ++range.first;
    }
  }
}

bool FileCollaborationManager::acceptCollaboration( const FileCollaborationRequestPointer& msg ) {
  try {
    FileCollaborationRequestPointer::Locked l = msg;
    if ( !l )
      throw "could not lock message";

    //Q_VERIFY( l->index() !=  0 );

    FileCollaborationSessionPointer session;
    if( m_sessions.values( l->sessionId() ) ) {
      session = m_sessions[ l->sessionId() ];
    } else {
      QString name = l->FileCollaborationRequestData::name();
      if( name.isEmpty() ) {
        name = "anonymous session";
        if(  l->info().user() )
          name += "@" + ~l->info().user().unsafe()->safeName();
      }
      session = startSession( name, l->files(), l->index(), l->sessionId() );
      if ( !session )
        throw "could not create FileCollaborationSession";
    }

    session->acceptMessage( msg );

    ///Now pull in all other requests waiting for this session
    bool ready = false;
    while( !ready ) {
      ready = true;
      std::pair< RequestMap::iterator, RequestMap::iterator > range = m_requests.equal_range( l->sessionId() );

      while( range.first != range.second ) {
        if( (*range.first).second == msg ) {
          m_requests.erase( range.first++ );
          continue;
        }
        FileCollaborationRequestPointer::Locked lm = (*range.first).second;
        if( lm ) {
          int cnt = m_requests.size();
          lm->denyCollaboration();
          if( m_requests.size() != (uint)cnt ) {
            ready = false;
            break;
          }
        }
        ++range.first;
      }
    }

    out( Logger::Debug ) << "collaboration accepted";
    m_requests.erase( l->sessionId() );
    return true;
  } catch ( const QString & str ) {
    err() << "error in acceptCollaboration: " << str;
    return false;
  } catch ( const char * str ) {
    err() << "error in acceptCollaboration: " << str;
    return false;
  }
}

void FileCollaborationManager::closeSession( const FileCollaborationSessionPointer& session ) {
  session->aboutToClose();
  m_sessions.remove( session );
}


#include "filecollaborationmanager.moc"

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on



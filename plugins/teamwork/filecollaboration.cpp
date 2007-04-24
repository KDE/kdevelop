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

#include "collaborationmanager.h"
#include "filecollaborationmanager.h"
#include "kdevteamwork_user.h"
#include <QAction>
#include <QMenu>
#include "kdevutils.h"
#include <krandom.h>
#include "documentwrapper.h"
#include <ktexteditor/document.h>
#include <ktexteditor/cursor.h>
#include <QFileInfo>
#include "kdevteamwork_helpers.h"
#include <QStandardItemModel>
#include <QModelIndex>
#include "verify.h"

#include "filecollaboration.h"


FileCollaboration::FileCollaboration( KDevTeamworkUserPointer user, FileCollaborationSession* session, const FileCollaborationRequestPointer& request ) : SafeLogger( session->manager() ->teamwork() ->logger() ), m_user( user ), m_session( session ),  /*m_dispatcher( *this ),*/ m_index( 0 ), m_userConnected( false ), m_invited( false ), m_connectionActive( false ) {
  connect( session->manager(), SIGNAL( fillCollaboratingUserMenu( QMenu*, const UserPointer& ) ), this, SLOT( slotFillCollaboratingUserMenu( QMenu*, const UserPointer& ) ) );

  KDevTeamworkUserPointer::Locked l = user;
  if ( !l )
    throw QString( "in FileCollaboration(): could not lock user" );

  connect( l.data(), SIGNAL( userStateChanged( KDevTeamworkUserPointer ) ), this, SLOT( userStateChanged() ) );

  m_userName = ~l->User::name();

  if ( !l->online().session() )
    throw QString( "in FileCollaboration(): user is not online" );

  if ( !request ) {
    m_index = session->allocateIndex(); ///Create a new index
  } else {
    FileCollaborationRequestPointer::Locked lr = request;
    if ( !lr )
      throw QString( "could not lock request-message" );
    m_index = lr->senderIndex(); ///The other side is the master, so the index is zero.
  }

  invite( request, false );
}

uint FileCollaboration::primaryIndex() const {
  return m_index;
}

std::string FileCollaboration::logPrefix() {
  return ~( "FileCollaboration(" + m_userName + ", " + m_session->name() + "): " );
}

void FileCollaboration::processMessage( const MessagePointer& msg ) {
  MessagePointer::Locked l = msg;
  if ( l ) {
    //m_dispatcher( l );
  } else {
    err() << "could not lock a message";
  }
}
/*
int FileCollaboration::dispatchMessage( MessageInterface* msg ) {
  out( Logger::Warning ) << "got unknown message of type " << msg->name();
  return 0;
}
 
int FileCollaboration::dispatchMessage( FileCollaborationMessage* msg ) {
  ///@todo clean/remove
  out( Logger::Debug ) << "got FileCollaborationMessage";
  if ( msg->isDerived() ) {
    out( Logger::Warning ) << "dispatched a message of type " << msg->name() << " as FileCollaborationMessage";
    return 0;
  }
  try {
    switch ( msg->message() ) {
      case FileCollaborationMessageData::Synchronize: {
        if ( !msg->messageData().canConvert<QString>() )
          throw "wrong messageData in Synchronize-request(cannot extract filename)";
        QString fileName = msg->messageData().value<QString>();
 
        KDevTeamworkUserPointer::Locked l = m_user;
        if ( !l )
          throw "could not lock user";
 
        if ( !l->online().session() )
          throw "user is not online";
 
        if ( !l->online().session().getUnsafeData() ->isOk() )
          throw "user is not online, session is dead";
 
      }
      break;
      default:
      out( Logger::Warning ) << "got unhandled FileCollaborationMessage with message " << msg->messageAsString();
      break;
    }
  } catch ( const QString & str ) {
    err() << "in dispatchMessage( FileCollaborationMessage* ): " << str;
  } catch ( const char * str ) {
    err() << "in dispatchMessage( FileCollaborationMessage* ): " << str;
  }
  return 0;
}
 
int FileCollaboration::dispatchMessage( FileCollaborationRequest* msg ) {
  out( Logger::Debug ) << "got FileCollaborationRequest";
  return 0;
}*/

bool FileCollaboration::connectionActive() const {
  return m_connectionActive;
}

void FileCollaboration::userStateChanged() {
  try {
    KDevTeamworkUserPointer::Locked l = m_user;
    if ( !l )
      throw "could not lock user";

    if ( l->online() && l->online().session().getUnsafeData() ->isOk() ) {
      if ( !m_userConnected ) {
        ///Automatically re-invite the user to connect
        invite( 0, true );
      }
    } else {
      m_connectionActive = false;
      m_userConnected = false;

      emit connectionResult( this, false );
    }
  } catch ( const QString & str ) {
    err() << "error in userStateChanged(): " << str;
  } catch ( const char * str ) {
    err() << "error in userStateChanged(): " << str;
  }
}

void FileCollaboration::slotConnectionResult( bool result ) {
  out( Logger::Debug ) << "slotConnectionResult " << result;
  m_userConnected = result;
  if ( result )
    m_connectionActive = true;
  else
    m_connectionActive = false;
  emit connectionResult( this, result );
}

KDevTeamworkUserPointer FileCollaboration::user() const {
  return m_user;
}

FileCollaborationSessionPointer FileCollaboration::session() const {
  return m_session;
}

void FileCollaboration::slotFillCollaboratingUserMenu( QMenu * menu, const UserPointer & user ) {}

bool FileCollaboration::invited() const {
  return m_invited;
}

void FileCollaborationUserTreeAction::doubleClicked() {
  if ( target )
    target->doubleClicked();
}

void FileCollaborationUserTreeAction::fillContextMenu( QMenu* menu ) {
  if ( target )
    target->fillContextMenu( var, menu );
}

void FileCollaboration::fillContextMenu( int i, QMenu* menu ) {
  KDevTeamwork::self()->fillUserMenu( menu, user().cast<User>() );
  if( m_session->isMasterSession() ) {
    menu->addAction( m_session->getRemoveUserAction( m_user.cast<User>() ) );
    menu->addSeparator();
  }
}

void FileCollaboration::doubleClicked() {
  if ( m_user ) {
    KDevTeamwork::self() ->guiSendMessageTo( m_user );
  }
}

void FileCollaboration::invite( const FileCollaborationRequestPointer& request, bool automatic ) throw( QString ) {
  KDevTeamworkUserPointer::Locked l = m_user;
  if ( !l )
    throw QString( "in FileCollaboration(): could not lock user" );

  LockedSharedPtr<FileCollaborationRequest> lmsg = globalMessageTypeSet().create<FileCollaborationRequest>( this, m_index, m_session->primaryIndex(), m_session->id() );
  if ( !lmsg )
    throw QString( "in FileCollaboration(): could not create message" );

  Q_VERIFY_SAME( lmsg->sessionId(), m_session->id() );
  Q_VERIFY_SAME( lmsg->index(), m_index );

  if ( automatic ) {
    if ( m_index != 0 ) {
      out( Logger::Info ) << "re-inviting user to join the session";
    } else {
      out( Logger::Info ) << "inviting session-host to continue the session";
    }

    lmsg->setAutomatic( true );
  }

  m_request = lmsg.freeCast<FileCollaborationRequest>();


  if ( request ) {
    ///When answering to a request, this will tell the other side that the request was accepted.
    LockedSharedPtr<FileCollaborationRequest> lrequest = request;
    if ( !lrequest )
      throw QString( "in FileCollaboration(): could not lock request" );

    lmsg->info().setReply( lrequest->info().id().uniqueId() );
    lmsg->info().setReplyMessage( request );
  } else {
    m_invited = true;
  }
  l->online().session().getUnsafeData() ->sendMessage( lmsg );

  if ( !request && !automatic ) {
    m_session->manager() ->teamwork() ->addMessageToList( lmsg.data() );
  }

}

void FileCollaboration::close( const QString & reason ) {
  KDevTeamworkUserPointer::Locked l = m_user;
  if ( l && l->online().session() ) {
    globalMessageSendHelper().send<FileCollaborationMessage>( l->online().session().getUnsafeData(), m_session->id(), FileCollaborationMessageData::CloseSession );
  }
}

QIcon FileCollaboration::icon() const {
  if( m_connectionActive ) {
    KDevTeamworkUserPointer::Locked l = m_user;
    if( l ) 
      return l->icon();
    else
      return QIcon();
  } else
    return IconCache::getIconStatic( "fileusercollaboration_inactive" );
}

void FileCollaboration::updateTree( QModelIndex& i, QStandardItemModel* model ) {
  KDevTeamworkUserPointer::Locked l = user();
  if ( !l ) {
    model->setData( i, "lost user", Qt::DisplayRole );
    model->setData( i, QIcon(), Qt::DecorationRole ); ///Maybe use some other icons indicating the state of the collaboration
  } else {
    model->setData( i, ~l->User::name(), Qt::DisplayRole );
    model->setData( i, icon(), Qt::DecorationRole ); ///Maybe use some other icons indicating the state of the collaboration
    QVariant v;
    v.setValue<CollaborationTreeActionPointer>( new FileCollaborationUserTreeAction( this ) );
    model->setData( i, v, Qt::UserRole );
  }
}


#include "filecollaboration.moc"

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on


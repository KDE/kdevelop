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

#include "filecollaborationmessages.h"
#include "filecollaborationmanager.h"
#include "kdevteamwork.h"
#include "collaborationmanager.h"
#include <QMenu>
#include <ktexteditor/document.h>
#include "network/messagesendhelper.h"
#include "network/messagetypeset.h"
#include "kdevteamwork_helpers.h"
#include "safesignaller.h"
#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include "qtserialization.h"

/* Exclude this file from doublequote_chars check as krazy doesn't understand
std::string*/
//krazy:excludeall=doubleqote_chars

Q_DECLARE_METATYPE( MessagePointer )

FileCollaborationRequestData::FileCollaborationRequestData( FileCollaboration* collab, uint index, uint senderIndex ) : m_state( Unknown ), m_index( index ), m_senderIndex( senderIndex ), m_isAutomatic( false ), m_collab( collab ), m_acceptAction( 0 ), m_denyAction( 0 ) {

  if ( collab ) {
    m_files = collab->session() ->fileNames();
    if ( collab->session() ) {
      m_session = collab->session();
      m_name = collab->session() ->name();
      connect( ( QObject* ) this, SIGNAL( stateChanged( const MessagePointer& ) ), ( QObject* ) collab->session() ->manager() ->teamwork() ->messageManager(), SLOT( updateMessage( const MessagePointer& ) ), Qt::QueuedConnection );
      connect( ( QObject* ) this, SIGNAL( connectionResult( bool ) ), ( QObject* ) collab, SLOT( slotConnectionResult( bool ) ), Qt::QueuedConnection );
    }

    connect( this, SIGNAL( processReply( const MessagePointer& ) ), collab, SLOT( processMessage( const MessagePointer& ) ), Qt::QueuedConnection );

  }
}

uint FileCollaborationRequestData::index( ) const {
  return m_index;
}

uint FileCollaborationRequestData::senderIndex( ) const {
  return m_senderIndex;
}

bool FileCollaborationRequestData::isAutomatic() const {
  return m_isAutomatic;
}

void FileCollaborationRequestData::setAutomatic( bool automatic ) {
  m_isAutomatic = automatic;
}

CollabFileList FileCollaborationRequestData::files() const {
  return m_files;
}

void FileCollaborationRequestData::setFiles( const CollabFileList& theValue ) {
  m_files = theValue;
}

QString FileCollaborationRequestData::name() const {
  return m_name;
}

void FileCollaborationRequestData::setName( const QString& theValue ) {
  m_name = theValue;
}

QString FileCollaborationRequest::messageText() const {
  QString name = "unknown";
  if ( info().user() )
    name = ~info().user().unsafe() ->safeName();
  QString files;
  for( CollabFileList::const_iterator it = m_files.begin(); it != m_files.end(); ++it ) {
    if( !files.isEmpty() ) files += "\n";
    files += (*it).file;
  }
  if ( info().isIncoming() ) {
    return QString( "File-collaboration-request from %1 on:\n%2" ).arg( name ).arg( files );
  } else {
    return QString( "File-collaboration-request to %1 on:\n%2" ).arg( name ).arg( files );
  }
}

QIcon FileCollaborationRequest::messageIcon() const {
  switch ( m_state ) {
    case Unknown:
    if ( info().isIncoming() )
      return IconCache::getIconStatic( "filecollaborationrequest_in" );
    else
      return IconCache::getIconStatic( "filecollaborationrequest_out" );
    case Accepted:
    if ( info().isIncoming() )
      return IconCache::getIconStatic( "filecollaborationrequest_in_accepted" );
    else
      return IconCache::getIconStatic( "filecollaborationrequest_out_accepted" );
    case Denied:
    if ( info().isIncoming() )
      return IconCache::getIconStatic( "filecollaborationrequest_in_denied" );
    else
      return IconCache::getIconStatic( "filecollaborationrequest_out_denied" );
    case Failed:
    if ( info().isIncoming() )
      return IconCache::getIconStatic( "filecollaborationrequest_in_failed" );
    else
      return IconCache::getIconStatic( "filecollaborationrequest_out_failed" );
  }
  return IconCache::getIconStatic( "unknown" );
}

FileCollaborationRequestData::State FileCollaborationRequestData::state() const {
  return m_state;
}

void FileCollaborationRequestData::updateState() {
  emit stateChanged( ( FileCollaborationRequest* ) this );
}

bool FileCollaborationRequest::needReply() const {
  if ( m_collab )
    return true;
  else
    return false;
}


MessageInterface::ReplyResult FileCollaborationRequest::gotReply( const MessagePointer& p ) {
  if ( !m_collab )
    return ReplyResult();
  bool handled = false;
  MessagePointer msg = p;
  if ( m_state == Unknown ) {
    if ( msg.cast<KDevSystemMessage>() ) {
      LockedSharedPtr<KDevSystemMessage> lmsg = msg.cast<KDevSystemMessage>();
      if ( lmsg ) {
        if ( lmsg->message() == KDevSystemMessage::ActionSuccessful ) {
          m_state = Accepted;
          emit connectionResult( true );
        } else if ( lmsg->message() == KDevSystemMessage::ActionDenied ) {
          m_state = Denied;
          emit connectionResult( false );
        } else {
          m_state = Failed;
          emit connectionResult( false );
        }
      } else {
        m_state = Failed;
        emit connectionResult( false );
      }
      handled = true;
    }

    if ( m_state == Unknown && msg.cast<FileCollaborationRequest>() ) {
      LockedSharedPtr<FileCollaborationRequest> lmsg = msg.cast<FileCollaborationRequest>();
      if ( lmsg ) {
        globalMessageSendHelper().sendReply<KDevSystemMessage>( lmsg, KDevSystemMessage::ActionSuccessful );
        m_state = Accepted;
        emit connectionResult( true );
      } else {
        m_state = Failed;
        emit connectionResult( false );
      }
      handled = true;
    }

    updateState();
  }

  emit processReply( p );

  if ( m_state != Denied && m_state != Failed && m_collab && m_session )
    return ReplyResult( true, true );
  else
    return ReplyResult( handled );
}

void FileCollaborationRequestData::dispatchSignal( const AcceptSignal& /*sig*/ ) {
  acceptCollaboration();
}

void FileCollaborationRequestData::dispatchSignal( const DenySignal& /*sig*/ ) {
  denyCollaboration();
}


void FileCollaborationRequestData::acceptCollaboration() {
  cout << "acceptCollaboration" << endl;

  MessagePointer::Locked l = ( FileCollaborationRequest* ) this;
  if ( l && m_teamwork ) {
    m_state = Accepted;
    if ( !m_teamwork->collaborationManager() ->fileCollaborationManager().acceptCollaboration( ( FileCollaborationRequest* ) this ) )
      m_state = Failed;
  } else {
    m_state = Failed;
  }

  updateState();
}

void FileCollaborationRequestData::denyCollaboration() {
  cout << "denyCollaboration" << endl;
  MessagePointer::Locked l = ( FileCollaborationRequest* ) this;
  if ( l && m_teamwork ) {
    globalMessageSendHelper().sendReply<KDevSystemMessage> ( l, KDevSystemMessage::ActionDenied );
    m_state = Denied;
    m_teamwork->collaborationManager() ->fileCollaborationManager().denyCollaboration( ( FileCollaborationRequest* ) this );
  } else {
    m_state = Failed;
  }

  updateState();
}

void FileCollaborationRequest::fillContextMenu( QMenu* menu, KDevTeamwork* teamwork ) {
  m_teamwork = teamwork;
  if ( m_state == Unknown && info().isIncoming() ) {

    /*QAction* a = menu->addAction( "Accept File-Collaboration", this, SLOT(acceptCollaboration()) );
    QAction* d = menu->addAction( "Deny File-Collaboration", this, SLOT( denyCollaboration() ) );*/
    QAction* a = menu->addAction( "Accept File-Collaboration", new QSafeSignaller( FileCollaborationRequestPointer(this), FileCollaborationRequestData::AcceptSignal(), menu ), SLOT( signal() ) );
    QAction* d = menu->addAction( "Deny File-Collaboration", new QSafeSignaller( FileCollaborationRequestPointer(this), FileCollaborationRequestData::DenySignal(), menu ), SLOT( signal() ) );

/*
    connect( a, SIGNAL( triggered( bool ) ), new SafeSignaller( FileCollaborationRequestPointer(this), FileCollaborationRequestData::AcceptSignal(), menu ), SLOT( signal() ) );
    connect( d, SIGNAL( triggered( bool ) ), new SafeSignaller( FileCollaborationRequestPointer(this), FileCollaborationRequestData::DenySignal(), menu ), SLOT( signal() ) );*/

    connect( ( QObject* ) this, SIGNAL( stateChanged( const MessagePointer& ) ), ( QObject* ) teamwork->messageManager(), SLOT( updateMessage( const MessagePointer& ) ), Qt::QueuedConnection );

    QVariant v;
    v.setValue<MessagePointer>( this );
    a->setData( v );
    d->setData( v ); ///This makes sure that the message is not deleted in the wrong moment, and the slots can be called safely by the actions.
  }
}

EASY_IMPLEMENT_MESSAGE( FileCollaborationMessage )
EASY_IMPLEMENT_MESSAGE( DocumentWrapperMessage )
EASY_IMPLEMENT_MESSAGE( FileCollaborationRequest )
EASY_IMPLEMENT_MESSAGE( FileEditMessage )
EASY_IMPLEMENT_MESSAGE( FileListMessage )
EASY_IMPLEMENT_MESSAGE( FileEditRejectMessage )

#include "filecollaborationmessages.moc"

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on




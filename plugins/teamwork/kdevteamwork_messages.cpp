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

#include "kdevteamwork_messages.h"
#include  "kdevteamwork.h"
#include  <QDateTime>
#include "kdevteamwork_messageshower.h"
#include "kdevteamwork_helpers.h"
#include "indocumentreference.h"
#include "teamworkserver.h"
#include "teamworkclient.h"

REGISTER_MESSAGE( KDevSystemMessage );
REGISTER_MESSAGE( KDevTeamworkTextMessage );
REGISTER_MESSAGE( ConnectionRequest );
REGISTER_MESSAGE( CollaborationMessage );


KDevSystemMessage::KDevSystemMessage( InArchive& arch, const Teamwork::MessageInfo& info ) : SystemMessage( arch, info ) {}
KDevSystemMessage::KDevSystemMessage( const Teamwork::MessageInfo& info, Message msg , const string& str ) : SystemMessage( info, ( Teamwork::SystemMessage::Message ) msg, str ) {}

KDevSystemMessage::Message KDevSystemMessage::message() {
  return ( Message ) SystemMessage::message();
}

QString KDevSystemMessage::messageAsString() {
  switch ( message() ) {
    case CollaborationAccepted:
    return "collaboration accepted";
    case CollaborationRefused:
    return "collaboration refused";
    case CollaborationClosed:
    return "collaboration stopped";
    case ActionFailed:
    return "action failed";
    case ActionSuccessful:
      return "action successful";
    case ActionDenied:
      return "action denied";
  }
  return "unknown kdev-system-message";
}

QString KDevSystemMessage::text() {
  return SystemMessage::text().c_str();
}

typedef SafeSharedPtr<KDevSystemMessage> KDevSystemMessagePointer;

KDevTeamworkTextMessage::KDevTeamworkTextMessage( const Teamwork::MessageInfo& info, const QString& text ) : TextMessage( info, text.toUtf8().data() ), creationTime_( QDateTime::currentDateTime() ), m_answered( false ) {}

KDevTeamworkTextMessage::KDevTeamworkTextMessage( InArchive& from, const Teamwork::MessageInfo& info ) : TextMessage( from, info ), m_answered( false ) {
  std::string date;
  from & date;
  creationTime_ = QDateTime::fromString( ~date, Qt::ISODate );
}

void KDevTeamworkTextMessage::setAnswered( bool a ) {
  m_answered = a;
}

bool KDevTeamworkTextMessage::answered() const {
  return m_answered;
}

QString KDevTeamworkTextMessage::messageText() const {
  return ~Teamwork::TextMessage::text();
}

class DocumentMessageInternal;
class InDocumentConversation;

bool KDevTeamworkTextMessage::needReply() const {
  return true;
}

ConnectionRequest::ConnectionRequest( InArchive& arch, const Teamwork::MessageInfo& info ) : KDevTeamworkTextMessage( arch, info ), m_state( Waiting ) {
  arch & user_;
  //user_.load( arch, 0 );
}

void ConnectionRequest::serialize( OutArchive& arch ) {
  Precursor::serialize( arch );
  //user_.save( arch, version );
  arch & user_;
}

const Teamwork::User& ConnectionRequest::user() {
  return user_;
}

bool ConnectionRequest::needReply() const {
  return true;
}

void ConnectionRequest::setState( State s ) {
  m_state = s;
}

ConnectionRequest::State ConnectionRequest::state() {
  return m_state;
}

bool KDevTeamworkTextMessage::canShowInWidget() {
  return true;
}


void KDevTeamworkTextMessage::showInWidget( QWidget* widget, KDevTeamwork* tw ) {
  new KDevTeamworkMessageShower( this, widget, tw );
}

QDateTime KDevTeamworkTextMessage::creationTime() {
  return creationTime_;
}

void KDevTeamworkTextMessage::serialize( OutArchive& arch ) {
  TextMessage::serialize( arch );
  std::string date = ~creationTime_.toString( Qt::ISODate );
  arch & date;
}

QIcon KDevTeamworkTextMessage::messageIcon() const {
  IconCache & cache( *IconCache::instance() );

  if ( info().isIncoming() )
    return cache( "text_message_in" );
  else
    return cache( "text_message_out" );
}

ConnectionRequest::ConnectionRequest( const Teamwork::MessageInfo& info, const Teamwork::UserPointer& self, const Teamwork::UserPointer& target, const QString& text, KDevTeamwork* teamwork ) : KDevTeamworkTextMessage( info, text ), m_state( Waiting ), m_emitter( new SafeTeamworkEmitter( teamwork ) ) {
  Teamwork::UserPointer::Locked l = self;
  Teamwork::UserPointer::Locked tl = target;
  if ( l ) {
    user_ = *l;
    user_.stripForTarget( *tl );
  } else {
    user_ = Teamwork::User( "unlockable" );
  }
}

MessageInterface::ReplyResult ConnectionRequest::gotReply( const DispatchableMessage& p ) {
  KDevSystemMessagePointer::Locked lmessage = ( ( MessagePointer ) p ).cast<KDevSystemMessage>();

  if ( lmessage ) {
    switch ( lmessage->message() ) {
      case KDevSystemMessage::CollaborationRefused:
      m_state = Denied;
      break;
      case KDevSystemMessage::CollaborationAccepted:
      m_state = Accepted;
      break;
      case KDevSystemMessage::ActionFailed:
      m_state = Unknown;
      break;
      default:
      m_state = Unknown;
      break;
    }
  } else {
    return ReplyResult( false, true ); ///Messages other than KDevSystemMessage are not accepted
  }

  if ( m_emitter.data() )
    m_emitter->updateMessageInfo( this );
  return ReplyResult();
}

QIcon ConnectionRequest::messageIcon() const {
  IconCache & cache( *IconCache::instance() );

  if ( !info().isIncoming() ) {
    if ( m_state == Denied ) {
      return cache( "collaborationrequest_out_denied" );
    } else if ( m_state == Accepted ) {
      return cache( "collaborationrequest_out_accepted" );
    } else if ( m_state == Unknown ) {
      return cache( "unknown" );
    } else {
      return cache( "collaborationrequest_out" );
    }
  } else {
    if ( m_state == Denied ) {
      return cache( "collaborationrequest_in_denied" );
    } else if ( m_state == Accepted ) {
      return cache( "collaborationrequest_in_accepted" );
    } else if ( m_state == Unknown ) {
      return cache( "unknown" );
    } else {
      return cache( "collaborationrequest_in" );
    }
  }

  if ( info().isIncoming() )
    return cache( "collaborationrequest_in" );
  else
    return cache( "collaborationrequest_out" );
}

QIcon AbstractGUIMessage::messageTypeIcon() const {
  IconCache & cache( *IconCache::instance() );
  return cache( "message" );
}

QIcon AbstractGUIMessage::messageIcon() const {
  IconCache & cache( *IconCache::instance() );
  return cache( "message" );
}

FailureMessage::FailureMessage( const QString& text, const MessagePointer& msg ) : KDevTeamworkTextMessage( Teamwork::MessageInfo( MessageId() ), text ) {
  info().setReplyMessage( msg );
  MessagePointer::Locked l = msg;
  if( l ) {
    info().setReply( l->info().id().uniqueId() );
    info().setUser( l->info().user() );
  }
}

QIcon FailureMessage::messageIcon() const {
  return IconCache::getIconStatic( "failed" );
}


// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on

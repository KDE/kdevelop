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

#include "common.h"
#include "messageimpl.h"
#include "basicsession.h"
#include "teamworkmessages.h"
#include "teamworkservermessages.h"
#include "messagesendhelper.h"



namespace Teamwork {
MessageTypeSet globalTypeSet;
MessageSendHelper globalSendHelper;

void registerDefaultMessages( MessageTypeSet& target ) {
  target.registerMessageType<RawMessage, MessageInterface>( );
  target.registerMessageType<TextMessage, RawMessage>( );
}


RawMessage::RawMessage( const MessageInfo& info, const DataVector& data ) : info_( info ), body_( data ) {}
;

///This should be used to indicate that this message is a reply to the other message(replyTarget)
void RawMessage::setReply( MessageInterface* replyTarget ) {
  info_.setReply( replyTarget->id().uniqueId() );
}

RawMessage::RawMessage( InArchive& from, const MessageInfo& info ) : info_( info ) {
  from & body_;
}

void RawMessage::serialize( OutArchive& target ) {
  target & body_;
}

const MessageInfo& RawMessage::info() const {
  return info_;
};

MessageInfo& RawMessage::info() {
  return info_;
};

/**This is called once a message has been tried to be sent, with the appropriate result, and can be used by the message to give some feedback to the sender. The Session used to send the message is still locked.
  */
void RawMessage::result( bool success ) {
  UNUSED( success );
}

/**Returns a reference to the data-store of this raw message. It does not include data serialized to inherited messages. */
DataVector& RawMessage::body() {
  return body_;
}

/**Returns a reference to the data-store of this raw message. It does not include data serialized to inherited messages. */
const DataVector& RawMessage::body() const {
  return body_;
}


TextMessage::TextMessage( const MessageInfo& info, const std::string& text ) : RawMessage( info, DataVector() ) {
  if ( text.length() == 0 )
    return ;
  body().resize( text.length() + 1 );

  memcpy( ( char* ) & ( body() [ 0 ] ), text.c_str(), text.length() + 1 );
}

TextMessage::TextMessage( InArchive& from, const MessageInfo& info ) : RawMessage( from, info ) {}

std::string TextMessage::text() const {
  if ( body().size() == 0 )
    return std::string();
  return std::string( &( body() [ 0 ] ) );
}

SystemMessage::SystemMessage( const MessageInfo& info, Message msg, const string& ptext ) : TextMessage( info, ptext ), msg_( msg ) {}

SystemMessage::SystemMessage( InArchive& arch, const MessageInfo& info ) : TextMessage( arch, info ) {
  serial( arch );
}

void SystemMessage::serialize( OutArchive& arch ) {
  Precursor::serialize( arch );
  serial( arch );
}

SystemMessage::Message SystemMessage::message() {
  return msg_;
}

string SystemMessage::messageAsString() {
  switch ( msg_ ) {
    case LoginSuccess:
    return "LoginSuccess";
    case LoginFailedUnknown:
    return "LoginFailedUnknown";
    case BadAuthentication:
    return "BadAuthentication";
    case ServerBusy:
    return "SeverBusy";
    case Kicked:
    return "Kicked";
    case BadTarget:
    return "BadTarget";
    case StoredOnServer:
    return "StoredOnServer";
    case AlreadyLoggedIn:
    return "AlreadyLoggedIn";
    case GetUserList:
    return "GetUserList";
    case NoMessage:
    return "NoMessage";
    case SerializationFailed:
    return "SerializationFailed";
  };
  return "";
}


TeamworkMessage::TeamworkMessage( const MessageInfo& info ) : RawMessage( info, DataVector() ) {}
TeamworkMessage::TeamworkMessage( InArchive& arch, const MessageInfo& info ) : RawMessage( arch, info ) {}

IdentificationMessage::IdentificationMessage( const MessageInfo& info, const User& user ) : TeamworkMessage( info ), user_( user ) {}

IdentificationMessage::IdentificationMessage( InArchive& arch, const MessageInfo& info ) : TeamworkMessage( arch, info ) {
  serial( arch );
}

IdentificationMessage::operator User() {
  return user_;
}

void IdentificationMessage::serialize( OutArchive& arch ) {
  Precursor::serialize( arch );
  serial( arch );
}


ForwardMessage::ForwardMessage( const MessageInfo& info, MessageInterface* messageToForward, const User& source, const User& targ ) : RawMessage( info, DataVector( ) ), source_( source ), target_( targ ) {
  serializeMessageToBuffer( body(), *messageToForward );
  source_.stripForTarget( target_ );
  target_.stripForIdentification();
}

ForwardMessage::ForwardMessage( InArchive& arch, const MessageInfo& info ) : RawMessage( arch, info ) {
  serial( arch );
}

void ForwardMessage::serialize( OutArchive& arch ) {
  Precursor::serialize( arch ) ;
  serial( arch );
}

User& ForwardMessage::source() {
  return source_;
}

User& ForwardMessage::target() {
  return target_;
}

MessagePointer ForwardMessage::deserializeContained( MessageTypeSet& messageTypes ) {
  try {
    return buildMessageFromBuffer( body(), messageTypes, info().session() );
  } catch ( ... ) {
    return MessagePointer();
  }
};

bool ForwardMessage::storeOnServer() {
  return false;
}

UserListMessage::UserListMessage( const MessageInfo& info, list<UserPointer> inUsers, const UserPointer& targetUser ) : TeamworkMessage( info ) {
  for ( list<UserPointer>::iterator it = inUsers.begin(); it != inUsers.end(); ++it ) {
    if ( *it == targetUser )
      continue;

    UserPointer::Locked l = *it;
    if ( l ) {
      if( !l->online() ) continue;
      users.push_back( *l );
      users.back().stripForPublic();
    } else {
      cout << "could not lock a user-pointer";
    }
  }
}

UserListMessage::UserListMessage( InArchive& arch, const MessageInfo& info ) : TeamworkMessage( arch, info ) {
  serial( arch );
}

void UserListMessage::serialize( OutArchive& arch ) {
  Precursor::serialize( arch );
  serial( arch );
}


REGISTER_MESSAGE( RawMessage );
REGISTER_MESSAGE( TextMessage );
REGISTER_MESSAGE( TeamworkMessage );
REGISTER_MESSAGE( IdentificationMessage );
REGISTER_MESSAGE( ForwardMessage );
REGISTER_MESSAGE( SystemMessage );
REGISTER_MESSAGE( UserListMessage );
}

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on

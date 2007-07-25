/***************************************************************************
  Copyright 2006 David Nolden <david.nolden.kdevelop@art-master.de>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "forwardsession.h"
#include "multisession.h"
#include "helpers.h"

namespace Teamwork {

LoggerPrinter ForwardSession::err( int prio ) {
  LoggerPrinter p ( logger_, Logger::Error );
  if ( prio )
    p << prio << ":";
  p << "error in session " << "(" << sessionName() << "): ";
  return p;
}

LoggerPrinter ForwardSession::out( Logger::Level lv  ) {
  LoggerPrinter p ( logger_, lv );
  p << "in session " << "(" << sessionName() << "): ";
  return p;
}

ForwardSession::ForwardSession( const UserPointer& sourceUser, const UserPointer& targetUser, const MultiSessionPointer& session, const LoggerPointer& logger, MessageTypeSet& messageTypes, HandlerPointer handler ) : SessionReplyManager( session.unsafe() ), session_( session ), targetUser_( targetUser ), sourceUser_( sourceUser ), logger_( logger ), notRunning_( false ), messageTypes_( messageTypes ), handler_( handler ) {
  MultiSessionPointer::Locked l = session_;
  if ( l ) {
    l->registerForwardSession( targetUser_, this );
  } else {
    ///error
  }
  setUser( targetUser );
  UserPointer::Locked lu = targetUser;
  if ( lu ) {
    setSessionName( "forward_" + lu->name() );
    SessionPointer::Locked ls = lu->online().session();
    if ( ls && ls->isRunning() ) {
      out( Logger::Warning ) << "creating an indirect session to " << lu->name() << ", but there already is a direct session";
    } else {
      lu->setSession( this );
    }

  } else {
    err() << "could not lock target-user of a forward-session";
  }
}

ForwardSession::~ForwardSession() {
}

bool ForwardSession::isOk() {
  MultiSessionPointer p = session_/*.get()*/;
  if ( !notRunning_ && p )
    return p.unsafe() ->isOk();
  return false;
}
bool ForwardSession::isRunning() {
  MultiSessionPointer p = session_/*.get()*/;
  if ( !notRunning_ && p )
    return p.unsafe() ->isRunning();
  return false;
}


bool ForwardSession::send( MessageInterface* msg ) {
  if ( !isRunning() )
    return false;
  MultiSessionPointer p = session_/*.get()*/;
  if( !p ) return false;

  msg->info().setSession( this );

  UserPointer::Locked source = sourceUser_;
  UserPointer::Locked target = targetUser_;

  if ( msg && source && target ) {
    if ( msg->needReply() )
      addWaitingMessage( msg );
    return p.unsafe() ->send( new ForwardMessage( messageTypes_, msg, *source, *target ) );
  } else {
    err() << "failed to do the necessary locks for sending a message(message and users)";
    return false;
  }
}

void ForwardSession::stopRunning() {
  out( Logger::Debug ) << "forwardsession: stopRunning";
  notRunning_ = true;
  session_ = 0;
}

void ForwardSession::stopRunningNow() {
  out( Logger::Debug ) << "forwardsession: stopRunningNow";
  notRunning_ = true;
  session_ = 0;
}

bool ForwardSession::think() {
  return false;
}

void ForwardSession::startSession() {}

UserPointer ForwardSession::target() {
  return targetUser_;
}

SessionInterface::SessionDirection ForwardSession::sessionDirection() {
  MultiSessionPointer::Locked l = session_;
  if ( !l )
    return Incoming;
  return l->sessionDirection();
}

///Must only be called from within session_!
bool ForwardSession::handleForwardMessage( const SafeSharedPtr<ForwardMessage>& msg ) {
  out( Logger::Debug ) << "handling forward-message";
  SafeSharedPtr<ForwardMessage>::Locked l = msg;
  UserPointer::Locked lu = sourceUser_;

  if ( l && lu ) {
    if ( lu->match( l->target() ) ) {
      MessagePointer message = l->deserializeContained( messageTypes_ );
      MessagePointer::Locked lmessage = message;
      if ( lmessage ) {
        lmessage->info().setSession( this );
      } else
        err() << "ForwardSession: could not lock a deserialized message";

      handleMessage( message );
      return true;
    }
  } else {
    out() << "could not lock forward-message or local user";
  }

  return false;
}

///Must only be called from within session_!
bool ForwardSession::handleMessage( MessagePointer msg ) throw() {
  if ( handleMessageWaiting( msg ) )
    return true;
  if ( handler_ )
    return handler_.unsafe() ->handleMessage( msg );
  return false;
}

SessionInterface::SessionType ForwardSession::sessionType() {
  return Forwarded;
}

void ForwardSession::final() {
  SessionInterface::final();
}


LoggerPrinter FakeSession::err( int prio ) {
  LoggerPrinter p ( logger_, Logger::Error );
  if ( prio )
    p << prio << ":";
  p << "error in session " << "(" << sessionName() << "): ";
  return p;
}

LoggerPrinter FakeSession::out( Logger::Level lv ) {
  LoggerPrinter p ( logger_, lv );
  p << "in session " << "(" << sessionName() << "): ";
  return p;
}

FakeSession::FakeSession( const UserPointer& targetUser, const LoggerPointer& logger, MessageTypeSet& messageTypes, HandlerPointer handler ) : logger_( logger ), messageTypes_( messageTypes ), handler_( handler ) {
  setUser( targetUser );
  UserPointer::Locked lu = targetUser;
  if ( lu ) {
    setSessionName( "session_fake_" + lu->name() );
  } else {
    setSessionName( "session_fake" );
    err() << "FakeSession: could not lock target-user of a fake-session";
  }
}

bool FakeSession::isOk() {
  return true;
}

bool FakeSession::isRunning() {
  return true;
}

bool FakeSession::send( MessageInterface* msg ) {
  SafeSharedPtr<FakeSession>::Locked l( this );
  if ( l ) {
    msg->info().setSession( this );

  ///Simulate sending the message by serializing and deserializing it within a local buffer
    vector<char> vec;
    serializeMessageToBuffer( vec, *msg );
    MessagePointer nmsg = buildMessageFromBuffer( vec, globalMessageTypeSet(), 0 );
    if ( !nmsg ) {
      err() << "FakeSession: could not build message from buffer";
      return false;
    }
    MessagePointer::Locked l = nmsg;
    if ( !l ) {
      err() << "FakeSession: could not lock created message";
      return false;
    }
    l->info().setReplyMessage( msg->info().replyToMessage() );

    receivedMessages_.push_back( nmsg );
    return true;
  } else {
    err() << "FakeSession: lock-error";
  }
  return true;
}

void FakeSession::stopRunning() {
  out( Logger::Debug ) << "FakeSession: stopRunning";
}

void FakeSession::stopRunningNow() {
  out( Logger::Debug ) << "FakeSession: stopRunningNow";
}

bool FakeSession::think() {
  return false;
}

void FakeSession::startSession() {}

SessionInterface::SessionDirection FakeSession::sessionDirection() {
  return Outgoing;
}

///Must only be called from within session_!
bool FakeSession::handleMessage( MessagePointer msg ) throw() {
  if ( handler_ )
    return handler_.unsafe() ->handleMessage( msg );
  return false;
}

SessionInterface::SessionType FakeSession::sessionType() {
  return Direct;
}

std::list<MessagePointer>& FakeSession::sentMessages() {
  return receivedMessages_;
}


/**This takes the first message off the stack of sent messages, serializes and deserializes it, and then returns the deserialized message.
   All exceptions from the serialization- and deserialization-process are passed by, additionally errors are indicated by throwing a const char*.
*/
MessagePointer::Locked FakeSession::getFirstMessage() throw( const char* ) {
  if ( receivedMessages_.empty() )
    throw "FakeSession: no messages available";
  return receivedMessages_.front();
}
}

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on

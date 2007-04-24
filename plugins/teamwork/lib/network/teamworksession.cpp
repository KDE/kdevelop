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

#include "teamworkserver.h"
#include "teamworksession.h"
#include "forwardsession.h"

namespace Teamwork {

TeamworkSession::TeamworkSession( ost::TCPSocket &server, HandlerPointer handler, MessageTypeSet& messages, const LoggerPointer& logger ) : BasicTCPSession( server, handler, messages, logger, "incoming_" ), incoming_( true ) {}

TeamworkSession::TeamworkSession( const ServerInformation& server, HandlerPointer handler, MessageTypeSet& messages, const LoggerPointer& logger, const string& namePrefix ) : BasicTCPSession( server.addr().c_str() , server.port(), handler, messages, logger, namePrefix ), incoming_( false ) {}

void TeamworkSession::deleteForwardSessions() {
  for ( ForwardSessionMap::iterator it = forwardSessions_.begin(); it != forwardSessions_.end(); ++it ) {
    ForwardSessionPointer::Locked l = ( *it ).second;
    if ( l ) {
      l->stopRunningNow();
      l->final();
    } else
      err() << "could not lock a Forward-session for final";
  }
  for ( list<ForwardSessionPointer>::iterator it = garbageSessions_.begin(); it != garbageSessions_.end(); ++it ) {
    ForwardSessionPointer::Locked l = ( *it );
    if ( l ) {
      l->stopRunningNow();
      l->final();
    } else
      err() << "could not lock a garbage Forward-session for final";
  }
  forwardSessions_.clear();
  garbageSessions_.clear();
}

void TeamworkSession::final() {
  deleteForwardSessions();
  BasicTCPSession::final();
}

bool TeamworkSession::think() {
  bool ret = false;

  for ( ForwardSessionMap::iterator it = forwardSessions_.begin(); it != forwardSessions_.end(); ) {
    ForwardSessionMap::iterator it2 = it;
    ++it;
    ///This costs very much and should be optimized once it works perfectly
    ForwardSessionPointer::Locked l = ( *it2 ).second;
    if ( l ) {
      if ( !( *it2 ).second.getUnsafeData() ->isRunning() ) {
        l->final();
        forwardSessions_.erase( it2 );
      } else {
        l->think();
      }
    } else
      out() << "could not lock a forward-session for think/final";
  }

  for ( list<ForwardSessionPointer>::iterator it = garbageSessions_.begin(); it != garbageSessions_.end(); ) {
    list<ForwardSessionPointer>::iterator it2 = it;
    ++it;
    if ( !( *it2 ).getUnsafeData() ->isRunning() ) {
      ForwardSessionPointer::Locked l = ( *it2 );
      if ( l ) {
        l->final();
        garbageSessions_.erase( it2 );
      } else
        out() << "could not lock a garbage Forward-session for final";
    }
  }

  return BasicTCPSession::think() | ret;
}

template <class Type>
    class FastLockHelper {}
;

TeamworkSession::~TeamworkSession() {
  deleteForwardSessions();
}

void TeamworkSession::registerForwardSession( const UserPointer& peer, const ForwardSessionPointer& sess ) {
  if ( forwardSessions_.find( peer ) != forwardSessions_.end() ) { ///finalize some older open session to the same target
    UserPointer::Locked u = peer;
    out() << "closing a duplicated indirect session to " << ( u ? u->name() : "not lockable" );
    forwardSessions_[ peer ].getUnsafeData() ->stopRunning();
    garbageSessions_.push_back( forwardSessions_[ peer ] );
  }

  forwardSessions_[ peer ] = sess;
}

bool TeamworkSession::handleMessage( DispatchableMessage msg ) throw() {
  MessagePointer mp = msg;
  SafeSharedPtr<ForwardMessage> forward = mp.cast<ForwardMessage>();
  if ( forward ) {
    SafeSharedPtr<ForwardMessage>::Locked l = forward;
    UserPointer u( new User( l->source() ) );
    ForwardSessionMap::iterator it = forwardSessions_.find( u );
    if ( it != forwardSessions_.end() ) {
      ForwardSessionPointer::Locked l = ( *it ).second;
      if ( l ) {
        return l->handleForwardMessage( forward );
      } else {
        err() << "could not lock a forward-session to process an incoming forward-message";
      }
    }
  }

  return BasicTCPSession::handleMessage( msg );
}

}
// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on

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

#include "teamworkclient.h"
#include "forwardsession.h"
#include "user.h"
#include "teamworkmessages.h"
#include "teamworkservermessages.h"
#include "messagesendhelper.h"
#include "forwardsession.h"
#include "multisession.h"


namespace Teamwork {

BIND_LIST_2( TeamworkClientDispatchMessages , ForwardMessage, UserListMessage );

///Takes a list of SafeSharedPtr's, and returns a list of the same LockedSharedPtr's
template <class Type, class Serialization>
list< typename SafeSharedPtr<Type, Serialization>::Locked > lockList( const list< SafeSharedPtr<Type, Serialization> >& lst ) {
  list< typename SafeSharedPtr<Type, Serialization>::Locked > ret;
  for ( typename list< SafeSharedPtr<Type, Serialization> >::const_iterator it = lst.begin(); it != lst.end(); ++it ) {
    ret.push_back( *it );
  }

  return ret;
}

void Client::userDisconnected( const Teamwork::UserPointer& /*user*/ ) {
  needUserUpdate_ = true; ///The connected client may have been a client that created a direct connection to us, and which may still be reachable through an indirect connection, so an update is needed
}


///this is called whenever a connection is successfully established to a server
void Client::connectedToServer( const ClientSessionDesc& /*session*/, const ServerInformation& /*server*/ ) {}

///this is called whenever the client disconnects from a connected server in any way
void Client::disconnectedFromServer( const ClientSessionDesc& /*session*/, const ServerInformation& /*server*/ ) {
  needUserUpdate_ = true;  ///The connected server may have been a client we created a direct connection to, and which is still reachable through an indirect connection, so an update is needed
}


bool Client::think() {
  for ( ClientSessionMap::iterator it = clientSessions_.begin(); it != clientSessions_.end(); ) {
    if ( !( *it ).second.session.unsafe() ->isRunning() ) {
      out() << "closing outgoing session because it stopped running";
      disconnectedFromServer( ( *it ).second, ( *it ).first );
      ClientSessionMap::iterator itb = it;
      ++it;
      clientSessions_.erase( itb );
    } else {
      if ( needUserUpdate_ ) {
        send<SystemMessage>( ( *it ).second.session.unsafe(), SystemMessage::GetUserList );
      }
      ++it;
    }
  }

  needUserUpdate_ = false;

  return Server::think();
}

void Client::connectToServer( const ServerInformation& server, const UserPointer& asUser ) {
  if ( isConnectedToServer( server ) )
    disconnectFromServer( server );

  UserPointer p = asUser;
  if ( !p )
    p = identity();
  if ( !p ) {
    err() << "tried to connect to a server without having an identity";
    return ;
  }

  SessionInterface* session = new MultiSession( server, new HandlerProxy<BasicServer>( this ), messageTypes(), logger() , server.desc() + "_outgoing_" );
  clientSessions_[ server ] = ClientSessionDesc( p, session );
  session->startSession();

  UserPointer::Locked l = p;
  if ( l )
    send<IdentificationMessage>( clientSessions_[ server ].session.unsafe(), *l );
  else
    err() << "could not lock user";

  connectedToServer( clientSessions_[ server ], server );
}

bool Client::isConnectedToServer( const ServerInformation& server ) {
  return clientSessions_.find( server ) != clientSessions_.end();
}

MultiSessionPointer Client::sessionToServer( const ServerInformation& server ) {
  ClientSessionMap::iterator it = clientSessions_.find( server );
  if ( it == clientSessions_.end() )
    return 0;
  else
    return ( *it ).second.session.cast<MultiSession>();
}

void Client::disconnectFromServer( const ServerInformation& server ) {
  ClientSessionMap::iterator it = clientSessions_.find( server );
  if ( it != clientSessions_.end() ) {
    disconnectedFromServer( ( *it ).second, ( *it ).first );
    ( *it ).second.session.unsafe() ->stopRunning();
    clientSessions_.erase( it );
  }
};

void Client::disconnectAllServers () {
  int c = 0;
  while ( !clientSessions_.empty() && ++c < 50000 ) {
    disconnectFromServer( ( *clientSessions_.begin() ).first );
  }
}

int Client::receiveMessage( UserListMessage* msg ) {
  out( Logger::Debug ) << "handling user-list of size " << msg->users.size();
  std::list<UserPointer> users;
  {
    std::set
      <UserPointer::Locked, UserPointer::Locked::ValueSmallerCompare> allReceivedUsers;

    MultiSessionPointer session = msg->info().session().cast<MultiSession>();

    if ( !session ) {
      out() << "got a user-list message on an indirect session, ignoring";
      return 0;
    }

    MultiSessionPointer::Locked lsession = session;
    if ( !lsession ) {
      err() << "could not lock session while receiving user-list";
      return 0;
    }

    UserPointer::Locked sUser = lsession->safeUser();

    if ( !sUser )
      out( Logger::Debug ) << "sender of the user-list has no user-identity";

    ///compare the user-list received from the server and the slave-sessions registered to the session.
    list<UserPointer::Locked> sessionUsers = lockList( lsession->getUserList() );

    for ( list<User>::iterator it = msg->users.begin(); it != msg->users.end(); ++it ) {
      UserPointer::Locked l = getUser( createUser( &( *it ) ) );
      if ( !l ) {
        err() << "could not get/create indirect user " << ( *it ).name();
        continue;
      }
      if ( l->online() )
        continue;   ///The user is already online, maybe through a direct connection

      ForwardSessionPointer s = lsession->getForwardSession( l );

      allReceivedUsers.insert( l );

      if ( s ) {
        ///There already is an open session to the user
      } else {
        ///Create a new session to the user
        users.push_back( l );

        l->setSession( new ForwardSession( identity(), l, msg->info().session().cast<MultiSession>(), logger(), messageTypes(), new HandlerProxy<BasicServer>( this ) ) );
        registerSessionUser( l ); ///Register the user together with its session, so it's noticed once the session is down.
      }
    }

    ///now invalidate all users that the session has a forward-session to but that were not listed by the server, because those probably are offline.

    for ( list<UserPointer::Locked>::iterator it = sessionUsers.begin(); it != sessionUsers.end(); ++it ) {
      std::set
        <UserPointer::Locked, UserPointer::Locked::ValueSmallerCompare>::iterator fnd = allReceivedUsers.find( *it );
      if ( fnd == allReceivedUsers.end() ) {
        ForwardSessionPointer p = lsession->getForwardSession( ( *it ).data() );
        if ( p ) {
          p.unsafe() ->stopRunning();
          out( Logger::Debug ) << "closing secondary session because the user went offline";
        }
      }
    }

  }

  //if( !users.empty() )
  gotUserList( users );

  return 1;
}

int Client::receiveMessage( MessageInterface* /*msg*/ ) {
  return 0;
}

int Client::receiveMessage( ForwardMessage * msg ) {
  out( Logger::Debug ) << "Client dispatching ForwardMessage";
  if ( !identity() )
    return 0;
  UserPointer::Locked l = identity();
  if ( !l ) {
    err() << "could not identity while processing a forwarded message";
    return 0;
  }

  if ( !l->matchIdentity( msg->target() ) )
    return 0;   ///the server should forward the message to the correct target

  out() << "an indirect message from a not yet known client arrived";

  if ( !msg->info().session().cast<MultiSession>() ) {
    out() << "got a forward-message on an indirect session, ignoring";
    return 0;
  }

  MessagePointer::Locked cl = msg->deserializeContained( messageTypes() );
  if ( cl ) {
    UserPointer::Locked user = getUser( createUser( &msg->source() ) );
    if ( !user ) {
      err() << "could not get/lock the user for a forward-message";
      return 1;
    }

    ForwardSession* f = new ForwardSession( identity(), user, msg->info().session().cast<MultiSession>(), logger(), messageTypes(), new HandlerProxy<BasicServer>( this ) );
    f->setUser( f->target() );
    cl->info().setSession( f );

    if ( user->online() ) {
      out( Logger::Warning ) << "got indirect message from directly connected user " << user->name() << ", keeping communication indirect";
    } else {
      user->setSession( f );
    }

    processMessage( cl.data() );
  } else {
    out() << "could not handle a forwarded message";
  }
  return 1;
}

void Client::processMessage( MessageInterface* msg ) {
  MessageDispatcher< Client, TeamworkClientDispatchMessages > dispatcher(*this);
  if ( dispatcher( msg ) ) {
    out( Logger::Debug ) << "handled message in the client-class";
  } else {
    out( Logger::Debug ) << "handing message from client-class to the server-class";
    Server::processMessage( msg );
  }
}

///Is called whenever a connected server sends its list of connected users
void Client::gotUserList( const std::list<UserPointer>& /*users*/ ) {
}

///this could be used to create a custom session deriven from MultiSession
SessionPointer Client::createSession( BasicTCPSocket* sock ) {
  return Server::createSession( sock );
}

bool Client::registerSession( SessionPointer session ) {
  if ( ! Server::registerSession( session ) )
    return false;

  if ( session.unsafe() ->sessionDirection() != SessionInterface::Incoming )
    return true; ///Only send an identification-message to incoming connections, outgoing ones are sent directly while connecting

  if ( identity() ) {
    UserPointer::Locked l = identity();
    if ( l )
      send<IdentificationMessage>( session.unsafe(), *l );
    else
      err() << "could not lock user to send identifaction to a new client";
  } else {
    out() << "this server has no identity";
  }

  return true;
};

Client::Client( ServerInformation serverInfo, LoggerPointer logger ) : Server( serverInfo, logger ), needUserUpdate_( false ) {
  //    messageTypes().registerMessageTypes<AllTeamworkClientMessages> ();
  //dispatcher_.print( cout );
  allowIncoming( false );
}

Client::~Client() {
  disconnectAllServers();
}
}


// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on

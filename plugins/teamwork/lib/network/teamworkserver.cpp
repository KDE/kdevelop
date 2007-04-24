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
#include "messageimpl.h"
#include "teamworkmessages.h"
#include "teamworkservermessages.h"
#include "forwardsession.h"
#include "teamworksession.h"


namespace Teamwork {
using namespace std;

template <class From, class To, class Compare>
list<To> mapValues( map<From, To, Compare> mp ) {
  list<To> ret;
  for ( typename map<From, To, Compare>::iterator it = mp.begin(); it != mp.end(); ++it ) {
    ret.push_back( ( *it ).second );
  }
  return ret;
}

void Server::setIdentity( const UserPointer& user ) {
  ident_ = user;
}

UserPointer Server::identity() const {
  return ident_;
}

Server::Server( const ServerInformation& inf, const LoggerPointer& logger ) : BasicServer( inf.addr().c_str(), inf.port(), globalMessageTypeSet(), logger ), MessageSendHelper( globalMessageTypeSet() ), dispatcher_( *this ), userListDirty_( false ) {
  //dispatcher_.print( cout );
}

void Server::getUserSet( UserSet& users ) {
  users = users_;
}

void Server::insertUserSet( const UserSet& users ) {
  users_.insert( users.begin(), users.end() );
}

Server::~Server() {
  for ( UserSet::iterator it = users_.begin(); it != users_.end(); ++it ) {
    UserPointer::Locked l = *it;
    if ( l ) {
      l->setSession( 0 );
    } else {
      err() << "could not lock a UserPointer while destruction";
    }
  }
}

SessionPointer Server::createSession( BasicTCPSocket* sock ) {
  return new TeamworkSession( *sock, new HandlerProxy<BasicServer>( this ), globalMessageTypeSet(), logger() );
  ///check yet whether this is thread-safe, if not give some kind of safe pointers
}

bool Server::registerSession( SessionPointer session ) {
  TeamworkSessionPointer s = session.cast<TeamworkSession>();
  if ( s ) {
    unknownSessions_.insert( s );
    return true;
  } else {
    return false;
  }
};

bool Server::registerSessionUser( const UserPointer& user ) {
  UserPointer::Locked l = user;
  if( !l ) return false;
  if( !l->online().session() ) return false;
  sessions_[ l->online().session() ] =  user ;
  return true;
}

void Server::sendUserLists() {
  out( Logger::Debug ) << "broadcasting user-list";
  list<UserPointer> users = mapValues( sessions_ );
  for ( SessionMap::iterator it = sessions_.begin(); it != sessions_.end(); ++it ) {
    send<UserListMessage>( ( *it ).first.getUnsafeData(), users, ( *it ).second );
  }
}

void Server::initial( void ) {
  return BasicServer::initial();
}

UserPointer Server::findUser( const UserPointer& user ) {
  UserSet::iterator it = users_.find( user );
  if ( it != users_.end() )
    return * it;
  else
    return 0;
}

UserPointer Server::findUser( const UserIdentity& user ) {
  UserSet::iterator it = users_.find( new User( user ) );
  if ( it != users_.end() )
    return * it;
  else
    return 0;
}

bool Server::think() {
  ///identify dead sessions and free pointers to them so they are deleted by reference-counting
  for ( SessionSet::iterator it = unknownSessions_.begin(); it != unknownSessions_.end(); ) {
    if ( !( *it ).getUnsafe() ->isRunning() ) {
      out() << "deleting unknown incoming session";
      SessionSet::iterator itb = it;
      ++it;
      unknownSessions_.erase( itb );
    } else
      ++it;
  }

  for ( SessionMap::iterator it = sessions_.begin(); it != sessions_.end(); ) {
    if ( !( *it ).first.getUnsafe() ->isRunning() ) {
      SessionMap::iterator itb = it;
      ++it;
      UserPointer::Locked l = ( *itb ).second;
      if ( l ) {
        l->setSession( 0 );
      } else {
        err() << "could not lock user-data, reference to session can not be cleared";
      }

      closeSession( ( *itb ).first );
      /*userDisconnected( (*itb).second );
      sessions_.erase( itb );*/
      out() << "deleting incoming session";
    } else
      ++it;
  }

  while ( !messages().empty() ) {
    MessagePointer::Locked l = ( MessagePointer ) messages().front();
    if ( l ) {
      processMessage( l.data() );
    } else {
      out() << "a message from the incoming queue could not be locked";
    }
    messages().pop_front();
  }

  if ( userListDirty_ ) {
    sendUserLists();
    userListDirty_ = false;
  }

  return BasicServer::think();
}

void Server::final( void ) {
  return BasicServer::final();
}

void Server::closeAllIncomingSessions() {
  int c = 0;
  while ( !sessions_.empty() && ++c < 50000 ) {
    closeSession( ( *sessions_.begin() ).first );
  }
}

void Server::closeSession( const SessionPointer& session ) {
  bool haveSession = false;
  {
    SessionMap::iterator it = sessions_.find( session );
    if ( it != sessions_.end() ) {
      UserPointer::Locked l = (*it).second;
      haveSession = true;

      if ( l && l->online().session() == session ) {
        l->setSession( 0 );
      } else
        out() << "error locking user";
    }
  }

  session.getUnsafe() ->stopRunning();
  if ( haveSession ) {
    userDisconnected( sessions_[ session ] );
    userListDirty_ = true;
  }
  sessions_.erase( session );
  unknownSessions_.erase( session.cast<TeamworkSession>() );
}

MessageTypeSet& Server::messageTypes() {
  return globalMessageTypeSet();
}

UserPointer Server::createUser( IdentificationMessage* msg ) {
  return new User( *msg );
}

UserPointer Server::createUser( const User* user ) {
  return new User( user );
}

void Server::addUser( UserPointer user ) {
  out() << "adding new user: \"" << user.getUnsafeData() ->safeName() << "\"";
  users_.insert( user );
}

UserPointer Server::getUser( const UserPointer& user ) {
  UserPointer ret = findUser( user );
  if ( !ret ) {
    UserPointer::Locked l = user;
    if ( l ) {
      UserPointer u = createUser( l );
      addUser( u );
      return u;
    } else {
      return 0;
    }
  } else {
    return ret;
  }
}

UserPointer Server::getUser( const UserIdentity& user ) {
  UserPointer ret = findUser( user );
  if ( !ret ) {
    UserPointer u = createUser( LockedSharedPtr<User>( new User( user ) ) );
    addUser( u );
    return u;
  } else {
    return ret;
  }
}

int Server::dispatchMessage( IdentificationMessage* msg ) {
  string username = "unknown user";
  TeamworkSessionPointer theSession;
  try {
    TeamworkSessionPointer session = msg->info().session().cast<TeamworkSession>();
    if ( !session )
      throw TeamworkError( "failed to cast server-session" );
    theSession = session;

    TeamworkSessionPointer::Locked l = session;
    if ( !l )
      throw TeamworkError( "failed to lock source-session" );

    UserPointer user = createUser( msg );
    if ( !user )
      throw TeamworkError( "could not create user" );

    UserPointer::Locked ul = user;
    if ( !ul )
      throw TeamworkError( "failed to lock user" );

    UserSet::iterator it = users_.find( user );

    username = ul->name();

    UserPointer::Locked id = identity();
    if ( id ) {
      if ( id->matchIdentity( *ul ) ) {
        throw TeamworkError( "a user tried to login with the identity of the local server" );
      }
    }

    if ( it == users_.end() ) {
      if( configuration_.serverPassword.empty() || ul->password() == configuration_.serverPassword ) {
        out() << "adding new user: " << ul->name();
        addUser( user );
      } else {
        throw TeamworkError( "server-password mismatched" );
      }
    } else {
      UserPointer::Locked nul = *it;
      if ( !nul )
        throw TeamworkError( "failed to lock user" );
      if ( !nul->match( *ul ) ) {
        ostringstream str;
        str << "authentification failed, names: \"" << nul->name() << "\" \"" << ul->name() << "\", banned: " << nul->banned();
        throw TeamworkError( str.str() );
      }
      ul = nul;
      user = *it;
    }

    l->setSessionName( ul->name() + "_" + l->sessionName() );
    l->setUser( user );

    if ( ul->online() ) {
      string peerDesc = "unknown peer";
      SessionPointer::Locked ll = ul->online().session();
      if ( ll )
        peerDesc = ll->peerDesc();
      out() << "the user " + ul->name() + " logged in twice, the first one from " << peerDesc << " is disconnected";
      send<SystemMessage>( ul->online().session().getUnsafeData(), SystemMessage::BadAuthentication, "another use with the name " + ul->name() + " logged in" );
      send<SystemMessage>( msg->info().session().getUnsafeData(), SystemMessage::AlreadyLoggedIn, "the user " + ul->name() + " was already logged in" );
      closeSession( ul->online().session() );
    }

    ul->setSession( msg->info().session() );
    sessions_[ session.cast<SessionInterface>() ] = user;
    unknownSessions_.erase( session );
    out() << "login successful: \"" << user.getUnsafeData() ->name() << "\"";
    send<SystemMessage>( l, SystemMessage::LoginSuccess, string( "welcome to this teamwork-server" ) );
    userConnected( user );

    userListDirty_ = true;
  } catch ( exception & exc ) {
    string addrInfo;
    TeamworkSessionPointer::Locked l = theSession;
    if ( l )
      addrInfo = "from " + l->peerDesc() + " ";
    ;

    if ( theSession )
      unknownSessions_.erase( theSession );

    out() << "login of user \"" << username << "\" " << addrInfo << "failed: " << exc.what();

    send<SystemMessage>( msg->info().session().getUnsafeData(), SystemMessage::LoginFailedUnknown, "reason: " + string( exc.what() ) );

  }
  return 0;
}

int Server::dispatchMessage( TextMessage* msg ) {
  out() << "got text-message: " << msg->text();
  return 0;
}

int Server::dispatchMessage( MessageInterface* /*msg*/ ) {
  out() << "got unknown message-type";
  return 0;
}

int Server::dispatchMessage( SystemMessage* msg ) {
  out() << "got system-message: " << msg->messageAsString() << ": " << msg->text();
  switch ( msg->message() ) {
    case SystemMessage::GetUserList: {
      //SessionPointer::Locked l = msg->info().session();
      if ( msg->info().session() ) {
        list<UserPointer> users = mapValues( sessions_ );
        if ( identity() )
          users.push_back( identity() );

        out() << "sending user-list of size " << users.size();

        sendReply<UserListMessage>( msg, users, msg->info().user() );
      }

    };
    break;
    default:
    break;
  }
  return 0;
}

UserPointer Server::findSessionUser( const SessionPointer& session ) {
  SessionMap::iterator it = sessions_.find( session );
  if ( it == sessions_.end() )
    return UserPointer();
  else
    return ( *it ).second;
}
}

int Teamwork::Server::dispatchMessage( ForwardMessage * msg ) {
  out( Logger::Debug ) << "forwarding message";

  UserPointer u( new User( msg->target() ) );
  UserSet::iterator it = users_.find( u );

  TeamworkSession* sess = msg->info().session().cast<TeamworkSession>().getUnsafe();
  if ( !sess ) {
    out() << "got forward-message from unknown session";
    return 0;
  }

  try {
    if ( it != users_.end() ) {
      ///@todo check if the given session (user) is authorized to send data to the given target(check for connection), and check whether the source-user-information in the message fits the real source
      UserPointer::Locked l = *it;
      if ( l ) {
        if ( l->online() && l->online().session() ) {
          out( Logger::Debug ) << "forwarding a message from " << msg->source().name() << " to " << msg->target().name();
          l->online().session().getUnsafe() ->sendMessage( msg );
        } else {
          if ( msg->storeOnServer() ) {
            out( Logger::Debug ) << "failed to forward a message from " << msg->source().name() << " to " << msg->target().name() << ", storing it on the server";
            sendReply<SystemMessage>( msg, SystemMessage::StoredOnServer, "" );
          } else {
            out( Logger::Debug ) << "failed to forward a message from " << msg->source().name() << " to " << msg->target().name() << " because the target is not online";
            sendReply<SystemMessage>( msg, SystemMessage::BadTarget, "" );
          }
        }
      } else {
        err() << "internal error: could not lock user";
        throw string( "internal error" );
      }
    } else {
      throw string( "the target-user does not exist" );
    }
  } catch ( string s ) {
    out( Logger::Debug ) << "failed to forward a message from " << msg->source().name() << " to " << msg->target().name() << " reason: " << s;
    sendReply<SystemMessage>( msg, SystemMessage::BadTarget, s );
  }

  return 1;
}

void Teamwork::Server::setConfiguration( const ServerConfiguration& conf ) {
  configuration_ = conf;
  if( !configuration_.serverName.empty() && !identity() ) {
    setIdentity( new User( conf.serverName ) );
  }
  for( ServerConfiguration::UserSet::iterator it = configuration_.registeredUsers.begin(); it != configuration_.registeredUsers.end(); ++it ) {
    UserPointer::Locked u = new User( *it );
    UserPointer ru = findUser( u );
    if( ru ) {
      ///The user already exists, update it
      UserPointer::Locked rlu = ru;
      if( rlu ) {
        (*rlu) = *it;
        rlu->setSession( rlu->online().session() ); ///Session is used to track the state of the user, so give a hint using that interface that something about the user changed.
      }
    } else {
      addUser( createUser( u ) );
    }
  }
}

const Teamwork::ServerConfiguration& Teamwork::Server::configuration() const {
  return configuration_;
}

void Teamwork::Server::processMessage( MessageInterface* msg ) {
  dispatcher_( msg );
}


// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on

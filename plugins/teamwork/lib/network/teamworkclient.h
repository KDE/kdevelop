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
#ifndef TEAMWORK_CLIENT_H
#define TEAMWORK_CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "message.h"
#include "pointer.h"
#include "basicserver.h"
#include "basicsession.h"
#include <string>
#include <list>
#include <map>
#include <set>
#include "sharedptr.h"
#include "teamworkmessages.h"
#include "teamworkserver.h"
#include "teamworkservermessages.h"
#include "user.h"
#include "messagesendhelper.h"
#include "forwardsession.h"

using namespace Tree;

namespace Teamwork {
struct ClientSessionDesc {
  UserPointer loginUser; ///user as which we are logged in
  SessionPointer session;
  ClientSessionDesc( const UserPointer& asUser = UserPointer(), const SessionPointer& _session = SessionPointer() ) {
    loginUser = asUser;
    session = _session;
  }
};
/*  typedef Binder< AllTeamworkServerMessages >::Append< UserListMessage >::Result
  AllTeamworkClientMessages;*/

struct SessionPointerCompare {
public:
  bool operator () ( const SessionPointer& s1, const SessionPointer& s2 ) const {
    return s1.getUnsafeData() < s2.getUnsafeData();
  }
};

class ClientSessionHandler;

BIND_LIST_2( TeamworkClientDispatchMessages , ForwardMessage, UserListMessage );


/// A teamwork-client generally is the same as a teamwork-server, except it not only supports incoming connections, but also outgoing ones and connections forwarded through a server, and may be associated with a user-identity.
class Client : public Server {
    typedef map< ServerInformation, ClientSessionDesc > ClientSessionMap;
    ClientSessionMap clientSessions_;
    MessageDispatcher< Client, TeamworkClientDispatchMessages > dispatcher_;
    friend class ClientSessionHandler;
    bool needUserUpdate_;

  protected:
    virtual bool think();

    ///Is called whenever a connected server sends its list of connected users
    virtual void gotUserList( const std::list<UserPointer>& /*users*/ ) {}

    ///this could be used to create a custom session deriven from TeamworkSession
    virtual SessionPointer createSession( BasicTCPSocket* sock ) {
      return Server::createSession( sock );
    }

    virtual bool registerSession( SessionPointer session ) {
      if ( ! Server::registerSession( session ) )
        return false;

      if ( session.getUnsafeData() ->sessionDirection() != SessionInterface::Incoming )
        return true; ///Only send an identification-message to incoming connections, outgoing ones are sent directly while connecting

      if ( identity() ) {
        UserPointer::Locked l = identity();
        if ( l )
          send<IdentificationMessage>( session.getUnsafe(), *l );
        else
          err() << "could not lock user to send identifaction to a new client";
      } else {
        out() << "this server has no identity";
      }

      return true;
    };

    virtual void userDisconnected( const Teamwork::UserPointer& user );

    ///this is called whenever a connection is successfully established to a server
    virtual void connectedToServer( const ClientSessionDesc& /*session*/, const ServerInformation& /*server*/ );
    ///this is called whenever the client disconnects from a connected server in any way
    virtual void disconnectedFromServer( const ClientSessionDesc& /*session*/, const ServerInformation& /*server*/ );

    virtual void processMessage( MessageInterface* msg );

  public:
    Client( ServerInformation serverInfo, LoggerPointer logger = new Logger() ) : Server( serverInfo, logger ), dispatcher_( *this ), needUserUpdate_( false ) {
      //    messageTypes().registerMessageTypes<AllTeamworkClientMessages> ();
      //dispatcher_.print( cout );
      allowIncoming( false );
    }

    virtual ~Client() {
      disconnectAllServers();
    }

    ///when no user is given, the identity-user is used. If that is not set, the connect fails.
    void connectToServer( const ServerInformation& server, const UserPointer& asUser = UserPointer() ) ;
    void disconnectFromServer( const ServerInformation& server ) ;
    void disconnectAllServers () ;

    bool isConnectedToServer( const ServerInformation& server ) ;
    ///may return invalid pointer when connectedToServer(...) == false
    TeamworkSessionPointer sessionToServer( const ServerInformation& server ) ;

    //int dispatchMessage( IdentificationMessage* msg );
    int dispatchMessage( ForwardMessage* msg );
    int dispatchMessage( MessageInterface* msg ) ;
    int dispatchMessage( UserListMessage* msg );
};
};

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on

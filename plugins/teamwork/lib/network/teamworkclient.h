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
#ifndef TEAMWORK_CLIENT_H
#define TEAMWORK_CLIENT_H

#include <list>
#include <map>

#include "networkfwd.h"
#include "safesharedptr.h"
#include "teamworkserver.h"
#include "user.h"

namespace Teamwork {

class ClientSessionHandler;
class UserListMessage;

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
    return s1.unsafe() < s2.unsafe();
  }
};

/// A teamwork-client generally is the same as a teamwork-server, except it not only supports incoming connections, but also outgoing ones and connections forwarded through a server, and may be associated with a user-identity.
class Client : public Server {
    typedef map< ServerInformation, ClientSessionDesc > ClientSessionMap;
    ClientSessionMap clientSessions_;
    friend class ClientSessionHandler;
    bool needUserUpdate_;

  protected:
    virtual bool think();

    ///Is called whenever a connected server sends its list of connected users
    virtual void gotUserList( const std::list<UserPointer>& /*users*/ );

    ///this could be used to create a custom session deriven from MultiSession
    virtual SessionPointer createSession( BasicTCPSocket* sock );

    virtual bool registerSession( SessionPointer session );

    virtual void userDisconnected( const Teamwork::UserPointer& user );

    ///this is called whenever a connection is successfully established to a server
    virtual void connectedToServer( const ClientSessionDesc& /*session*/, const ServerInformation& /*server*/ );
    ///this is called whenever the client disconnects from a connected server in any way
    virtual void disconnectedFromServer( const ClientSessionDesc& /*session*/, const ServerInformation& /*server*/ );

    virtual void processMessage( MessageInterface* msg );

  public:
    explicit Client( ServerInformation serverInfo, LoggerPointer logger = new Logger() );

    virtual ~Client();

    ///when no user is given, the identity-user is used. If that is not set, the connect fails.
    void connectToServer( const ServerInformation& server, const UserPointer& asUser = UserPointer() ) ;
    void disconnectFromServer( const ServerInformation& server ) ;
    void disconnectAllServers () ;

    bool isConnectedToServer( const ServerInformation& server ) ;
    ///may return invalid pointer when connectedToServer(...) == false
    MultiSessionPointer sessionToServer( const ServerInformation& server ) ;

    //int receiveMessage( IdentificationMessage* msg );
    int receiveMessage( ForwardMessage* msg );
    int receiveMessage( MessageInterface* msg ) ;
    int receiveMessage( UserListMessage* msg );
};
}

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on

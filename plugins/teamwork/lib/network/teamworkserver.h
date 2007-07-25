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

/** @Todo hide all this behind an interface
 * */

#ifndef TEAMWORKSERVER_H
#define TEAMWORKSERVER_H

#include <string>
#include <map>
#include <set>

#include "networkfwd.h"
#include "safesharedptr.h"
#include "sessioninterface.h"
#include "messageinterface.h"
#include "basicserver.h"
#include "user.h"
#include "messagesendhelper.h"
#include "serverconfig.h"

namespace Teamwork {
class Server;
class IdentificationMessage;
class ServerInformation;

class SystemMessage;
class ForwardMessage;
class IdentificationMessage;
class TextMessage;

class ForwardSession;
typedef SafeSharedPtr<ForwardSession> ForwardSessionPointer;

class MultiSession;
typedef SafeSharedPtr<MultiSession> MultiSessionPointer;
typedef WeakSafeSharedPtr<MultiSession> WeakMultiSessionPointer;

class Server : public BasicServer, public MessageSendHelper {
    typedef std::map< SessionPointer, UserPointer > SessionMap;
    typedef std::set< MultiSessionPointer > SessionSet;

    UserPointer ident_;

  public:
    typedef std::set< UserPointer, UserPointer::ValueSmallerCompare > UserSet;

    Server( const ServerInformation& inf, const LoggerPointer& logger );
    ~Server();

    ///If the server-name in the configuration is not empty, and there currently is no local identity std::set, the local identity is std::set to that name.
    void setConfiguration( const ServerConfiguration& conf );

    const ServerConfiguration& configuration() const;


    ///The following two functions can be used to make UserPointers consistent through a longer session where the server may be closed/opened multiple times:
    void getUserSet( UserSet& users );

    ///Insert the given user-std::set into the server's own.
    void insertUserSet( const UserSet& users );


    ///If the user is not registered yet, the user is either created, copied and returned, or zero is returned.
    virtual UserPointer getUser( const UserPointer& user );

    ///Creates and registers a user with that identity if it does not already exist. May return zero on conflict.
    virtual UserPointer getUser( const UserIdentity& user );

    MessageTypeSet& messageTypes();

    ///These two should be overridden to use derived user-classes.
    virtual UserPointer createUser( IdentificationMessage* msg );
    virtual UserPointer createUser( const User* user );


    int receiveMessage( SystemMessage* msg );
    int receiveMessage( ForwardMessage* msg );
    int receiveMessage( IdentificationMessage* msg );
    int receiveMessage( TextMessage* msg );
    int receiveMessage( MessageInterface* msg );

    ///Tries to find the user(together with appropriately filled online-information) among connected users, servers, and users available through another connected server
    UserPointer findUser( const UserPointer& user );

    UserPointer findUser( const UserIdentity& user );

    ///Returns the user associated with the given session, may return invalid pointer.
    virtual UserPointer findSessionUser( const SessionPointer& session );

    virtual void closeSession( const SessionPointer& session );

    virtual void closeAllIncomingSessions();

        /**Associate this client with a user-identity. May be invalid( is initialized as such ).
    Without this identity correctly std::set, forwarded messages can not be dispatched. The identity will also be broadcasted to new clients that connect to the server, but not to already connected ones */
    void setIdentity( const UserPointer& user );
    ///Returns the user-identity currently associated with this client. May be invalid.
    virtual UserPointer identity() const;

  protected:

    virtual SessionPointer createSession( BasicTCPSocket* sock );

    virtual bool registerSession( SessionPointer session );

    ///Internally maps the given user to his session. Fails if the user already has a session
    virtual bool registerSessionUser( const UserPointer& user );

    virtual void initial( void );

    virtual bool think();

    virtual void final( void );

    virtual void processMessage( MessageInterface* msg );

    ///this is called whenever a new user successfully logged into the server, or when a connected server has sent an identification-message
    virtual void userConnected( const UserPointer& /*user*/ ) {}

    ///called whenever a user leaves the server(or connection to a server that had an identification is lost)
    virtual void userDisconnected( const UserPointer& /*user*/ ) {}

  private:
    SessionMap sessions_;
    SessionSet unknownSessions_;
    UserSet users_;

    ServerConfiguration configuration_;

    bool userListDirty_;

    void sendUserLists();

    void addUser( UserPointer user );
};

///Encapsulates information about a server to connect to
class ServerInformation {
    std::string addr_;
    int port_;
  public:
    explicit ServerInformation( const std::string& addr = "", int port = 0 ) {
      addr_ = addr;
      port_ = port;
    }
    bool operator < ( const ServerInformation& rhs ) const {
      return addr_ < rhs.addr_ || ( addr_ == rhs.addr_ && port_ < rhs.port_ );
    }

    const std::string& addr() const {
      return addr_;
    }
    int port() const {
      return port_;
    }

    std::string desc() const {
      std::stringstream s;
      s << addr_ << ":" << port_;
      return s.str();
    }
};
}

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on

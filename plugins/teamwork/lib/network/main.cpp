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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include "basicserver.h"
#include "basicsession.h"
#include "teamworkserver.h"
#include "messageimpl.h"
#include "teamworkclient.h"
#include "dynamicmessagedispatcher.h"
#include <assert.h>
#include <vector>
#include <list>
#include "handler.h"
#include "serverconfig.h"
#include "defines.h"

using namespace  Teamwork;
#define USE_DYNAMIC_DISPATCHER

#define CHECKARGS(b) if( !(b) ) { cout << "wrong argument-count"; return 3; }
#define NEXTARG if( argc > 0 ) arg = argv[0]; else arg = ""; --argc;

int changeSettings( int argc, char * argv[] ) {
  /*NEXTARG;
  NEXTARG;*/
  std::string arg = argv[1];
  cout << "\n\n\n\n";
  if( arg == "--help" ||  arg == "--info" || arg == "?" ) {
    cout << "possible commands configuration-manipulation-commands(everything within the [] is optional): \n";
    cout << "--adduser username [password]         | Add the user to the list of known users\n";
    cout << "--serverpassword [password]         | Change the server-wide password which is used for users without an own password. If empty, access is made public.\n";
    cout << "--servername name          | Change the name of the server\n";
    cout << "--deluser username  | delete a user from the list of known users\n";
    cout << "--list         | list all registered users\n";
    cout << "--show        | show current settings of the server\n";
    cout << "--setrights username [banned][admin][trusted]        | change the rights of the user(no argument for normal user-rights)\n";
    cout << "--setport [port]        | change the port the server is opened on(if the port is left away the default-port("<<STANDARDPORT<<") is used)\n";
    cout << "--setbind [address]        | change the address the server is bound to(if it is left away the default(0.0.0.0) is used)\n";
    cout << "\nAll these commands manipulate the configuration-file created in the run-directory. \nThe settings will then apply for the next start of the server.\n";
    cout << "only one of the commands can be called at one time.\n";
    return 0;
  }

  ServerConfiguration conf;
  if( !Teamwork::loadServerConfiguration( conf ) ) {
    cout << "could not load the configuration-file" << endl;
  }
  bool changed = false;

  if( arg == "--setport" ) {
    NEXTARG;
    if( argc > 2 ) {
      conf.port = strtol( argv[2], 0, 10 );
    } else {
      conf.port = STANDARDPORT;
    }
    cout << "changed the port to" << conf.port << endl;
    changed = true;
  }
  if( arg == "--setbind" ) {
    if( argc > 2 ) {
      conf.bind = argv[2];
    } else {
      conf.bind = "0.0.0.0";
    }
    cout << "changed the bound address to" << conf.bind << endl;
    changed = true;
  }

  if( arg == "--setrights" ) {
    CHECKARGS( argc > 2 );
    std::string username = argv[2];

    int rights = User::UserRights;
    
    for( int n = 3; n < argc; n++ ) {
      if( argv[n] == std::string( "admin" ) ) rights |= User::AdminRights;
      if( argv[n] == std::string( "trusted" ) ) rights |= User::TrustedRights;
    }

    for( std::set<User>::iterator it = conf.registeredUsers.begin(); it != conf.registeredUsers.end(); ++it ) {
      //UserPointer::Locked l = *it;
      if( it->name() == username ) {
        User u = *it;
        const_cast<User&>(*it).setRights( rights );
        changed = true;
        cout << "successfully set rights for" << it->name() << ":" << it->rightsAsString() << endl;
        break;
      }
    }
    if( !changed )
      cout << "could not find user \"" << username << "\"" << endl;
  }

  if( arg == "--show" ) {
    cout << "server-name: \"" << conf.serverName << "\"\n";
    cout << "server-password: \"" << conf.serverPassword << "\"\n";
    cout << "count of registered users:" << conf.registeredUsers.size() << endl;
    cout << "locally bound to address:" << conf.bind << "on port:" << conf.port << endl;
  }

  if( arg == "--list" ) {
    cout << "users:" << endl;
    for( std::set<User>::iterator it = conf.registeredUsers.begin(); it != conf.registeredUsers.end(); ++it ) {
      //UserPointer::Locked l = *it;
      cout << it->name();
      if( !it->password().empty() )
        cout << " (has password)";
      cout << "rights:" << it->rightsAsString();
      cout << endl;
    }
  }
  
  if( arg == "--adduser" ) {
    CHECKARGS( argc > 2 );
    std::string username = argv[2];
    std::string password;
    if( argc > 3 )
      password = argv[3];

    bool passwordChanged = false;

    for( std::set<User>::iterator it = conf.registeredUsers.begin(); it != conf.registeredUsers.end(); ++it ) {
      //UserPointer::Locked l = *it;
      if( it->name() == username ) {
        if( !password.empty() ) {
          const_cast<User&>(*it).setPassword( password );
          passwordChanged = true;
          break;
        } else {
          cout << "the user named \"" << username << "\" already exists" << endl;
          return 1;
        }
      }
    }

    if( passwordChanged ) {
      cout << "\"" << username << "\"'s password changed to: \"" << password << "\""<< endl;
    } else {
      cout << "user \"" << username << "\" added, password: \"" << password << "\""<< endl;
      conf.registeredUsers.insert( Teamwork::User( username, password ) );
    }
    changed = true;
  }

  if( arg == "--deluser" ) {
    CHECKARGS( argc > 2 );
    std::string username = argv[2];
    std::string password;
    if( argc > 3 )
      password = argv[3];

    bool found = false;
    for( std::set<User>::iterator it = conf.registeredUsers.begin(); it != conf.registeredUsers.end(); ++it ) {
      //UserPointer::Locked l = *it;
      if( it->name() == username ) {
        found = true;
        conf.registeredUsers.erase( it );
        break;
      }
    }

    if( found ) {
      changed = true;
      cout << "user \"" << username << "\" removed" << endl;
    } else {
      cout << "user \"" << username << "\" is not registered" << endl;
      return 4;
    }
  }

  if( arg == "--serverpassword" ) {
    if( argc > 2 ) {
      conf.serverPassword = std::string( argv[2] );
      cout << "server-password changed to \"" << conf.serverPassword << "\"" << endl;
    } else {
      if( conf.serverPassword.empty() ) {
        cout << "server-password already was disabled" << endl;
      } else {
        conf.serverPassword = "";
        cout << "server-password disabled" << endl;
      }
    }
    changed = true;
  }

  if( arg == "--servername" ) {
    CHECKARGS( argc > 2 );
    conf.serverName = std::string( argv[2] );
    changed = true;
    cout << "server-name changed to \"" << conf.serverName << "\"" << endl;
  }

  if( arg == "--servername" ) {
    CHECKARGS( argc > 2 );
    conf.serverName = std::string( argv[2] );
    changed = true;
  }
  

  if( changed ) {
    if( !Teamwork::saveServerConfiguration( conf ) ) {
      cout << "failed to save the configuration-file" << endl;
      return 2;
    }
  } else {
    cout << "bad arguments, see --help" << endl;
  }
  
  return 1;
}

using namespace Teamwork;

#ifndef USE_DYNAMIC_DISPATCHER

///Version that uses the static message-dispatcher
class StandaloneServer : public Teamwork::Server {
    typedef Binder< RawMessage > :: Append< TextMessage > :: Result MessagesToDispatch;
  public:
    StandaloneServer( int port, std::string bind ) : Teamwork::Server( ServerInformation( bind, port ), new Logger() ) {
    }
    virtual ~StandaloneServer() {}

    int receiveMessage( TextMessage* msg ) {
      out() << "standalone-server got text-message:" << msg->text() << "real message-type:" << msg->name();
      return 1;
    }

    int receiveMessage( MessageInterface* msg ) {
      UserPointer::Locked lu = msg->info().user();
      out() << "standalone-server got unknown message:" << msg->name() << "from:" << (lu ? lu->name() : "unknown user" );
      return 1;
    }

    virtual bool handleMessage( MessagePointer m ) throw() {
      MessageDispatcher< StandaloneServer, MessagesToDispatch > dispatcher(*this);
      dispatcher( m.lock ().data() );
      return Teamwork::Server::handleMessage( m );
    }
};

#else

///Version that uses the dynamic message-dispatcher (use these as examples)
class StandaloneServer : public Teamwork::Server {
    DynamicMessageDispatcher dispatcher_;
  public:
    StandaloneServer( int port, std::string bind ) : Teamwork::Server( ServerInformation( bind, port ), new Logger() ) {
      dispatcher_.registerCallback<TextMessage>( this, &StandaloneServer::receiveTextMessage );
      dispatcher_.registerCallback<MessageInterface>( this, &StandaloneServer::receiveUnknownMessage );
    }
    virtual ~StandaloneServer() {}

    int receiveTextMessage( const SafeSharedPtr<TextMessage>& smsg ) {
      LockedSharedPtr<TextMessage> msg = smsg;
      out() << "standalone-server got text-message:" << msg->text() << "real message-type:" << msg->name();
      return 1;
    }

    int receiveUnknownMessage( const SafeSharedPtr<MessageInterface>& smsg ) {
      LockedSharedPtr<MessageInterface> msg = smsg;
      UserPointer::Locked lu = msg->info().user();
      out() << "standalone-server got unknown message:" << msg->name() << "from:" << (lu ? lu->name() : "unknown user" );
      return 1;
    }

    virtual bool handleMessage( MessagePointer m ) throw() {
      if( dispatcher_( m ) )
        return true;
      return Teamwork::Server::handleMessage( m );
    }
};

#endif

int server() {
  ServerConfiguration conf;
  if( !Teamwork::loadServerConfiguration( conf ) ) {
    cout << "could not load the configuration-file, using defaults" << endl;
  }

  SafeSharedPtr<StandaloneServer> serv = new StandaloneServer( conf.port, conf.bind );
  serv.unsafe()->setConfiguration( conf );
  serv.unsafe() ->start();
  serv.unsafe() ->allowIncoming( true );
  cout << "server running on port" << conf.port << endl;

  serv.unsafe() ->join();
  return 0;
}


void test2() {
  globalMessageTypeSet().stats();
  SafeSharedPtr<Client> client1 = new Client( ServerInformation( "0.0.0.0", 8155 ) );
  client1.unsafe() ->allowIncoming( true );
  client1.unsafe() ->start();
  SafeSharedPtr<Client> client2 = new Client( ServerInformation( "0.0.0.0", 8156 ) );
  client2.unsafe() ->allowIncoming( true );
  client2.unsafe() ->start();
  {
    SafeSharedPtr<Client>::Locked l = client1;
    l->connectToServer( ServerInformation( "127.0.0.1", 8156 ), new User( "david", "honk", "I am a user" ) );
  }

  client2.unsafe() ->join();
}


int main( int argc, char * argv[] ) {
  if( argc > 1 ) {
    if( strcmp(argv[1], "test") != 0 ) {
      return changeSettings( argc, argv );
    } else {
      test2();
    }
  }
  server();

  return EXIT_SUCCESS;
}
// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on

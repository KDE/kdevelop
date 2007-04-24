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

#include "sharedptr.h"
#include <cc++/socket.h>
#include <cstdlib>
#include <vector>
#include <exception>
#include <map>
#include <list>
#include "interfaces.h"
#include <typeinfo>
#include <sstream>
#include "message.h"
#include "messageimpl.h"
#include "helpers.h"
#include "basicsession.h"
#include "server.h"


#define SLEEPTIME 50

using namespace std;

typedef char StandardDataType;


namespace Teamwork {

BasicTCPSocket::BasicTCPSocket( InetAddress &ia, int port ) : TCPSocket( ia, port ) {}
;

bool BasicTCPSocket::onAccept( const InetHostAddress &ia, tpport_t port ) {
  setCompletion( false );
  cout << "accepting from: " << ia.getHostname() << ":" << port << endl;

  return true;
}

/**The MessageId holds all information necessary to Identify a Message and build an Object from it.
  The IdList is an inheritance-chain. Messages whose list begins with IDs of other Messages must be specializations of those, and the parent-types should be able to handle those too.
*/

void BasicServer::buildSocket() {
  closeSocket();
  failed_ = false;

  try {
    server_ = new BasicTCPSocket( addr, port_ );
    out() << "server is listening on " << addr << ":" << port_;
  } catch ( Socket * socket ) {
    failed_ = true;
    tpport_t port;
    int erro = socket->getErrorNumber();
    InetAddress saddr = ( InetAddress ) socket->getPeer( &port );
    err() << "socket error " << saddr.getHostname() << ":" << port << " = " << erro;
    if ( erro == Socket::errBindingFailed ) {
      err() << "bind failed; port busy";
    } else {
      err() << "client socket failed";
    }
    if ( server_ )
      delete server_;
    server_ = 0;
  }
}

void BasicServer::run() {
  lockCountUp();
  out() << "server started";
  bool needMore = false;
  while ( !exit_ ) {
    lockCountDown(); ///leave room time other threads that try to lock this one
    if ( server_ && server_->isPendingConnection( needMore ? 1 : SLEEPTIME ) ) {
      lockCountUp();
      try {
        InetHostAddress next = server_->getRequest();
        if ( !allowIncoming_ ) {
          out() << "refusing session for client " << next;
          server_->reject();
        } else {
          out() << "creating session for client " << next;
          SessionPointer tcp = createSession( server_ );
          if ( tcp ) {
            out() << "session created" ;
            if ( !registerSession( tcp ) ) {
              server_->reject();
              out() << "session rejected";
            } else {
              tcp.getUnsafe() ->startSession();
            }
          } else {
            out() << "session was rejected";
            server_->reject();
          }
        }
      } catch ( Socket * socket ) {
        tpport_t port;
        int err = socket->getErrorNumber();
        InetAddress saddr = ( InetAddress ) socket->getPeer( &port );
        cerr << "socket error " << saddr.getHostname() << ":" << port << " = " << err << endl;
        if ( err == Socket::errBindingFailed )
          cerr << "bind failed; port busy" << endl;
        else
          cerr << "client socket failed" << endl;
      }
    } else {
      sleep( 10 );
      lockCountUp();
    }
    needMore = think();

    if ( _Shared_count() == 1 && selfPointer_ ) { ///The server is only referenced by itself, so it may be deleted
      out() << "server is closing because the external reference-count reached zero";
      exit_ = true;
    }
  }
  closeSocket();

  out() << "server exiting";
  stopRunning();
  exit();
}

void BasicServer::allowIncoming( bool allow ) {
  if ( allow ) {
    out() << "incoming connections activated, opening socket";
    buildSocket();
  } else {
    out() << "incoming connections deactivated, closing socket";
    closeSocket();
  }
  allowIncoming_ = allow;
}

    void closeSocket() {
      if( server_ ) delete server_;
      server_ = 0;
    }

    /**Should be overridden to create own types derived from BasicTCPSession, can return 0*/
    virtual SessionPointer createSession( BasicTCPSocket* sock ) {
      BasicTCPSession* s = new BasicTCPSession( *sock , new HandlerProxy<BasicServer>( this ), messageTypes_, logger_, "incoming_" );
      return s;
    }
}

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on


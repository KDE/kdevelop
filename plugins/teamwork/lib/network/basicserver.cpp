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


#include "basicserver.h"
#include "serialization.h"

#include "handler.h"
#include "messageinterface.h"
#include "basicsession.h"


#define SLEEPTIME 50

//#ifdef  CCXX_NAMESPACES
using namespace std;
//#endif

typedef char StandardDataType;


namespace Teamwork {

BasicTCPSocket::BasicTCPSocket( ost::InetAddress &ia, int port ) : ost::TCPSocket( ia, port ) {}

bool BasicTCPSocket::onAccept( const ost::InetHostAddress &ia, ost::tpport_t port ) {
  setCompletion( false );
  cout << "accepting from:" << ia.getHostname() << ":" << port << endl;

  return true;
}

/**The MessageType holds all information necessary to Identify a Message and build an Object from it.
  The IdList is an inheritance-chain. Messages whose list begins with IDs of other Messages must be specializations of those, and the parent-types should be able to handle those too.
*/

void BasicServer::buildSocket() {
  closeSocket();
  failed_ = false;

  try {
    server_ = new BasicTCPSocket( addr, port_ );
    out() << "server is listening on" << addr << ":" << port_;
  } catch ( ost::Socket * socket ) {
    failed_ = true;
    ost::tpport_t port;
    int erro = socket->getErrorNumber();
    ost::InetAddress saddr = ( ost::InetAddress ) socket->getPeer( &port );
    err() << "socket error" << saddr.getHostname() << ":" << port << "=" << erro;
    if ( erro == ost::Socket::errBindingFailed ) {
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
        ost::InetHostAddress next = server_->getRequest();
        if ( !allowIncoming_ ) {
          out() << "refusing session for client" << next;
          server_->reject();
        } else {
          out() << "creating session for client" << next;
          SessionPointer tcp = createSession( server_ );
          if ( tcp ) {
            out() << "session created" ;
            if ( !registerSession( tcp ) ) {
              server_->reject();
              out() << "session rejected";
            } else {
              tcp.unsafe() ->startSession();
            }
          } else {
            out() << "session was rejected";
            server_->reject();
          }
        }
      } catch ( ost::Socket * socket ) {
        ost::tpport_t port;
        int err = socket->getErrorNumber();
        ost::InetAddress saddr = ( ost::InetAddress ) socket->getPeer( &port );
        cerr << "socket error" << saddr.getHostname() << ":" << port << "=" << err << endl;
        if ( err == ost::Socket::errBindingFailed )
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

void BasicServer::closeSocket() {
  if ( server_ )
    delete server_;
  server_ = 0;
}

/**Should be overridden to create own types derived from BasicTCPSession, can return 0*/
SessionPointer BasicServer::createSession( BasicTCPSocket* sock ) {
  BasicTCPSession * s = new BasicTCPSession( *sock , new HandlerProxy<BasicServer>( this ), messageTypes_, logger_, "incoming_" );
  return s;
}

bool BasicServer::handleMessage( MessagePointer msg ) throw() {
  messagesToHandle_ << msg;
  return true;
}


LoggerPrinter BasicServer::err() {
  LoggerPrinter l( logger_, Logger::Error );
  l << "server: ";
  return l;
}

LoggerPrinter BasicServer::out( Logger::Level level ) {
  LoggerPrinter l( logger_, level );
  l << "server: ";
  return l;
}

void BasicServer::run();

void BasicServer::initial() {
  Thread::initial();
}

bool BasicServer::think() {
  //messagesToHandle_.clear();
  return false;
}

void BasicServer::final( void ) {
  stopRunning();
  lockCountDown();
  clearSelfPointer();
}

bool BasicServer::registerSession( SessionPointer /*session*/ ) {
  return false;
}

void BasicServer::clearSelfPointer() {
  SafeSharedPtr<BasicServer> s = selfPointer_;
  selfPointer_ = 0;
}

LoggerPointer& BasicServer::logger() {
  return logger_;
}

BasicServer::BasicServer( const char* str, int port, MessageTypeSet& messageTypes, LoggerPointer logger, bool openServer ) : Thread( ), messageTypes_( messageTypes ), logger_( logger ), failed_( false ), exit_( false ), allowIncoming_( openServer ), server_( 0 ), port_( port ), selfPointer_( this ) {
  addr = str;

  if( allowIncoming_ )
    buildSocket();
}

BasicServer::~BasicServer() {
  closeSocket();
}

bool BasicServer::isOk() {
  return !exit_ && !failed_;
}

SafeList<MessagePointer>& BasicServer::messages() {
  return messagesToHandle_;
}

void BasicServer::stopRunning() {
  exit_ = true;
}

}

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on


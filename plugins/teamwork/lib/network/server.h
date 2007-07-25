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

#ifndef TEAMWORK_SERVER_H
#define TEAMWORK_SERVER_H

#include "networkfwd.h"
#include "basicsession.h"
#include "helpers.h"
#include "messagetypeset.h"
#include "safesharedptr.h"
#include "weaksafesharedptr.h"
#include "handler.h"

namespace Teamwork {

  class BasicTCPSocket : public TCPSocket
  {
    protected:
      bool onAccept(const InetHostAddress &ia, tpport_t port);

    public:
      BasicTCPSocket(InetAddress &ia, int port);
  };

  /**This class runs the server as the run()-method is called in the same thread.
  This class never deletes itself, it can be deleted from outside once isRunning() returns false or the threads are joined, or it can be deleted through a SharedPtr/SafeSharedPtr.
  For each client that connects, it creates a session derived from BasicSession, which can also be
  created by a derived class.
   */
  class BasicServer : protected Thread, public WeakSafeShared
  {
    friend class HandlerProxy<BasicServer>;
    BroadcastAddress addr;
    LoggerPointer logger_;
    bool failed_, exit_, allowIncoming_;
    BasicTCPSocket* server_;
    MessageTypeSet& messageTypes_;
    int port_;
    SafeList<MessagePointer> messagesToHandle_;
    SafeSharedPtr< BasicServer > selfPointer_;

    void buildSocket();

    void closeSocket() {
      if( server_ ) delete server_;
      server_ = 0;
    }

    /**Should be overridden to create own types derived from BasicTCPSession, can return 0*/
    virtual SessionPointer createSession( BasicTCPSocket* sock ) {
      BasicTCPSession* s = new BasicTCPSession( *sock , new HandlerProxy<BasicServer>( this ), messageTypes_, logger_, "incoming_" );
      return s;
    }

    protected:

    ///this one is called from within another thread, so it is more useful to override processMessage(...) in the teamwork-server than this one
      virtual bool handleMessage( MessagePointer msg ) throw() {
        messagesToHandle_ << msg;
        return true;
      }


      virtual LoggerPrinter err() {
        LoggerPrinter l( logger_, Logger::Error );
        l << "server: ";
        return l;
      }

      virtual LoggerPrinter out( Logger::Level level = Logger::Info ) {
        LoggerPrinter l( logger_, level );
        l << "server: ";
        return l;
      }

      virtual void run();

      /**This is called once at the beginning of the thread*/
      virtual void initial() {
        Thread::initial();
      }
    
      /**Gets called regularly from within the server-thread
      should be overridden, can return whether it needs more cpu-time*/
      virtual bool think() {
        //messagesToHandle_.clear();
        return false;
      }
      
      /**This is called once at end of the thread*/
      virtual void final(void) {
        stopRunning();
        lockCountDown();
        clearSelfPointer();
      }

      /**In this function, the derived class should take the ownership of the session. If it refuses the ownership, it can return false.
      once it has the ownership, it also has to care about proper
      Must be ovverridden. */
      virtual bool registerSession( SessionPointer /*session*/ ) {
        return false;
      }

    ///After this is called, the object may well be deleted
      void clearSelfPointer() {
        SafeSharedPtr<BasicServer> s = selfPointer_;
        selfPointer_ = 0;
      }
      
    LoggerPointer& logger() {
      return logger_;
    }
    
    
    public:
    BasicServer( const char* str, int port, MessageTypeSet& messageTypes, LoggerPointer logger ) : Thread( ), logger_(logger), failed_( false ), exit_( false ), allowIncoming_( true ), server_(0), messageTypes_(messageTypes), port_( port ), selfPointer_( this )
      {
        addr = "255.255.255.255";
//      out() << "testing addr: " << addr.getHostname() << ":" << port;
        addr = str;
//      out() << "binding for: " << addr.getHostname() << ":" << port;

        buildSocket();
      }

      virtual ~BasicServer() {
        closeSocket();
      }

      virtual bool isOk() {
        return !exit_ && !failed_;
      }

      ///returns the reference to a synchronized list of messages that are waiting for being processed. Processed messages should be removed from that list.
      SafeList<MessagePointer>& messages() {
        return messagesToHandle_;
      }
    
    /**After this function was called, the thread is going to exit soon.
      once isRunning() returns false, it can be deleted.*/
      void stopRunning() {
        exit_ = true;
      }

    ///Returns whether the thread is running
      using Thread::isRunning;

    ///Starts the thread, should be called after the thread was constructed
      using Thread::start;

      using Thread::join;
    
    ///This can be used to allow/disallow incoming connections. Already connected clients are not kicked on deactivation.
    void allowIncoming( bool allow );
  };
}

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on

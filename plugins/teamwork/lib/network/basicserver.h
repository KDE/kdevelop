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

#ifndef TEAMWORK_SERVER_H
#define TEAMWORK_SERVER_H

#include "common.h"
#include "basicsession.h"
#include "helpers.h"
#include "message.h"
#include "pointer.h"
#include "weakpointer.h"
#include "handler.h"

namespace Teamwork {

class BasicTCPSocket : public ost::TCPSocket {
  protected:
    bool onAccept( const ost::InetHostAddress &ia, ost::tpport_t port );

  public:
    BasicTCPSocket( ost::InetAddress &ia, int port );
};

/**This class runs the server as the run()-method is called in the same ost::Thread.
This class never deletes itself, it can be deleted from outside once isRunning() returns false or the threads are joined, or it can be deleted through a SharedPtr/SafeSharedPtr.
For each client that connects, it creates a session derived from BasicSession, which can also be
created by a derived class.
 */
class BasicServer : protected ost::Thread, public WeakSafeShared {
  public:
    BasicServer( const char* str, int port, MessageTypeSet& messageTypes, LoggerPointer logger, bool openServer = true );

    virtual ~BasicServer();

    virtual bool isOk();

    ///returns the reference to a synchronized list of messages that are waiting for being processed. Processed messages should be removed from that list.
    SafeList<DispatchableMessage>& messages();

    /**After this function was called, the ost::Thread is going to exit soon.
      once isRunning() returns false, it can be deleted.*/
    void stopRunning();

    ///Returns whether the ost::Thread is running
    using ost::Thread::isRunning;

    ///Starts the ost::Thread, should be called after the ost::Thread was constructed
    using ost::Thread::start;

    using ost::Thread::join;

    ///This can be used to allow/disallow incoming connections. Already connected clients are not kicked on deactivation.
    void allowIncoming( bool allow );

  protected:

    ///this one is called from within another ost::Thread, so it is more useful to override processMessage(...) in the teamwork-server than this one
    virtual bool handleMessage( DispatchableMessage msg ) throw();

    virtual LoggerPrinter err();

    virtual LoggerPrinter out( Logger::Level level = Logger::Info );

    virtual void run();

    /**This is called once at the beginning of the ost::Thread*/
    virtual void initial();

    /**Gets called regularly from within the server-ost::Thread
    should be overridden, can return whether it needs more cpu-time*/
    virtual bool think();

    /**This is called once at end of the ost::Thread*/
    virtual void final( void );

    /**In this function, the derived class should take the ownership of the session. If it refuses the ownership, it can return false.
    once it has the ownership, it also has to care about proper
    Must be ovverridden. */
    virtual bool registerSession( SessionPointer /*session*/ );

    ///After this is called, the object may well be deleted
    void clearSelfPointer();

    LoggerPointer& logger();
  private:
    friend class HandlerProxy<BasicServer>;
    ost::BroadcastAddress addr;
    LoggerPointer logger_;
    bool failed_, exit_, allowIncoming_;
    BasicTCPSocket* server_;
    MessageTypeSet messageTypes_;
    int port_;
    SafeList<DispatchableMessage> messagesToHandle_;
    SafeSharedPtr< BasicServer > selfPointer_;

    void buildSocket();

    void closeSocket();

    /**can be overridden to create own types derived from BasicTCPSession, can return 0(reject connection). Not called for forwarded sessions.*/
    virtual SessionPointer createSession( BasicTCPSocket* sock );
};
}

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on

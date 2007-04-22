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

#ifndef BASICSESSION
#define BASICSESSION

#include <cc++/socket.h>
#include "common.h"
#include "interfaces.h"
#include "message.h"
#include "pointer.h"
#include "helpers.h"
#include "messageimpl.h"
#include <cc++/socket.h>
#include "logger.h"
#include "handler.h"
#include "weakpointer.h"

namespace Teamwork {
class User;
typedef SafeSharedPtr<User, BoostSerialization> UserPointer;
typedef WeakSafeSharedPtr<SessionInterface> WeakSessionPointer;

class SessionInterface : public /*Weak*/SafeShared {
    string sessionName_;
    UserPointer user_;

  protected:
    SessionInterface();

    /**This can be overridden to possibly handle messages directly in this object.
    The default-implementation just sends the messages to the handler which created this session.*/
    virtual bool handleMessage( DispatchableMessage msg ) throw();

    ///This can be overridden to do some work on a regular basis. Returns whether more time is needed.
    virtual bool think() = 0;

    ///This can be overridden to do some cleanup-work within the session-thread. By default it tries to lock the associated user, and call setSession(this) so the user has a chance to notice the difference.
    virtual void final();
    ;

  public:

    virtual void startSession() {}

    virtual ~SessionInterface();
    /**After this function was called, the thread is going to exit soon.
    once isRunning() returns false, it can be deleted(better call join()).*/
    virtual void stopRunning() = 0;

    virtual void stopRunningNow() = 0;

    ///This must be thread-safe
    virtual bool sendMessage( MessageInterface* msg ) = 0;

    ///Returns true if no errors occured and the session was not ended.  Must be thread-safe.
    virtual bool isOk() = 0;

    ///Returns whether the thread is running. Must be thread-safe.
    virtual bool isRunning() = 0;

    virtual string sessionName() {
      return sessionName_;
    }

    virtual void setSessionName( const string& name ) {
      sessionName_ = name;
    }

    virtual string peerDesc() {
      return string();
    }

    virtual string localDesc() {
      return string();
    }

    enum SessionType {
      Direct,
      Forwarded
  };

    ///this function must be thread-safe
    virtual SessionType sessionType() {
      return Direct;
    }

    enum SessionDirection {
      Outgoing,
      Incoming
  };

    ///this function must be thread-safe
    virtual SessionDirection sessionDirection() = 0;

    ///This function is thread-safe.  The user is not necessarily filled.
    UserPointer safeUser() const;

    ///This should never be called again once it has been set on a session, to make it thread-safe.
    void setUser( const UserPointer& user );
};

class SessionReplyManager {
    typedef map<uint, MessagePointer> WaitingMap;
    MutexInterfaceImpl* selfMutex_;
    WaitingMap waitingMessages_; ///here, all messages that wait for a reply are stored, mapped to their unique-ids.
  protected:
    SessionReplyManager( MutexInterfaceImpl* selfMutex );

    void addWaitingMessage( MessageInterface* msg );

    bool handleMessageWaiting( DispatchableMessage & msg );

    void removeAllMessages();

    virtual ~SessionReplyManager();
};

class BasicTCPSession : protected TCPSession,   /*public virtual SafeShared,*/ public SessionInterface, public SessionReplyManager {
  public:
    using Thread::setName;
    using Thread::getName;

    ///Used by the server to construct a session
    BasicTCPSession( TCPSocket &server, HandlerPointer handler_, MessageTypeSet& messages, const LoggerPointer& logger, const string& namePrefix = "" );

    ///Used by the client to connect to a server
    BasicTCPSession( const InetHostAddress &host, tpport_t port, HandlerPointer handler, MessageTypeSet& messages, const LoggerPointer& logger, const string& namePrefix = "" );

    virtual ~BasicTCPSession();

    virtual bool isOk();

    virtual void startSession();

    /**After this function was called, the thread is going to exit soon.
    once isRunning() returns false, it can be deleted(better call join()).*/
    virtual void stopRunning();

    ///This should get some tolerance in future
    virtual void stopRunningNow();

    ///THREAD-SAFE
    virtual bool sendMessage( MessageInterface* msg );

    ///Returns whether the thread is running
    virtual bool isRunning();

    ///Starts the thread, should be called after the thread was constructed
    using Thread::start;

    using Thread::join;

    using Socket::isPending;

    virtual string peerDesc();

    virtual string localDesc();

    virtual SessionDirection sessionDirection();

  protected:

    virtual LoggerPrinter err( int prio = 0 );

    virtual LoggerPrinter out( Logger::Level lv = Logger::Info );

    /**This can be reimplemented to decompress/decrypt the message.*/
    virtual void processIncomingMessage( std::vector<char>& /*data*/ ) {}
    ;

    /**This can be reimplemented to add compression or encryption. The data can be changed however desired.
    It is called just before the message is sent. */
    virtual void processOutgoingMessage( std::vector<char>& /*data*/ ) {}
    ;

    /**This can be overridden to possibly handle messages directly in this object.
    The default-implementation just sends the messages to the handler which created this session.*/
    virtual bool handleMessage( DispatchableMessage msg ) throw() ;

    ///This can be overridden to do some work on a regular basis. Returns whether more time is needed.
    virtual bool think() {
      return false;
    }

    ///This can be overridden to do final cleanup, store settings/state etc.
    virtual void final( void );

    virtual void initial();

    ///After this is called, the object may well be deleted
    void clearSelfPointer();

    virtual void initName( const string& namePrefix );

  private:
    virtual void run();

    void removeAllMessages();

    bool inputOutput();

    void serializeMessage();

    ///Takes all Data that was written away from the vector, throws StreamError
    template <class DataType>
        u32 writeData( vector<DataType>& from, u32 max );

    ///Appends the data to the given Vector, throws StreamError
    template <class DataType>
        u32 getData( vector<DataType>& to, u32 max );

    void failed( std::string reason = "" );

    void sendingResult( bool result );

    bool dataToSend();

    SessionDirection myDirection;
    static const int ConnectionTimeout;
    bool exit_, failed_, running_, hadFinal_;
    uint receivingSize_;
    HandlerPointer handler_;
    LoggerPointer logger_;
    MessageTypeSet& messages_;
    std::vector<char> lookBack_;
    SafeList<MessagePointer> messagesToSend_;
    std::vector<char> sendData_;
    MessagePointer sendingMessage_; ///when sendData_ is not empty, this holds the message that is currently sent.
    std::vector<char> receivingData_;
    SafeSharedPtr<BasicTCPSession> selfPointer_; ///This is necessary to avoid deletion of the object before the thread has completed running
};
}

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on

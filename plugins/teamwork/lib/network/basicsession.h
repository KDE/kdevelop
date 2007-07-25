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

#ifndef BASICSESSION
#define BASICSESSION

#include <cc++/socket.h>
#include <vector>

#include "networkfwd.h"
#include "safesharedptr.h"
#include "safelist.h"
#include "logger.h"
#include "sessioninterface.h"
#include <map>

namespace Teamwork {

 class SessionReplyManager {
    typedef std::map<uint, MessagePointer> WaitingMap;
    MutexInterfaceImpl* selfMutex_;
    WaitingMap waitingMessages_; ///here, all messages that wait for a reply are stored, mapped to their unique-ids.
  protected:
    SessionReplyManager( MutexInterfaceImpl* selfMutex );

    void addWaitingMessage( MessageInterface* msg );

    bool handleMessageWaiting( MessagePointer & msg );

    void removeAllMessages();

    virtual ~SessionReplyManager();
};

class BasicTCPSession : protected ost::TCPSession,   /*public virtual SafeShared,*/ public SessionInterface, public SessionReplyManager {
  public:
    using Thread::setName;
    using Thread::getName;

    ///Used by the server to construct a session
    BasicTCPSession( ost::TCPSocket &server, HandlerPointer handler_, MessageTypeSet& messages, const LoggerPointer& logger, const string& namePrefix = "" );

    ///Used by the client to connect to a server
    BasicTCPSession( const ost::InetHostAddress &host, ost::tpport_t port, HandlerPointer handler, MessageTypeSet& messages, const LoggerPointer& logger, const string& namePrefix = "" );

    virtual ~BasicTCPSession();

    virtual bool isOk();

    virtual void startSession();

    /**After this function was called, the thread is going to exit soon.
    once isRunning() returns false, it can be deleted(better call join()).*/
    virtual void stopRunning();

    ///This should get some tolerance in future
    virtual void stopRunningNow();

    ///THREAD-SAFE
    virtual bool send( MessageInterface* msg );

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
    virtual bool handleMessage( MessagePointer msg ) throw() ;

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

    ///Takes all Data that was written away from the std::vector, throws StreamError
    template <class DataType>
        u32 writeData( std::vector<DataType>& from, u32 max );

    ///Appends the data to the given std::vector, throws StreamError
    template <class DataType>
        u32 getData( std::vector<DataType>& to, u32 max );

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

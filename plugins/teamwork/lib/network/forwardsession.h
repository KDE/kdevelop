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

#ifndef FORWARDSESSION_H
#define FORWARDSESSION_H

#include "handler.h"
#include "sessioninterface.h"
#include "basicsession.h"
#include "networkexport.h"

namespace Teamwork {
  class ForwardMessage;
  ///A virtual session that uses another session to forward messages through it to a specified user
  class NETWORK_EXPORT ForwardSession : public SessionInterface, public SessionReplyManager {
      friend class MultiSession;
      MultiSessionPointer session_;
      UserPointer targetUser_;
      UserPointer sourceUser_;
      LoggerPointer logger_;
      bool notRunning_;
      MessageTypeSet& messageTypes_;
      HandlerPointer handler_;


      virtual LoggerPrinter err( int prio = 0 );

      virtual LoggerPrinter out( Logger::Level lv = Logger::Info );

    public:
      ForwardSession( const UserPointer& sourceUser, const UserPointer& targetUser, const MultiSessionPointer& session, const LoggerPointer& logger, MessageTypeSet& messageTypes, HandlerPointer handler = 0 );

      ~ForwardSession();

      virtual bool isOk();

      virtual bool isRunning();

      virtual bool send( MessageInterface* msg );

      virtual void stopRunning();

      virtual void stopRunningNow();

      virtual bool think();

      virtual void startSession();

      UserPointer target();

      virtual SessionDirection sessionDirection();

      ///Must only be called from within session_!
      bool handleForwardMessage( const SafeSharedPtr<ForwardMessage>& msg );

      ///Must only be called from within session_!
      virtual bool handleMessage( MessagePointer msg ) throw();

      virtual SessionType sessionType();

    protected:
      virtual void final();
  };

  ///This is a class that implements the session-interface, and just stores the messages that should be sent in a list. It can be used for debugging.
  class NETWORK_EXPORT FakeSession : public SessionInterface {
      LoggerPointer logger_;
      MessageTypeSet& messageTypes_;
      HandlerPointer handler_;
      std::list<MessagePointer> receivedMessages_;

      virtual LoggerPrinter err( int prio = 0 );

      virtual LoggerPrinter out( Logger::Level lv = Logger::Info );

    public:
      FakeSession( const UserPointer& targetUser, const LoggerPointer& logger, MessageTypeSet& messageTypes, HandlerPointer handler = 0 );

      virtual bool isOk();

      virtual bool isRunning();

      virtual bool send( MessageInterface* msg );

      virtual void stopRunning();

      virtual void stopRunningNow();

      virtual bool think();

      virtual void startSession();

      virtual SessionDirection sessionDirection();

      ///Must only be called from within session_!
      virtual bool handleMessage( MessagePointer msg ) throw();

      virtual SessionType sessionType();

      std::list<MessagePointer>& sentMessages();


      /**This takes the first message off the stack of sent messages, serializes and deserializes it, and then returns the deserialized message.
         All exceptions from the serialization- and deserialization-process are passed by, additionally errors are indicated by throwing a const char*.
      */
      MessagePointer::Locked getFirstMessage() throw( const char* );
  };
}

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on

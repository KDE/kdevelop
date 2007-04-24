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

#ifndef FORWARDSESSION_H
#define FORWARDSESSION_H

#include "teamworkclient.h"
#include "teamworksession.h"

namespace Teamwork {
  ///A meta-session that uses another real session to forward messages through it to a specified user
  class ForwardSession : public SessionInterface, public SessionReplyManager {
      friend class TeamworkSession;
      TeamworkSessionPointer session_;
      UserPointer targetUser_;
      UserPointer sourceUser_;
      LoggerPointer logger_;
      bool notRunning_;
      MessageTypeSet& messageTypes_;
      HandlerPointer handler_;


      virtual LoggerPrinter err( int prio = 0 ) {
        LoggerPrinter p ( logger_, Logger::Error );
        if ( prio )
          p << prio << ":";
        p << "error in session " << "(" << sessionName() << "): ";
        return p;
      }

      virtual LoggerPrinter out( Logger::Level lv = Logger::Info ) {
        LoggerPrinter p ( logger_, lv );
        p << "in session " << "(" << sessionName() << "): ";
        return p;
      }

    public:
      ForwardSession( const UserPointer& sourceUser, const UserPointer& targetUser, const TeamworkSessionPointer& session, const LoggerPointer& logger, MessageTypeSet& messageTypes, HandlerPointer handler = 0 ) : SessionReplyManager( session.getUnsafeData() ), session_( session ), targetUser_( targetUser ), sourceUser_( sourceUser ), logger_( logger ), notRunning_( false ), messageTypes_( messageTypes ), handler_( handler ) {
        TeamworkSessionPointer::Locked l = session_;
        if ( l ) {
          l->registerForwardSession( targetUser_, this );
        } else {
          ///error
        }
        setUser( targetUser );
        UserPointer::Locked lu = targetUser;
        if ( lu ) {
          setSessionName( "forward_" + lu->name() );
          SessionPointer::Locked ls = lu->online().session();
          if ( ls && ls->isRunning() ) {
            out( Logger::Warning ) << "creating an indirect session to " << lu->name() << ", but there already is a direct session";
          } else {
            lu->setSession( this );
          }

        } else {
          err() << "could not lock target-user of a foward-session";
        }
      }

      ~ForwardSession() {
      }

      virtual bool isOk() {
        TeamworkSessionPointer p = session_/*.get()*/;
        if ( !notRunning_ && p )
          return p.getUnsafeData() ->isOk();
        return false;
      }
      virtual bool isRunning() {
        TeamworkSessionPointer p = session_/*.get()*/;
        if ( !notRunning_ && p )
          return p.getUnsafeData() ->isRunning();
        return false;
      }


      virtual bool sendMessage( MessageInterface* msg ) {
        if ( !isRunning() )
          return false;
        TeamworkSessionPointer p = session_/*.get()*/;
        if( !p ) return false;

        msg->info().setSession( this );

        UserPointer::Locked source = sourceUser_;
        UserPointer::Locked target = targetUser_;

        if ( msg && source && target ) {
          if ( msg->needReply() )
            addWaitingMessage( msg );
          return p.getUnsafeData() ->sendMessage( messageTypes_.create<ForwardMessage>( msg, *source, *target ) );
        } else {
          err() << "failed to do the necessary locks for sending a message(message and users)";
          return false;
        }
      }

      virtual void stopRunning() {
        out( Logger::Debug ) << "forwardsession: stopRunning";
        notRunning_ = true;
        session_ = 0;
      }

      virtual void stopRunningNow() {
        out( Logger::Debug ) << "forwardsession: stopRunningNow";
        notRunning_ = true;
        session_ = 0;
      }

      virtual bool think() {
        return false;
      }

      virtual void startSession() {}

      UserPointer target() {
        return targetUser_;
      }

      virtual SessionDirection sessionDirection() {
        TeamworkSessionPointer::Locked l = session_;
        if ( !l )
          return Incoming;
        return l->sessionDirection();
      }

      ///Must only be called from within session_!
      bool handleForwardMessage( const SafeSharedPtr<ForwardMessage>& msg ) {
        out( Logger::Debug ) << "handling forward-message";
        SafeSharedPtr<ForwardMessage>::Locked l = msg;
        UserPointer::Locked lu = sourceUser_;

        if ( l && lu ) {
          if ( lu->match( l->target() ) ) {
            MessagePointer message = l->deserializeContained( messageTypes_ );
            MessagePointer::Locked lmessage = message;
            if ( lmessage ) {
              lmessage->info().setSession( this );
            } else
              err() << "ForwardSession: could not lock a deserialized message";

            handleMessage( message );
            return true;
          }
        } else {
          out() << "could not lock forward-message or local user";
        }

        return false;
      }

      ///Must only be called from within session_!
      virtual bool handleMessage( DispatchableMessage msg ) throw() {
        if ( handleMessageWaiting( msg ) )
          return true;
        if ( handler_ )
          return handler_.getUnsafeData() ->handleMessage( msg );
        return false;
      }

      virtual SessionType sessionType() {
        return Forwarded;
      }

    protected:
      virtual void final() {
        SessionInterface::final();
      }
  };

  ///This is a class that implements the session-interface, and just stores the messages that should be sent in a list. It can be used for debugging.
  class FakeSession : public SessionInterface {
      LoggerPointer logger_;
      MessageTypeSet& messageTypes_;
      HandlerPointer handler_;
      std::list<MessagePointer> receivedMessages_;

      virtual LoggerPrinter err( int prio = 0 ) {
        LoggerPrinter p ( logger_, Logger::Error );
        if ( prio )
          p << prio << ":";
        p << "error in session " << "(" << sessionName() << "): ";
        return p;
      }

      virtual LoggerPrinter out( Logger::Level lv = Logger::Info ) {
        LoggerPrinter p ( logger_, lv );
        p << "in session " << "(" << sessionName() << "): ";
        return p;
      }

    public:
      FakeSession( const UserPointer& targetUser, const LoggerPointer& logger, MessageTypeSet& messageTypes, HandlerPointer handler = 0 ) : logger_( logger ), messageTypes_( messageTypes ), handler_( handler ) {
        setUser( targetUser );
        UserPointer::Locked lu = targetUser;
        if ( lu ) {
          setSessionName( "session_fake_" + lu->name() );
        } else {
          setSessionName( "session_fake" );
          err() << "FakeSession: could not lock target-user of a fake-session";
        }
      }

      virtual bool isOk() {
        return true;
      }

      virtual bool isRunning() {
        return true;
      }

      virtual bool sendMessage( MessageInterface* msg ) {
        SafeSharedPtr<FakeSession>::Locked l( this );
        if ( l ) {
          msg->info().setSession( this );

        ///Simulate sending the message by serializing and deserializing it within a local buffer
          vector<char> vec;
          serializeMessageToBuffer( vec, *msg );
          MessagePointer nmsg = buildMessageFromBuffer( vec, globalMessageTypeSet(), 0 );
          if ( !nmsg ) {
            err() << "FakeSession: could not build message from buffer";
            return false;
          }
          MessagePointer::Locked l = nmsg;
          if ( !l ) {
            err() << "FakeSession: could not lock created message";
            return false;
          }
          l->info().setReplyMessage( msg->info().replyToMessage() );

          receivedMessages_.push_back( nmsg );
          return true;
        } else {
          err() << "FakeSession: lock-error";
        }
        return true;
      }

      virtual void stopRunning() {
        out( Logger::Debug ) << "FakeSession: stopRunning";
      }

      virtual void stopRunningNow() {
        out( Logger::Debug ) << "FakeSession: stopRunningNow";
      }

      virtual bool think() {
        return false;
      }

      virtual void startSession() {}

      virtual SessionDirection sessionDirection() {
        return Outgoing;
      }

      ///Must only be called from within session_!
      virtual bool handleMessage( DispatchableMessage msg ) throw() {
        if ( handler_ )
          return handler_.getUnsafeData() ->handleMessage( msg );
        return false;
      }

      virtual SessionType sessionType() {
        return Direct;
      }

      std::list<MessagePointer>& sentMessages() {
        return receivedMessages_;
      }


      /**This takes the first message off the stack of sent messages, serializes and deserializes it, and then returns the deserialized message.
         All exceptions from the serialization- and deserialization-process are passed by, additionally errors are indicated by throwing a const char*.
      */
      MessagePointer::Locked getFirstMessage() throw( const char* ) {
        if ( receivedMessages_.empty() )
          throw "FakeSession: no messages available";
        return receivedMessages_.front();
      }
  };

  typedef SafeSharedPtr<ForwardSession> ForwardSessionPointer;
};

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on

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

#include "serialization.h"
#include "logger.h"
#include "handler.h"
#include "basicsession.h"
#include "messageimpl.h"
#include "user.h"
#include "messagetypeset.h"
#include "helpers.h"
#define MESSAGEDEBUG 
//#define DISABLETRYCATCH
#define DISABLEUNIVERSALCATCH 
//#define DEBUGLOG(x) out( Logger::Debug ) << x
//#define DEBUGLOG(x)
#define DEBUGLOG( x ) cout << x

namespace Teamwork {
using namespace std;

SessionReplyManager::SessionReplyManager( MutexInterfaceImpl* selfMutex ) : selfMutex_( selfMutex ) {}

void SessionReplyManager::addWaitingMessage( MessageInterface* msg ) {
  waitingMessages_[ msg->uniqueId() ] = MessagePointer( msg );
}

bool SessionReplyManager::handleMessageWaiting( MessagePointer & msg ) {
  if ( msg ) {
    uint i = msg.unsafe() ->info().isReplyTo();
    if ( i ) {
      WaitingMap::iterator it = waitingMessages_.find( i );
      if ( it != waitingMessages_.end() ) {
        MessageInterface::ReplyResult ret;
        MessagePointer::Locked l = ( *it ).second;
        if ( l ) {
          if ( selfMutex_ )
            selfMutex_->lockCountDown();
          ret = l->gotReply( msg );
          if ( selfMutex_ )
            selfMutex_->lockCountUp();
          if ( !ret.messageHandled )
            msg.unsafe() ->info().setReplyMessage( ( *it ).second );
        } else {
          //err() << "could not lock waiting message";
        }

        if ( !ret.awaitingMore )
          waitingMessages_.erase( it );

        return ret.messageHandled;
      }
    }
  }
  return false;
}

void SessionReplyManager::removeAllMessages() {
  for ( WaitingMap::iterator it = waitingMessages_.begin(); it != waitingMessages_.end(); ++it ) {
    MessagePointer::Locked l = ( *it ).second;
    if ( l ) {
      if ( selfMutex_ )
        selfMutex_->lockCountDown();
      l->gotReply( MessagePointer() );
      if ( selfMutex_ )
        selfMutex_->lockCountUp();
    }
  }

  waitingMessages_.clear();
}

SessionReplyManager::~SessionReplyManager() {
  removeAllMessages();
}

SessionInterface::SessionInterface() {
}

SessionInterface::~SessionInterface() {
}

UserPointer SessionInterface::safeUser() const {
  return user_;
}

void SessionInterface::setUser( const UserPointer& user ) {
  user_ = user;
}

void SessionInterface::final() {
  /*UserPointer::Locked l = user_;
  if( l )
  l->setSession( this );*/
}


bool SessionInterface::handleMessage( MessagePointer /*msg*/ ) throw() {
  return false;
}

string toString( std::vector<char>& v ) {
  string ret;
  for ( uint a = 0; a < v.size(); a++ )
    ret += v[ a ];
  return ret;
}

void BasicTCPSession::initial( void ) {
  running_ = true;
  out() << "connecting";
  ost::TCPSession::initial();
  if ( isConnected() ) {
    out() << "connection successfull";
  } else {
    err() << "connection failed";
  }

  if ( isActive() ) {}
  else {
    err() << "connection inactive";
  }
}

bool BasicTCPSession::inputOutput() {
  bool worked = false;
#ifndef DISABLETRYCATCH
  try {
#endif
    if ( isPending( pendingInput, 0 ) ) {
      if ( !receivingSize_ ) {
        std::vector<char> sz;
        int cnt = getData( sz, 4 );
        if ( cnt == 4 && sz.size() == 4 ) {
          int s = ( *( int* ) ( &sz[ 0 ] ) );
          if ( s >= 0 && s < MAXMESSAGESIZE ) {
            receivingData_.clear();
            receivingSize_ = s;
            //out() << "receiving message of size " << receivingSize_;
          } else {
            throw StreamError( "message-header-integer is wrong: " + formatInt( s ) );
          }
        } else {
          throw StreamError( "message-header-integer is wrong, count of bytes: " + formatInt( cnt ) + " size of buffer: " + formatInt( sz.size() ) );
        }
        DEBUGLOG( "receiving message of size " << receivingSize_ );
      }

      if ( isPending( pendingInput, 0 ) ) {
        receivingSize_ -= getData( receivingData_, receivingSize_ );

        if ( receivingSize_ <= 0 ) {
          ///process a complete message
          if ( !receivingData_.empty() ) {
            processIncomingMessage( receivingData_ );
            //out() << "got: " << toString( receivingData_ );
            MessagePointer msg = buildMessageFromBuffer( receivingData_, messages_, this );
            MessagePointer::Locked l = ( MessagePointer ) msg;
            if ( l ) {
              l->info().setSession( this );
              DEBUGLOG( "handling message of size " << receivingData_.size() << " and type " << messages_.identify( l ) );
            } else
              out() << "handling not lockable message";

            if ( msg ) {
              if ( !handleMessageWaiting( msg ) )
                handleMessage( msg );
            } else {
              err() << "a received message could not be reconstructed";
            }
            worked = true;
          }

          receivingSize_ = 0;
          receivingData_.clear();
        }
        //out() << "received: " << receivingData_.size() << " yet needed: " << receivingSize_;
      }
    }
    if ( sendData_.empty() ) {
      //if( sendingMessage_ )
      sendingResult( true );
      serializeMessage();
    }
    if ( !sendData_.empty() ) {
      //out() << "writing " << sendData_.size();
      uint cnt = writeData( sendData_, sendData_.size() );
      //DEBUGLOG( "wrote " << cnt << " left: " << sendData_.size() );
      //if( sendData_.empty() )
      if ( cnt )
        worked = true;
    }

#ifndef DISABLETRYCATCH

  } catch ( const StreamError & err ) {
    failed( std::string( "stream-error in inputOuput: " ) + err.what() );
  }
#ifndef DISABLEUNIVERSALCATCH
  catch ( std::exception & err ) {
    failed( std::string( "exception occured in inputOuput: " ) + err.what() );
  } catch ( ... ) {
    failed( std::string( "unknown exception occured in inputOuput" ) );
  }
#endif
 #endif

  return worked;
}


void BasicTCPSession::serializeMessage() {
  if ( !messagesToSend_.empty() ) {
    MessagePointer::Locked mp = messagesToSend_.front().lock();
    if ( !mp )
      err() << "could not lock message";
    else {

#ifndef DISABLETRYCATCH
      try {
#endif
        sendingMessage_ = messagesToSend_.front();
        if ( !tcp() )
          throw StreamError( "tcp-stream missing" );
        std::vector<char> buffer;
        serializeMessageToBuffer( buffer, *mp );
#ifdef MESSAGEDEBUG

        MessagePointer clone;
        try {
          clone = buildMessageFromBuffer( buffer, messages_, 0 );
        } catch ( const CannotReserialize& ) {
          clone = 0;
        }
#ifndef DISABLEUNIVERSALCATCH
        catch ( ... ) {
          err() << "exception while reconstructing the message";
          throw;
        }
#endif
        try {
          if ( clone ) {
            std::vector<char> buffer2;
            serializeMessageToBuffer( buffer2, *clone.unsafe() );
            if ( buffer2.size() == buffer.size() ) {
              if ( memcmp( &buffer[ 0 ], &buffer2[ 0 ], buffer.size() ) != 0 )
                err() << "the reconstruction of a message of type \"" << mp->name() << "\" is wrong, the serialized content does not match!" ;
            } else {
              err() << "the reconstruction of a message of type \"" << mp->name() << "\" is wrong!(mismatch in size: " << buffer.size() << " -> " << buffer2.size() << ")" ;
            }
          } else {
            err() << "a message that is being sent could not be reconstructed for debugging: \"" << mp->name() << "\"";
          }
        } catch ( const CannotReserialize& ) {}
#ifndef DISABLEUNIVERSALCATCH

        catch ( ... ) {
          err() << "exception while serializing the reconstructed message";
          throw;
        }
#endif
#endif
        processOutgoingMessage( buffer );
        sendData_ = binaryInt( buffer.size() );
        sendData_ += buffer;
        //out() << "sending: " << toString( buffer );

        DEBUGLOG( "serialized outgoing message of size " << sendData_.size() << " and type " << messages_.identify( mp ) );

#ifndef DISABLETRYCATCH

      } catch ( const NonFatalSerializationError & error ) {
        err() << std::string( "could not serialize message of type \"" ) + mp->name() + "\", reason: " + std::string( error.what() );
        mp->result( false );
        if ( mp->info().isReplyTo() ) {
          MessageInterface * msg = new SystemMessage( messages_, SystemMessage::SerializationFailed, error.what() );
          msg->info().setReply( mp->info().isReplyTo() );
          messagesToSend_.pop_front();
          messagesToSend_.push_front( msg );
          return ;
        }
      }
#ifndef DISABLEUNIVERSALCATCH
      catch ( const std::exception & exc ) {
        failed( std::string( "could not serialize message of type \"" ) + mp->name() + "\", reason: " + std::string( exc.what() ) );
      } catch ( ... ) {
        failed( "could not serialize message " + mp->name() );
      }
#endif
#endif

    }
    messagesToSend_.pop_front();
  }
}

template <class DataType>
u32 BasicTCPSession::writeData( std::vector<DataType>& from, u32 max ) {
  u32 count = 0;
  if ( isPending( pendingOutput, 0 ) ) {
    while ( isPending( pendingOutput, 0 ) && isOk() && count != max ) {
      int Portion = 1000;
      if ( max ) {
        if ( count + Portion > max ) {
          Portion = max - count;
        }
      }
      u32 write = Socket::writeData( &from[ count ], Portion * sizeof( DataType ) );
      //DEBUGLOG( "wrote portion of size " << write );
      if ( write == 0 || write == 0xffffffff ) {
        if ( isPending( pendingOutput, 0 ) ) {
          throw StreamError( "writeData could not send" );
        }

        return count;
      } else {
        u32 units = write / sizeof( DataType );
        if ( write % sizeof( DataType ) != 0 ) {
          throw StreamError( "writeData wrote a wrong unit" );
        }
        count += units;
      }
    }
  }
  if ( count == from.size() ) {
    from.clear();
  } else {
    std::vector<DataType> newVec;
    newVec.resize( from.size() - count );
    memcpy( &newVec[ 0 ], &from[ count ], ( from.size() - count ) * sizeof( DataType ) );
    from = newVec;
  }

  return count;
}

template <class DataType>
u32 BasicTCPSession::getData( std::vector<DataType>& to, u32 max ) {
  u32 count = 0;
  if ( isPending( pendingInput, 0 ) ) {
    while ( isPending( pendingInput, 0 ) && isOk() && count != max ) {
      u32 osize = to.size();
      int Portion = 1000;
      if ( max ) {
        if ( count + Portion > max ) {
          Portion = max - count;
        }
      }
      to.resize( osize + Portion );
      u32 read = Socket::readData( ( char* ) & to[ osize ], Portion * sizeof( DataType ) );
      if ( read == 0 || read == 0xffffffff ) {
        to.resize( osize );
        if ( isPending( pendingInput, 0 ) ) {
          throw StreamError( "could not get data" );
        }

        //DEBUGLOG( "got " << count );
        return count;
      } else {
        u32 units = read / sizeof( DataType );
        count += units;
        to.resize( osize + units );
        if ( read % sizeof( DataType ) != 0 ) {
          throw StreamError( "getData got a wrong unit" );
        }
      }
    }
  }
  //DEBUGLOG( "got " << count );
  return count;
}

const int BasicTCPSession::ConnectionTimeout = 10000;


BasicTCPSession::BasicTCPSession( const ost::InetHostAddress &host, ost::tpport_t port, HandlerPointer handler, MessageTypeSet& messages, const LoggerPointer& logger, const string& namePrefix ) : ost::TCPSession( host, port ), SessionReplyManager( this ), myDirection( Outgoing ), exit_( false ), failed_( false ), running_( true ), hadFinal_(false), receivingSize_( 0 ), handler_( handler ), logger_( logger ), messages_( messages ), selfPointer_( this ) {
  initName( namePrefix );
}


BasicTCPSession::BasicTCPSession( ost::TCPSocket &server, HandlerPointer handler, MessageTypeSet& messages, const LoggerPointer& logger, const string& namePrefix ) : ost::TCPSession( server ), SessionReplyManager( this ), myDirection( Incoming ), exit_( false ), failed_( false ), running_( true ), hadFinal_(false), receivingSize_( 0 ), handler_( handler ), logger_( logger ), messages_( messages ), selfPointer_( this ) {
  initName( namePrefix );
}

void BasicTCPSession::failed( std::string reason ) {
  err() << "failed: " << reason.c_str();
  sendingResult( false );
  failed_ = true;
  exit_ = true;
}

void BasicTCPSession::sendingResult( bool result ) {
  if ( sendingMessage_ ) {
    MessagePointer::Locked l = sendingMessage_;
    if ( l ) {
      lockCountDown();
      l->result( result );
      lockCountUp();
      if ( result && l->needReply( ) ) {
        addWaitingMessage( l );
      }
    } else {
      err() << "could not lock message for sendingResult!";
    }
    sendingMessage_ = 0;
  }
}

LoggerPrinter BasicTCPSession::err( int prio ) {
  LoggerPrinter p ( logger_, Logger::Error );
  if ( prio )
    p << prio << ":";
  p << "error in session " << "(" << sessionName() << "): ";
  return p;
}

LoggerPrinter BasicTCPSession::out( Logger::Level lv ) {
  LoggerPrinter p ( logger_, lv );
  p << "in session " << "(" << sessionName() << "): ";
  return p;
}

void BasicTCPSession::clearSelfPointer() {
  SafeSharedPtr<BasicTCPSession> s = selfPointer_;
  selfPointer_ = 0;
}

void BasicTCPSession::initName( const string& namePrefix ) {
  static int currentNum = 1;
  currentNum++;
  ostringstream o;
  o << currentNum;
  setSessionName( namePrefix + o.str() );
}

bool BasicTCPSession::dataToSend() {
  return !sendData_.empty();
}

void BasicTCPSession::run() {
  lockCountUp();
  while ( !exit_ ) {
    think();
    if ( !inputOutput() ) {
      lockCountDown();
      if ( !isPending( dataToSend() ? ( ost::Socket::Pending ) ( pendingInput | pendingOutput ) : pendingInput, 20 ) )
        sleep( 10 );
      lockCountUp();
    } else {
      lockCountDown();
      ///window for external mutex-requests
      lockCountUp();
    }
    if ( !isConnected() || !isActive() ) {
      out() << "session is closing because the connection is lost";
      exit_ = true;
    }

    if ( _Shared_count() == 1 && selfPointer_ ) { ///The session is only referenced by itself, so it may be deleted
      out() << "session is closing because the external reference-count reached zero";
      exit_ = true;
    }
  }

  out() << "session exiting";
  exit();
}

BasicTCPSession::~BasicTCPSession() {
  stopRunningNow();
  terminate();
  removeAllMessages();
}

bool BasicTCPSession::isOk() {
  return !exit_ && !failed_;
}

void BasicTCPSession::startSession() {
  start();
}

void BasicTCPSession::final( void ) {
  if( hadFinal_ ) return;
  SessionInterface::final();
  hadFinal_ = true;
  DEBUGLOG( "final" );
  removeAllMessages();
  exit_ = true;
  lockCountDown();
  running_ = false;
  endSocket(); /// think about how safe it is to call this
  clearSelfPointer();
}

bool BasicTCPSession::handleMessage( MessagePointer msg ) throw() {
  if ( handler_ ) {
    lockCountDown();
    bool ret = false;
    {
      HandlerPointer::Locked l = handler_;
      if ( l ) {
        DEBUGLOG( "giving message to handler" );
        l->handleMessage( msg );
      } else {
        out() << "could not lock handler";
      }
    }
    lockCountUp();
    return ret;
  } else {
    out() << "no handler";
  }
  return false;
}

void BasicTCPSession::removeAllMessages() {
  if ( sendingMessage_ ) {
    MessagePointer::Locked l = sendingMessage_;
    if ( l ) {
      lockCountDown();
      l->result( false );
      lockCountUp();
    }
    sendingMessage_ = 0;
  }

  SessionReplyManager::removeAllMessages();

  while ( messagesToSend_ ) {
    MessagePointer::Locked l = messagesToSend_.front();
    if ( l ) {
      l->result( false );
    } else {
      err() << "could not lock messages while removing it from send-list";
    }
    messagesToSend_.pop_front();
  }
}

bool BasicTCPSession::send( MessageInterface* msg ) {
  msg->info().setSession( this );
  if ( !exit_ && !failed_ ) {
    messagesToSend_ << msg;
  } else {
    msg->result( false );
    return false;
  }
  return true;
}

void BasicTCPSession::stopRunning() {
  if ( !exit_ )
    out( Logger::Debug ) << "stop running";
  exit_ = true;
}

///This should get some tolerance in future
void BasicTCPSession::stopRunningNow() {
  stopRunning();
  if ( !Thread::isThread() && Thread::isRunning() )
    Thread::join();
}

///Returns whether the thread is running
bool BasicTCPSession::isRunning() {
  return running_ && Thread::isRunning();
}

string BasicTCPSession::peerDesc() {
  ostringstream str;
  ost::tpport_t port;

  str << getPeer( &port ) << ":" << port;

  return str.str();
}

string BasicTCPSession::localDesc() {
  ostringstream str;
  ost::tpport_t port;

  str << getLocal( &port ) << ":" << port;

  return str.str();
}

SessionInterface::SessionDirection BasicTCPSession::sessionDirection() {
  return myDirection;
}

}

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on

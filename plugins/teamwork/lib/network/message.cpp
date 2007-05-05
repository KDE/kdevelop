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
#include "messagetypeset.h"
#include "basicsession.h"
#include "user.h"

namespace Teamwork {

MessageInfo::MessageInfo() : isReplyTo_( 0 ), uniqueId_( 0 ), deserialized_( false ) {
}

MessageInfo::MessageInfo( const MessageType& id, UniqueMessageId uniqueId ) : isReplyTo_( 0 ), uniqueId_( uniqueId ), deserialized_( false ) {
  id_ = id;
  //  cout << "creating message-info normally";
}

MessageInfo::MessageInfo( const MessageType& id, UniqueMessageId uniqueId, const SessionPointer& session, UniqueMessageId isReplyTo ) : isReplyTo_( isReplyTo ), uniqueId_( uniqueId ), deserialized_( false ) {
  id_ = id;
  session_ = session;
  //  cout << "creating message-info normally";
}

MessageInfo::~MessageInfo() {
}

void MessageInfo::setReplyMessage( const MessagePointer& msg ) {
  replyToMessage_ = msg;
}

const MessagePointer& MessageInfo::replyToMessage() const {
  return replyToMessage_;
}

SessionPointer MessageInfo::session() const {
  if ( !session_ ) {
    UserPointer::Locked l = user_;
    if ( l ) {
      return l->online().session();
    } else {
      return 0;
    }
  } else {
    return session_;
  }
}

UserPointer MessageInfo::user() const {
  if ( session_ ) {
    return session_.unsafe() ->safeUser();
  } else {
    return user_;
  }
}

void MessageInfo::setSession( const SessionPointer& sess ) {
  if ( !sess ) {
    session_ = 0;
    user_ = 0;
    return ;
  }
  if ( UserPointer p = sess.unsafe() ->safeUser() ) {
    setUser( p );
  } else {
    session_ = sess;
    user_ = 0;
  }
}

void MessageInfo::setUser( const UserPointer& user ) {
  user_ = user;
  session_ = 0;
}

MessageFactoryInterface* MessageTypeSet::findFactory( MessageType& id ) const {
  while ( id ) { ///walk up the tree until a matching maybe inherited message-type is found
    TypeMap::const_iterator it = types_.find( id );
    if ( it != types_.end() ) {
      return ( *it ).second;
    }
    --id;
  }
  return 0;
}

MessagePointer MessageTypeSet::buildMessage( InArchive& from, const MessageInfo& inf ) const {
  MessageType id = inf.type();

  MessageFactoryInterface* i = findFactory( id );
  if ( i ) {
    MessageInfo info( inf );
    info.setId( id );
    return MessagePointer( i->buildMessage( from, info ) );
  }

  cout << "could not build message with id " << id.desc() << endl;
  return MessagePointer();
}

void MessageType::packFastId() {
  //return;
  fastId_ = 0;
  if ( idList_.size() > 4 ) {
    useFastId = false;
  } else {
    useFastId = true;
    int shift = 24;
    for ( IdList::iterator it = idList_.begin(); it != idList_.end(); ++it ) {
      fastId_ += *it << shift;
      shift -= 8;
    }
  }
}

MessageType::MessageType( IdList IDs ) : idList_( IDs ), useFastId( false ) {
  packFastId();
}

MessageType::MessageType( InArchive& from ) : useFastId( false ) {
  serialize( from );
}

bool MessageType::operator == ( const MessageType& rhs ) const {
  if ( useFastId && rhs.useFastId )
    return fastId_ == rhs.fastId_;
  int s1 = idList_.size();
  int s2 = rhs.idList_.size();
  if ( s1 != s2 )
    return false;
  for ( int a = 0; a < s1; a++ ) {
    if ( idList_[ a ] != rhs.idList_[ a ] )
      return false;
  }

  return true;
}

bool MessageType::operator < ( const MessageType& rhs ) const {
  if ( useFastId && rhs.useFastId )
    return fastId_ < rhs.fastId_;
  int s1 = idList_.size();
  int s2 = rhs.idList_.size();
  int ms = s2 > s1 ? s1 : s2;
  for ( int a = 0; a < ms; a++ ) {
    if ( idList_[ a ] < rhs.idList_[ a ] ) {
      return true;
    }
    if ( idList_[ a ] > rhs.idList_[ a ] ) {
      return false;
    }
  }
  if ( s1 < s2 ) {
    return true;
  }
  if ( s1 > s2 ) {
    return false;
  }

  return false;
}

int MessageType::length() const {
  return idList_.size();
}


bool MessageType::startsWith( const MessageType& rhs ) const {
  int s1 = idList_.size();
  int s2 = rhs.idList_.size();
  if ( s1 < s2 )
    return false;

  for ( int a = 0; a < s1; a++ ) {
    if ( idList_[ a ] != rhs.idList_[ a ] )
      return false;
  }

  return true;
}

MessageType& MessageType::operator += ( unsigned char append ) {
  if ( !idList_.empty() )
    idList_.pop_back();
  idList_.push_back( append );
  idList_.push_back( 0 );
  packFastId();
  return *this;
}

MessageType& MessageType::operator -- () {
  if ( !idList_.empty() ) {
    idList_.pop_back();
    idList_.pop_back();
    idList_.push_back( 0 );
  }
  packFastId();
  return *this;
}

MessageType::operator bool() const {
  return !idList_.empty();
}

std::string MessageType::desc() const {
  if ( idList_.empty() )
    return "'invalid id'";
  std::ostringstream ret;
  IdList::const_iterator end = idList_.end();
  if ( end != idList_.begin() )
    --end;
  for ( IdList::const_iterator it = idList_.begin(); it != end; ++it )
    ret << ( int ) * it << "-";
  return ret.str();
}

MessageType::operator const unsigned char*() const {
  if ( idList_.empty() ) {
    return ( const uchar* ) "";
  } else {
    return ( const uchar* ) & ( idList_[ 0 ] );
  }
}

UniqueMessageId MessageInfo::uniqueId() const {
  return uniqueId_;
}

/**Since the uniqueId is not used for sorting this casts away constness so the id of a MessageType that is used as Key in a Map can be changed */
void MessageInfo::setUniqueId( UniqueMessageId newId ) const {
  const_cast<MessageInfo*>( this ) ->uniqueId_ = newId;
}

MessageTypeSet::TypeMap::iterator MessageTypeSet::search( const MessageType& id ) {
  return types_.find( id );
}

MessageType MessageTypeSet::allocateSubId( const MessageType& id, int preferredSubId  ) {
  MessageType tempId = id;
  if ( preferredSubId == 0 )
    preferredSubId = 1;
  for ( int a = preferredSubId; a < 255; a++ ) {
    tempId += a;
    if ( types_.find( tempId ) == types_.end() ) {
      return tempId;
    }
    --tempId;
  }

  cout << "problem while allocating sub-id for " << id.desc() << ", all sub-id's seem to be taken" << endl;
  ///This should not happen, but anyway try to allocate some id
  if ( tempId ) {
    return allocateSubId( allocateSubId( --tempId ), preferredSubId );
  }

  return MessageType();
}
MessageTypeSet::MessageTypeSet() {
  srand( time( NULL ) );
  currentUniqueMessageId_ = rand();
  currentUniqueMessageId_ *= rand();
  currentUniqueMessageId_ *= rand();
  currentUniqueMessageId_ *= rand();
}

MessagePointer MessageTypeSet::buildMessage( InArchive& from, const MessageInfo& inf ) const;

MessageTypeSet::~MessageTypeSet() {
  for ( TypeMap::iterator it = types_.begin(); it != types_.end(); ++it ) {
    delete ( *it ).second;
  }
  types_.clear();
}

const MessageType& MessageTypeSet::idFromName( const std::string& name ) const {
  TypeNameMap::const_iterator it = ids_.find( name );
  if ( it != ids_.end() ) {
    return ( *it ).second;
  } else if( name == "MessageInterface" ) {
    return const_cast<TypeNameMap&>(ids_)[name]; //Automatically register MessageInterface with the zero-id
  } else {
    cout << "could not assign an ID to a message called \"" << name << "\", it seems not to be registered in the message-type-set" << endl;
    return const_cast<TypeNameMap&>(ids_)[ name ];
  }
}

std::string MessageTypeSet::stats() const {
  ostringstream ret;
  ret << "count of  message-types: " << types_.size() << endl;
  for ( TypeNameMap::const_iterator it = ids_.begin(); it != ids_.end(); ++it ) {
    ret << "type: " << ( *it ).first << " id: " << ( *it ).second.desc() << endl;
  }
  return ret.str();
}

///returns the class-name of the message(the most specialized one registered in this type-set)
std::string MessageTypeSet::identify( MessageInterface* msg ) const {
  MessageType id = msg->info().type();
  MessageFactoryInterface* i = findFactory( id );
  if ( i ) {
    return i->identify();
  } else {
    return "could not identify";
  }
}

}
// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on

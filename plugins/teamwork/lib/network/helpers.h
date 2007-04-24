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

#ifndef TEAMWORK_HELPERS
#define TEAMWORK_HELPERS
#include "common.h"
#include "pointer.h"
#include <list>
#include <set>
#include <map>

//for ost::Mutex
#include <cc++/thread.h>

namespace Teamwork {
  template <class Type>
  class SafeList {
    class MutexLocker {
      ost::Mutex& m_;
      MutexLocker& operator=( const MutexLocker& rhs ) {
        UNUSED( rhs );
        return *this;
      };
      
    public:
      MutexLocker( ost::Mutex& mutex ) : m_( mutex ) {
        m_.enterMutex();
      }

      MutexLocker( MutexLocker& rhs ) : m_( rhs.m_ ) {
        m_.enterMutex();
      };

      ~MutexLocker() {
        m_.leaveMutex();
      }
    };
    std::list<Type> list_;
    mutable ost::Mutex m_;
  public:
    SafeList() {
    };
    
    void push_back( const Type& t ) {
      MutexLocker l( m_ );
      list_.push_back( t );
    }
    
    void push_front( const Type& t ) {
      MutexLocker l( m_ );
      list_.push_front( t );
    }
    
    void pop_back() {
      MutexLocker l( m_ );
      list_.pop_back();
    }
    
    void pop_front() {
      MutexLocker l( m_ );
      list_.pop_front();
    }
    
    Type& front() {
      MutexLocker l( m_ );
      return list_.front();
    }
    
    Type& back() {
      MutexLocker l( m_ );
      return list_.back();
    }
    
    const Type& front() const {
      MutexLocker l( m_ );
      return list_.front();
    }
    
    const Type& back() const {
      MutexLocker l( m_ );
      return list_.back();
    }
    
    SafeList& operator << ( const Type& item ) {
      MutexLocker l( m_ );
      push_back( item );
      return *this;
    }
    
    bool empty() const {
      MutexLocker l( m_ );
      return list_.empty();
    }
    
    void clear() {
      MutexLocker l( m_ );
      list_.clear();
    }
    
    bool empty() {
      MutexLocker l( m_ );
      return list_.empty();
    }
    
    operator bool() const {
      return !empty();
    }
    
    int size() const {
      MutexLocker l( m_ );
      return list_.size();
    }
  };
};
//must not be included earlier because of include-recursion
#include "message.h"

namespace Teamwork {

  class BasicTCPSession;

    template<class Archive>
  DispatchableMessage buildMessageFromArchive( Archive& arch, MessageTypeSet& messages, SessionPointer sess  ) {
    MessageInfo inf( arch );
    inf.setSession( sess );
    
    return messages.buildMessage( arch, inf );
  }
  
  ///May throw several exceptions
  template<class ArchType>
  void serializeMessageToArchive( ArchType& arch, MessageInterface& message ) {
    message.info().serialize( arch );
    
    message.serialize( arch ); ///it must be called like this, because the serialization would add type-information we don't want
  }

  ///May throw several exceptions
  void serializeMessageToBuffer( std::vector<char>& buf, MessageInterface& message );
  DispatchableMessage buildMessageFromBuffer( const std::vector<char>& buf, MessageTypeSet& messages, SessionPointer sess = 0  );
}

template <class VecType, class Vec2Type>
std::vector<VecType>& operator += ( std::vector<VecType>& lhs, const std::vector<Vec2Type>& rhs ) {
  uint osize = lhs.size();
  lhs.resize( lhs.size() + ( (rhs.size() * sizeof( Vec2Type )) / sizeof(VecType))  );
  memcpy( &lhs[osize], &rhs[0], sizeof(Vec2Type) * rhs.size() );
  return lhs;
}

std::string formatInt( int i );

std::vector<char> binaryInt( int num );

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on

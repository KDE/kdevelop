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
#ifndef SAFELIST_H
#define SAFELIST_H

#include <list>
#include <cc++/thread.h>
#include "safesharedptr.h"

namespace Teamwork {
  template <class Type>
  class SafeList {
    class MutexLocker {
      ost::Mutex& m_;
      MutexLocker& operator=( const MutexLocker& /*rhs*/ ) {
        return *this;
      };

    public:
      MutexLocker( ost::Mutex& mutex ) : m_( mutex ) {
        m_.enterMutex();
      }

      MutexLocker( MutexLocker& rhs ) : m_( rhs.m_ ) {
        m_.enterMutex();
      }

      ~MutexLocker() {
        m_.leaveMutex();
      }
    };
    std::list<Type> list_;
    mutable ost::Mutex m_;
  public:
    SafeList() {
    }

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
}

#endif

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
#ifndef MUTEXINTERFACE_H
#define MUTEXINTERFACE_H

#include <QMutex>
#include "networkexport.h"

class NETWORK_EXPORT MutexInterfaceImpl {
  mutable QMutex m_;
  public:
    MutexInterfaceImpl( const MutexInterfaceImpl& /*rhs*/ ) : m_(QMutex::Recursive) {
    }

    const MutexInterfaceImpl& operator=( const MutexInterfaceImpl& /*rhs*/ ) {
      return *this;
    }

    MutexInterfaceImpl() {}

    void lockCountUp() const {
      m_.lock();
    }

    bool tryLockCountUp() const {
      return m_.tryLock();
    }

    ///timeout in nanoseconds, may have a significant delay, so it shouldn't be used too much
    bool tryLockCountUp( int timeout ) const {
      if(timeout) {
	timeout /= 1000000; //nanoseconds -> milliseconds
	if(timeout)
	  timeout = 1;
      }
      return m_.tryLock( timeout );
    }

    void lockCountDown() const {
      m_.lock();
    };

    ~MutexInterfaceImpl() {}
};

typedef MutexInterfaceImpl MutexInterface;
#endif

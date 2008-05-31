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

#ifndef HANDLER_H
#define HANDLER_H

#include "networkfwd.h"
#include "safesharedptr.h"
#include "networkexport.h"

namespace Teamwork {
class NETWORK_EXPORT HandlerInterface : public SafeShared {
  public:
    virtual ~HandlerInterface() {}
    /**When the handler is multithreaded, this function must be thread-safe by just queueing the Message to a secure list and processing later.
    No locking must be done within this function, since it may cause deadlocks as the handler itself tries to lock the message-sender */
    virtual bool handleMessage( MessagePointer msg ) = 0;
};

template <class Target>
struct NETWORK_EXPORT HandlerProxy : public HandlerInterface {
  WeakSafeSharedPtr<Target> handler_;
  HandlerProxy( const SafeSharedPtr<Target>& targ ) : handler_( targ ) {}

  virtual bool handleMessage( MessagePointer msg ) {
    SafeSharedPtr<Target> targ = handler_.get();
    if ( targ )
      return targ.unsafe() ->handleMessage( msg );
    else
      return false;
  }
};
}

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on


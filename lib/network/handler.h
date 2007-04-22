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

#ifndef HANDLER_H
#define HANDLER_H

namespace Teamwork {
class HandlerInterface : public SafeShared {
  public:
    virtual ~HandlerInterface() {}
    /**When the handler is multithreaded, this function must be thread-safe by just queueing the Message to a secure list and processing later.
    No locking must be done within this function, since it may cause deadlocks as the handler itself tries to lock the message-sender */
    virtual bool handleMessage( DispatchableMessage msg ) = 0;
};

template <class Target>
struct HandlerProxy : public HandlerInterface {
  WeakSafeSharedPtr<Target> handler_;
  HandlerProxy( const SafeSharedPtr<Target>& targ ) : handler_( targ ) {}

  virtual bool handleMessage( DispatchableMessage msg ) {
    SafeSharedPtr<Target> targ = handler_.get();
    if ( targ )
      return targ.getUnsafeData() ->handleMessage( msg );
    else
      return false;
  }
};
}

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on


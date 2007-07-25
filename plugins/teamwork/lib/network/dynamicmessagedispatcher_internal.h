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

/** This file contains the implementation-details of DynamicMessageDeliverer
 * */

#ifndef DYNAMICMESSAGEDISPATCHER_INTERNAL_H
#define DYNAMICMESSAGEDISPATCHER_INTERNAL_H

#include "weaksafesharedptr.h"
#include "networkfwd.h"
#include "messageinterface.h"

namespace Teamwork {

  ///Some helper-classes for implementing the dispatcher, see below
class MessageDelivererBase {
  public:
  virtual int tryDeliverMessage( const MessagePointer& msg ) = 0;
  virtual ~MessageDelivererBase() {
  }
  virtual bool isOk() const = 0;
  virtual const MessageType& type() const = 0;
};

template<class Message, class Target>
  class MessageDeliverer : public MessageDelivererBase {
    public:
      typedef int (Target::*FunctionType)(const SafeSharedPtr<Message>& );
      
      MessageDeliverer( const WeakSafeSharedPtr<Target>& target, FunctionType function,  MessageType type, bool lockBeforeCall = true ) : m_function(function), m_target(target), m_lockBeforeCall( lockBeforeCall ), m_type(type) {
      }

      virtual bool isOk() const {
        return m_target.get();
      }

      virtual const MessageType& type() const {
        return m_type;
      }

    virtual int tryDeliverMessage( const MessagePointer& msg ) {
      SafeSharedPtr<Message> myMsg = msg.cast<Message>();
      if( myMsg ) {
        if( m_lockBeforeCall ) {
          //The message can be casted to the needed type, so it should be delivered.
          LockedSharedPtr<Target> lockedTarget = m_target;
          if( lockedTarget ) {
            //Deliver the message
            return (lockedTarget->*m_function) ( myMsg );
          } else {
            //Problem
          }
        } else {
          //Do not lock the target before calling the callback-function
          SafeSharedPtr<Target> ref = m_target.get(); //Make a normal safe-shared-pointer out of the weak one, so the target can not be deleted while the call
          if( ref )
            return (ref.unsafe()->*m_function) ( myMsg );
        }
      }
      return 0;
    };
    virtual ~MessageDeliverer() {
    }
    private:
      FunctionType m_function;
      WeakSafeSharedPtr<Target> m_target;
      bool m_lockBeforeCall;
      MessageType m_type;
  };
}

#endif

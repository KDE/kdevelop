/***************************************************************************
copyright            : (C) 2007 by David Nolden
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

///WARNING: This is not tested yet

/** This class works similar to MessageDispatcher, except that it is not static.
 *  Callback-functions can be registered at runtime, and
 *  there is less dynamically generated code.
 * 
 * The target-classes must be based on WeakSafeShared.
 * That way noone has to care about unregistering targets, because DynamicMessageDispatcher will disconnect
 * destroyed targets automatically..
 *
 * Also the target must be kept alive by a SafeSharedPtr.
 * (else they would get destructed as soon as the reference-count gets increased and decreased once)
 * 
 *
 * The downside is that it's slower, the callback-functions need to be registered on runtime,
 * it's probably more error-prone, and what's happening is not absolutely clear on compile-time.
 *
 * It should be preferred over MessageDispatcher either when the message-delivery should be more
 * flexible, or when several(maybe unknown) targets should be allowed to register callback-functions on their own, like plugins.
 *
 * This class is not suitable for handling many(above 50) dispatch-target.
 *
 * The order of message-delivery:
 *   When receiveMessage(...) is called, the order of message-delivery is defined this way:
 *    1. Deliver first to the best specialized targets
 *    2. Among targets that match equally well, the last registered targets will get the message first.
 *
 *   Once any delivery-function returns a nonzero return-value, the message-deliverey will be stopped and the
 *   return-value returned by the receiveMessage(...) -function.
**/

#ifndef DYNAMICMESSAGEDISPATCHER_H
#define DYNAMICMESSAGEDISPATCHER_H

#include <vector>
#include "dynamicmessagedispatcher_internal.h"
#include "messagetypeset.h" //needed so the message-type-description can be extracted while registering

namespace Teamwork {

  ///The message-dispatcher:
class DynamicMessageDispatcher {
  public:
    DynamicMessageDispatcher( MessageTypeSet& messageTypes = globalMessageTypeSet() );
    ~DynamicMessageDispatcher();

    /**
     * Use this to register your class for receiving all messages based on a given one.
     *
     * When a callback is registered using this function, the target will be locked before the target-function is called.
     * 
     * @param Message(template-param 1) The message-type your class should receive
     * @param target a weak-pointer to the target the message should be delivered to(the weak pointer makes it safe to register and later delete the object without notification)
     * @param function pointer to a member-function of the taget-class, that must have the signature "int function( const SafeSharedPtr<Message>& )"
     *
     * For an example see main.cpp, there's a version of StandaloneServer that uses this.
     * */
    template<class Message, class TargetClass>
    void registerCallback( TargetClass* target, int (TargetClass::*function) (const SafeSharedPtr<Message>&) ) {
      addDeliverer( new MessageDeliverer<Message, TargetClass> ( target, function, m_messageTypes.type<Message>(), true ) );
    }

    /**
     * Same as above, except that the target will not be locked before the callback is called
     * */
    template<class Message, class TargetClass, class Serialization>
    void registerUnsafeCallback( TargetClass* target, typename MessageDeliverer<Message, TargetClass>::FunctionType function ) {
      addDeliverer( new MessageDeliverer<Message, TargetClass> ( target, function, m_messageTypes.type<Message>(), false ) );
    }

    /** Call this to dispatch the message to the registered targets
     * @return 0 if there is no fitting target, or the value returned by the target's dispatch-function
    */
    int operator() ( const MessagePointer& msg );

  protected:
    /** Call this to dispatch the message to the registered targets. It is the same as operator(), but looks cleaner when called from an inherited class.
     * @return 0 if there is no fitting target, or the value returned by the target's dispatch-function
    */
    int dispatch( const MessagePointer& msg );
    
    void addDeliverer( MessageDelivererBase* );

  private:
    typedef std::vector<MessageDelivererBase*> DelivererList;
    DelivererList m_deliverers;
    MessageTypeSet& m_messageTypes;
};
};

#endif

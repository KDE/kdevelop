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

#ifndef MESSAGE_H
#define MESSAGE_H

#include "networkfwd.h"
#include <map>
#include <string>
#include <iostream>
#include "messagetypesetinternal.h"

#include "messageinterface.h"

#define MAXMESSAGESIZE 50000000
#define DISABLEWARNINGS

#define USE_RTTI

template <class Message>
const char* staticMessageName() {
  return Message::staticName();
}

namespace Teamwork {

using namespace Tree;

/**This allows multiple differing dispatch-targets(like the server + the thread). */
class MessageTypeSet {
    //protected:
    ///this should be optimized, and all the no more necessary template dispatch-code above removed
    typedef std::map<MessageType, MessageFactoryInterface* > TypeMap;
    typedef std::map<std::string, MessageType> TypeNameMap;
    TypeMap types_; ///would be more efficient to build a tree from the Chains and walk through that
    TypeNameMap ids_;

    ///this should walk through the messages upwards until it finds the best match
    TypeMap::iterator search( const MessageType& id );

    MessageType allocateSubId( const MessageType& id, int preferredSubId = 0 );

    mutable UniqueMessageId currentUniqueMessageId_;

    ///Changes the given id to the matching one
    MessageFactoryInterface* findFactory( MessageType& id ) const;
  public:
    MessageTypeSet();

    MessagePointer buildMessage( InArchive& from, const MessageInfo& inf ) const;

    ~MessageTypeSet();

    //Returns the message-type mapped to a message-name. A message-name in rtti (typeid(Message).name()) format is also accepted for messages derived from MessageInterface
    const MessageType& idFromName( const std::string& name ) const;

    std::string stats() const;

    ///returns the class-name of the message(the most specialized one registered in this type-set)
    std::string identify( MessageInterface* msg ) const;

    ///Register a static list of message-types to this type-set. Not yet registered parent-messages are automatically registered too.
    template < class Types >
    void registerMessageTypes() {
      RegisterAllTypes< typename Tree::MakeComplete< typename Tree::ExtractChains< Types >::Result, MessageInterface > ::Result, MessageTypeSet >::reg ( *this );
    }

    /** registers a type as slave of another given type, for simlicity registerMessageTypes should be used instead, together with a whole list of types */
    template <class Message, class ParentMessage>
    MessageType registerMessageType( int preferredSubId = 0 ) {
      if ( ids_.find( staticMessageName<Message>() ) != ids_.end() )
        return ids_[ staticMessageName<Message>() ];

      MessageType parentId = type<ParentMessage>();
      MessageType myId = allocateSubId( parentId, preferredSubId );
      if ( !parentId && typeid( ParentMessage ) != typeid( MessageInterface ) )
      std::cout << "while registering \"" << staticMessageName<Message>() << "\" the id of \"" << staticMessageName<ParentMessage>() << "\" could not be allocated!" << endl;
      if ( myId ) {
        //std::cout << "registering message-type " << staticMessageName<Message>() << " with id " << myId.desc() << endl;
        ids_[ staticMessageName<Message>() ] = myId;
	ids_[ typeid(Message).name() ] = myId;
        if ( types_.find( myId ) != types_.end() )
          delete types_[ myId ];
        types_[ myId ] = new MessageFactory<Message>();
      } else {
        std::cout << "could not allocate id for message-type " << staticMessageName<Message>() << endl;
        ///No ID could be allocated
      }
      return myId;
    }

    template <class Type>
    inline const MessageType& type() {
      return idFromName( staticMessageName<Type>() );
    }

//     MessageInterface* changeUniqueId( MessageInterface* msg ) {
//       msg->info().setUniqueId( currentUniqueMessageId_++ );
//       return msg;
//     }

    /** Returns a new correctly filled MessageInfo-structure, that also contains a vald UniqueId.
     * */
    template<class Message>
    MessageInfo messageInfo() const {
      return MessageInfo( idFromName( staticMessageName<MessageType>() ), currentUniqueMessageId_++ );
    }
    
    /**Returns a new correctly filled MessageInfo-structure, that also contains a vald UniqueId,
     * by the name as which the message is registered, or by the message's rtti-name.
     * */
    MessageInfo messageInfo(const char* name) const {
      return MessageInfo( idFromName( name ), currentUniqueMessageId_++ );
    }
    /*
    //For now the following is disabled to test whether it can be conveniently replaced with something that isn't that bad for executable-size
    ///Conveniently create a new message with the correct Message-Id and unique-Id filled. All parameters are passed to the constructor of the created message.
    template <class MessageType>
    inline MessageType* create() {
      return (MessageType*) changeUniqueId( new MessageType( idFromName( staticMessageName<MessageType>() )  ) );
    }
    ///Conveniently create a new message with the correct Message-Id and unique-Id filled. All parameters are passed to the constructor of the created message.
    template <class MessageType, class ParamType1>
    inline MessageType* create( ParamType1 param ) {
      return (MessageType*) changeUniqueId( new MessageType( idFromName( staticMessageName<MessageType>() ),  param ) );
    }
    ///Conveniently create a new message with the correct Message-Id and unique-Id filled. All parameters are passed to the constructor of the created message.
    template <class MessageType, class ParamType1, class ParamType2>
    inline MessageType* create( ParamType1 param, ParamType2 param2 ) {
      return (MessageType*)changeUniqueId( new MessageType( idFromName( staticMessageName<MessageType>() ), param, param2 ) );
    }
    ///Conveniently create a new message with the correct Message-Id and unique-Id filled. All parameters are passed to the constructor of the created message.
    template <class MessageType, class ParamType1, class ParamType2, class ParamType3>
    inline MessageType* create( ParamType1 param, ParamType2 param2, ParamType3 param3 ) {
      return (MessageType*)changeUniqueId( new MessageType( idFromName( staticMessageName<MessageType>() ), param, param2, param3 ) );
    }
    ///Conveniently create a new message with the correct Message-Id and unique-Id filled. All parameters are passed to the constructor of the created message.
    template <class MessageType, class ParamType1, class ParamType2, class ParamType3, class ParamType4>
    inline MessageType* create( ParamType1 param, ParamType2 param2, ParamType3 param3, ParamType4 param4 ) {
      return (MessageType*)changeUniqueId( new MessageType( idFromName( staticMessageName<MessageType>() ), param, param2, param3, param4 ) );
    }
    ///Conveniently create a new message with the correct Message-Id and unique-Id filled. All parameters are passed to the constructor of the created message.
    template <class MessageType, class ParamType1, class ParamType2, class ParamType3, class ParamType4, class ParamType5>
    inline MessageType* create( ParamType1 param, ParamType2 param2, ParamType3 param3, ParamType4 param4, ParamType5 param5 ) {
      return (MessageType*)changeUniqueId( new MessageType( idFromName( staticMessageName<MessageType>() ), param, param2, param3, param4, param5 ) );
    }
    ///Conveniently create a new message with the correct Message-Id and unique-Id filled. All parameters are passed to the constructor of the created message.
    template <class MessageType, class ParamType1, class ParamType2, class ParamType3, class ParamType4, class ParamType5, class ParamType6>
    inline MessageType* create( ParamType1 param, ParamType2 param2, ParamType3 param3, ParamType4 param4, ParamType5 param5, ParamType6 param6 ) {
      return (MessageType*)setUniqueId( new MessageType( idFromName( staticMessageName<MessageType>() ), param, param2, param3, param4, param5, param6 ) );
    }
    */
};

/**This is a message-type-set dedicated only to dispatching a chosen set of message-types
to a chosen dispatch-target. It delivers the given message as the most special type the message
is based on, by calling "TargetType::receiveMessage( MessageType* )".
 * @param TargetType must be the type the messages should be delivered to
 * @param Messages must be the list of messages(without any inheritance-info)*/
template <class TargetType, class Messages>
class MessageDispatcher {
    TargetType& targ_;

    struct EntryTemplate {
      template <class MessageType>
      struct Entry {
        int dispatch( TargetType& targ, MessageInterface* msg ) const {
          //std::cout << "dispatching as " << staticMessageName<MessageType>() << endl;
          ///Do the dispatch from the MessageDispatcher, so it can be declared friend and the dispatch-functions can be private.
          return MessageDispatcher<TargetType, Messages>::template dispatch<MessageType>
          ( targ, msg );
        }
      };
    };

    struct Action {
      TargetType& targ;
      MessageInterface* msg;
      int ret;

      Action( TargetType& target, MessageInterface* mesg ) : targ( target ), msg( mesg ), ret( 0 ) {}
      template <class Type, class Node>
      void operator () ( const Node& node ) {
        ret = node.dispatch( targ, msg );
      }
    };

    typedef Tree::TreeMasterNode< MessageInterface, Messages, EntryTemplate > Nodes;
    Nodes nodes_;
  public:
    MessageDispatcher( TargetType& targ ) : targ_( targ ) {}

    ///dispatches the message to the target that was given while construction
    int operator () ( MessageInterface* msg ) {
      Action act( targ_, msg );
      nodes_.find( msg->type(), act );
      return act.ret;
    }

    template <class Out>
    void print( Out& out ) {
#ifdef USE_RTTI
      out << "Full Tree of " << typeid( *this ).name() << "\n";
      nodes_.print( out );
#else
      out << "cannot print Message-Tree because RTTI is disabled" << "\n";
#endif
    }

    template < class MessageType >
    static int dispatch( TargetType& targ, MessageInterface* msg ) {
      if ( dynamic_cast<MessageType*>( msg ) )      //this is just a security-check, which should hopefully never trigger
        return targ.receiveMessage( ( MessageType* ) msg );
      else
        cerr << "serious error in the message-type-system!";
      return 0;
    }
};


///This message-type-set contains all messages that were registered by a call to REGISTER_MESSAGE, and may be used instead of the local type-sets, for simplicity. That way messages can be serialized/deserialized without the core-parts being aware that they exist.
MessageTypeSet& globalMessageTypeSet();

template <class MessageType>
class RegisterMessageTypeInternal {
  public:
    RegisterMessageTypeInternal() {
      globalMessageTypeSet().registerMessageTypes<MessageType>();
    }
};

/**This should be used directly from outside to globally register new message-types.
The messages should have used the DECLARE_MESSAGE(...)-macro.
*/
#define REGISTER_MESSAGE(TYPE)  \
struct registerType##TYPE {     \
  RegisterMessageTypeInternal<TYPE> msgReg;\
  registerType##TYPE() : msgReg() {} \
};                                \
registerType##TYPE register##TYPE; \

}

#ifndef USE_RTTI

BOOST_CLASS_IMPLEMENTATION( Teamwork::MessageInfo, boost::serialization::object_serializable );

#endif

#endif

#include "user.h"

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on

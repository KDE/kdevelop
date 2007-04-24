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

#ifndef MESSAGE_H
#define MESSAGE_H

#include "common.h"
#include <sstream>
#include "pointer.h"
#include "weakpointer.h"
#include "interfaces.h"
#include <map>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include "statictree.h"
#include "messageinternal.h"
#include <boost/cstdint.hpp>

#define MAXMESSAGESIZE 50000000
#define DISABLEWARNINGS

#define USE_RTTI

struct MessageSerialization;

template <class Message>
const char* staticMessageName() {
  return Message::staticName();
}

namespace Teamwork {
class MessageInterface;
class User;
class BasicTCPSession;
class DispatchableMessage;

typedef SafeSharedPtr<MessageInterface, MessageSerialization> MessagePointer;
typedef SafeSharedPtr<User, BoostSerialization> UserPointer;

///UniqueMessageId represents the identity of a single message
typedef uint64_t UniqueMessageId;

using namespace Tree;

///MessageId represents the Message-Type
class MessageId {
  public:
    typedef std::vector<unsigned char> IdList;
  private:
    IdList idList_;
    unsigned int fastId_; ///can only be used when the idList has <= 4 entries
    bool useFastId;
    UniqueMessageId uniqueId_;  ///This is only filled when the MessageId belongs to a Message(0 means invalid)
    void packFastId();
  public:
    MessageId( IdList IDs = IdList(), uint uniqueId = 0 );

    MessageId( InArchive& from );

    template <class ArchType>
    void serialize( ArchType& target, const uint version = 0 ) {
      target & idList_;
      target & uniqueId_;
      packFastId();
      UNUSED( version );
    }

    ///Only compares the type-id, not the uniqueId
    bool operator == ( const MessageId& rhs ) const;

    bool operator < ( const MessageId& rhs ) const;

    bool startsWith( const MessageId& rhs ) const;

    MessageId& operator += ( unsigned char append );

    MessageId& operator -- ();

    operator bool() const;

    std::string desc() const;

    operator const uchar*() const;

    UniqueMessageId uniqueId() const;

    /**Since the uniqueId is not used for sorting this casts away constness so the id of a MessageId that is used as Key in a Map can be changed */
    const MessageId& modifyUniqueId( UniqueMessageId newId ) const;
};

///this class holds information about the instance of a message(it's id and an optional pointer to the session/user it was created in/for)
class MessageInfo {
    MessageId id_;
    SessionPointer session_;
    UserPointer user_;
    MessagePointer replyToMessage_;
    UniqueMessageId isReplyTo_;
    bool deserialized_;
  public:

    template < class ArchType >
    MessageInfo( ArchType& arch ) : id_( arch ), deserialized_( true ) {
      cout << "deserializing message-info";
      arch & isReplyTo_;
    }

    MessageInfo( const MessageId& id, const SessionPointer& session = SessionPointer(), UniqueMessageId isReplyTo = 0 );

    ///The session this message was received through/has been sent through.(whether received or sent: see isIncoming() )
    SessionPointer session() const;

    ///The user this message is related to.
    UserPointer user() const;

    ///Sets the session, and invalidates the user. If the session has an associated user, the user is used instead.
    void setSession( const SessionPointer& sess );

    ///Sets the user and invalidates the session. All session()-calls will be redirected to the session associated to the user.
    void setUser( const UserPointer& sess );

    const MessageId& id() const {
      return id_;
    }

    template < class Archive >
    void serialize( Archive& arch, const uint version = 0 ) {
      id_.serialize( arch, version );
      arch & isReplyTo_;
    }

    /** if the message is a reply to another message, this should be the uniqueId of the other message.
      this function is thread-safe.(because after creation, this value should not be changed anymore) */
    UniqueMessageId isReplyTo() const {
      return isReplyTo_;
    }

    /** to make one message a reply to another while sending, this is enough. To make the fast locally usable, setReplyMessage(...) should be set too.*/
    void setReply( UniqueMessageId replyId ) {
      isReplyTo_ = replyId;
    }

    /** this does not automatically set the reply-id, it just sets a message-pointer. */
    void setReplyMessage( const MessagePointer& msg );

    ///If the reply is not handled directly by the message that the reply has gone to, the message this is a reply to is stored here.(The message with the id isReplyTo() ), may be invalid
    const MessagePointer& replyToMessage() const;

    void setId( const MessageId& id ) {
      id_ = id;
    }

    /**Removes all kept references to important objects like sessions(those might keep the sessions alive too long)
      this should be called before a message is stored permanently */

    void makePersistent() {
      setSession( 0 );
    }

    ///Returns whether the message was constructed from a stream(if false, it was created locally)
    bool isIncoming() const {
      return deserialized_;
    }

    ///This can be used to set the isIncoming()-flag. The application or messages may use the flag as indication whether the message is incoming or outgoing.
    void setIsIncoming( bool b ) {
      deserialized_ = b;
    }
};

/**Since messages can have different IDs in different Message-Sets, the id's cannot be hardcoded and must be given with each creation.
That is simplified by using MessageTypeSet::create which directly fills the correct id
To create and use an own message there's three things to do:
1. Implement a message based on MessageInterface(or simpler based on RawMessage)
2. Within the class-declaration, use the DECLARE_MESSAGE(name, parent, subindex)-macro to define the inheritance used in the message-tree
3. Register the message to the used MessageTypeSet, to register it to globalMessageTypeSet() simply use the REGISTER_MESSAGE(Message)-macro(within an implementation-file).
Now the message can be sent and received. */

class MessageInterface : public SafeShared /*, public virtual Serializable*/ {
  public:

    virtual void serialize( OutArchive& target ) = 0;

    ///this function must be thread-safe(so the message does not need to be locked when it is called)
    virtual MessageInfo& info() = 0;

    virtual const MessageInfo& info() const = 0;

    /**This is called once a message has been tried to be send, with the appropriate result, and can be used by the message to give some feedback to the sender. The Session used to send the message is still locked.
    */
    virtual void result( bool success ) = 0;

    virtual ~MessageInterface() {}
    ;
    
    virtual const char* name() {
      return staticName();
    }

    static const char* staticName() {
      return "MessageInterface";
    }

    ///This must be thread-safe! It should not be changed once the object was created and is used by multiple different threads.
    const MessageId& id() const {
      return info().id();
    }

    ///This should return "true" when the message wants a reply. gotReply will then be called once a reply has been received.
    virtual bool needReply() const {
      return false;
    }

    struct ReplyResult {
      bool messageHandled; ///This should indicate that the message was used locally and should not be handled on by higher instances
      bool awaitingMore; ///whether the message is awaiting more replies.(if this is true, the message will not be deleted and needReply will be called again as soon as another reply arrives)
      ReplyResult( bool messageHandled_ = false, bool awaitingMore_ = false ) : messageHandled( messageHandled_ ), awaitingMore( awaitingMore_ ) {}
    }
    ;
    /** p can be invalid, that means that the reply timed out. It is not guaranteed that this function is called. See the description of ReplyResult. */
    virtual ReplyResult gotReply( const DispatchableMessage& p ) {
      UNUSED( p );
      return false;
    }
};

/**This class is used to derive messages from each other, it automatically creates some necessary entries for building static message-trees
each message must have an explicit sub-id. To create messages even easier and safer, see easymessage.h */

#define DECLARE_MESSAGE( Name, parent, subindex ) \
  public:                         \
  inline static const char* staticName() { return #Name; }    \
  typedef parent Precursor; \
  enum {  \
    preferredIndex = subindex   \
  };    \
  virtual const char* name()  { return staticName(); };  \
  /*This little helper-functions returns true when the type as which the function is called is the real type of the message*/ \
  inline bool isDerived() { return strcmp( #Name, name() ) != 0; } \


///simple container-helper-class that could be removed
class DispatchableMessage {
    MessagePointer msg_;
  public:
    DispatchableMessage( const MessagePointer& msg = MessagePointer() ) : msg_( msg ) {}

    operator bool() const {
      return ( bool ) msg_;
    }

    operator MessagePointer() const {
      return msg_;
    }

    MessagePointer* operator->() {
      return & msg_;
    }
};


/**This allows multiple differing dispatch-targets(like the server + the thread). */
class MessageTypeSet {
    //protected:
    ///this should be optimized, and all the no more necessary template dispatch-code above removed
    typedef std::map<MessageId, MessageFactoryInterface* > TypeMap;
    typedef std::map<std::string, MessageId> TypeNameMap;
    TypeMap types_; ///would be more efficient to build a tree from the Chains and walk through that
    TypeNameMap ids_;

    ///this should walk through the messages upwards until it finds the best match
    TypeMap::iterator search( const MessageId& id );

    MessageId allocateSubId( const MessageId& id, int preferredSubId = 0 );

    UniqueMessageId currentUniqueMessageId_;

    ///Changes the given id to the matching one
    MessageFactoryInterface* findFactory( MessageId& id ) const;
  public:
    MessageTypeSet();

    DispatchableMessage buildMessage( InArchive& from, const MessageInfo& inf ) const;

    ~MessageTypeSet();

    const MessageId& idFromName( const std::string& name );

    std::string stats() const;

    ///returns the class-name of the message(the most specialized one registered in this type-set)
    std::string identify( MessageInterface* msg ) const;

    ///Register a static list of message-types to this type-set. Not yet registered parent-messages are automatically registered too.
    template < class Types >
    void registerMessageTypes() {
      RegisterAllTypes< typename Tree::MakeComplete< typename Tree::ExtractChains< Types >::Result, MessageInterface > ::Result, MessageTypeSet >::reg ( *this );
    }

    /** registers a type as slave of another given type, for simlicity registerMessageTypes should be used instead, together with a whole list of types */
    template <class MessageType, class ParentMessageType>
    void registerMessageType( int preferredSubId = 0 ) {
      if ( ids_.find( MessageType::staticName() ) != ids_.end() )
        return ;

      MessageId parentId = id<ParentMessageType>();
      MessageId myId = allocateSubId( parentId, preferredSubId );
      if ( !parentId && typeid( ParentMessageType ).name() != string( "MessagInterface" ) )
      cout << "while registering \"" << staticMessageName<MessageType>() << "\" the id of \"" << staticMessageName<ParentMessageType>() << "\" could not be allocated!" << endl;
      if ( myId ) {
        //cout << "registering message-type " << staticMessageName<MessageType>() << " with id " << myId.desc() << endl;
        ids_[ staticMessageName<MessageType>() ] = myId;
        if ( types_.find( myId ) != types_.end() )
          delete types_[ myId ];
        types_[ myId ] = new MessageFactory<MessageType>();
      } else {
        cout << "could not allocate id for message-type " << staticMessageName<MessageType>() << endl;
        ///No ID could be allocated
      }
    }

    template <class Type>
    inline const MessageId& id() {
      return idFromName( staticMessageName<Type>() );
    }

    ///Conveniently create a new message with the correct Message-Id and unique-Id filled. All parameters are passed to the constructor of the created message.
    template <class MessageType>
    inline MessageType* create() {
      return new MessageType( idFromName( staticMessageName<MessageType>() ).modifyUniqueId( currentUniqueMessageId_++ ) );
    }
    ///Conveniently create a new message with the correct Message-Id and unique-Id filled. All parameters are passed to the constructor of the created message.
    template <class MessageType, class ParamType1>
    inline MessageType* create( ParamType1 param ) {
      return new MessageType( idFromName( staticMessageName<MessageType>() ).modifyUniqueId( currentUniqueMessageId_++ ), param );
    }
    ///Conveniently create a new message with the correct Message-Id and unique-Id filled. All parameters are passed to the constructor of the created message.
    template <class MessageType, class ParamType1, class ParamType2>
    inline MessageType* create( ParamType1 param, ParamType2 param2 ) {
      return new MessageType( idFromName( staticMessageName<MessageType>() ).modifyUniqueId( currentUniqueMessageId_++ ), param, param2 );
    }
    ///Conveniently create a new message with the correct Message-Id and unique-Id filled. All parameters are passed to the constructor of the created message.
    template <class MessageType, class ParamType1, class ParamType2, class ParamType3>
    inline MessageType* create( ParamType1 param, ParamType2 param2, ParamType3 param3 ) {
      return new MessageType( idFromName( staticMessageName<MessageType>() ).modifyUniqueId( currentUniqueMessageId_++ ), param, param2, param3 );
    }
    ///Conveniently create a new message with the correct Message-Id and unique-Id filled. All parameters are passed to the constructor of the created message.
    template <class MessageType, class ParamType1, class ParamType2, class ParamType3, class ParamType4>
    inline MessageType* create( ParamType1 param, ParamType2 param2, ParamType3 param3, ParamType4 param4 ) {
      return new MessageType( idFromName( staticMessageName<MessageType>() ).modifyUniqueId( currentUniqueMessageId_++ ), param, param2, param3, param4 );
    }
    ///Conveniently create a new message with the correct Message-Id and unique-Id filled. All parameters are passed to the constructor of the created message.
    template <class MessageType, class ParamType1, class ParamType2, class ParamType3, class ParamType4, class ParamType5>
    inline MessageType* create( ParamType1 param, ParamType2 param2, ParamType3 param3, ParamType4 param4, ParamType5 param5 ) {
      return new MessageType( idFromName( staticMessageName<MessageType>() ).modifyUniqueId( currentUniqueMessageId_++ ), param, param2, param3, param4, param5 );
    }
    ///Conveniently create a new message with the correct Message-Id and unique-Id filled. All parameters are passed to the constructor of the created message.
    template <class MessageType, class ParamType1, class ParamType2, class ParamType3, class ParamType4, class ParamType5, class ParamType6>
    inline MessageType* create( ParamType1 param, ParamType2 param2, ParamType3 param3, ParamType4 param4, ParamType5 param5, ParamType6 param6 ) {
      return new MessageType( idFromName( staticMessageName<MessageType>() ).modifyUniqueId( currentUniqueMessageId_++ ), param, param2, param3, param4, param5, param6 );
    }
};


/**This is a message-type-set dedicated only to dispatching a chosen set of message-types
to a chosen dispatch-target. It delivers the given message as the most special type the message
is based on, by calling "TargetType::dispatchMessage( MessageType* )". Messages must be the list of messages(without any inheritance-info)*/
template <class TargetType, class Messages>
class MessageDispatcher {
    TargetType& targ_;

    struct EntryTemplate {
      template <class MessageType>
      struct Entry {
        int dispatch( TargetType& targ, MessageInterface* msg ) const {
          //cout << "dispatching as " << staticMessageName<MessageType>() << endl;
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
      nodes_.find( msg->id(), act );
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
        return targ.dispatchMessage( ( MessageType* ) msg );
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

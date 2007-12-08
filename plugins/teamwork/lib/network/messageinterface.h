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

/** This is the file you should include when you only want to handle messages
 *
 * */

#include "networkfwd.h"
#include "safesharedptr.h"
#include "networkexport.h"
#include <vector>

#ifndef MESSAGEINTERFACE_H
#define MESSAGEINTERFACE_H

namespace Teamwork {

///UniqueMessageId represents the identity of a single message
typedef uint64_t UniqueMessageId;

///MessageType represents the Message-Type
class NETWORK_EXPORT MessageType {
  public:
    typedef std::vector<unsigned char> IdList;
  private:
    IdList idList_;
    unsigned int fastId_; ///can only be used when the idList has <= 4 entries
    bool useFastId;
    UniqueMessageId uniqueId_;  ///This is only filled when the MessageType belongs to a Message(0 means invalid)
    void packFastId();
  public:
    MessageType( IdList IDs = IdList() );

    MessageType( InArchive& from );

    template<class Archive>
    void serialize( Archive& target, const uint /*version*/ = 0 );

    ///Only compares the type-id, not the uniqueId
    bool operator == ( const MessageType& rhs ) const;

    bool operator < ( const MessageType& rhs ) const;

    bool startsWith( const MessageType& rhs ) const;

    ///Returns the length of the id
    int length() const;
    
    MessageType& operator += ( unsigned char append );

    MessageType& operator -- ();

    operator bool() const;

    std::string desc() const;

    operator const unsigned char*() const;
};

///this class holds information about the instance of a message(it's id and an optional pointer to the session/user it was created in/for)
class NETWORK_EXPORT MessageInfo {
    MessageType id_;
    SessionPointer session_;
    UserPointer user_;
    MessagePointer replyToMessage_;
    UniqueMessageId isReplyTo_;
    UniqueMessageId uniqueId_;
    bool deserialized_;
  public:

    //Only for temporary usage
    MessageInfo();

    MessageInfo( InArchive& arch );
    MessageInfo( const MessageType& type, UniqueMessageId id );
    MessageInfo( const MessageType& type, UniqueMessageId id, const SessionPointer& session, UniqueMessageId isReplyTo = 0 );
    MessageInfo( const MessageInfo& rhs );
    MessageInfo& operator = ( const MessageInfo& rhs );
    ~MessageInfo();

    ///The session this message was received through/has been sent through.(whether received or sent: see isIncoming() )
    SessionPointer session() const;

    ///The user this message is related to.
    UserPointer user() const;

    ///Sets the session, and invalidates the user. If the session has an associated user, the user is used instead.
    void setSession( const SessionPointer& sess );

    ///Sets the user and invalidates the session. All session()-calls will be redirected to the session associated to the user.
    void setUser( const UserPointer& sess );

    const MessageType& type() const;

    void serialize( OutArchive& arch, const uint version = 0 );

    /**The unique id is the identity of a message-instance, not of the message-type */
    UniqueMessageId uniqueId() const;

    /**Since the uniqueId is not used for sorting this casts away constness so the id of a MessageType that is used as Key in a Map can be changed */
    void setUniqueId( UniqueMessageId newId ) const;
    
    /** if the message is a reply to another message, this should be the uniqueId of the other message.
      this function is thread-safe.(because after creation, this value should not be changed anymore) */
    UniqueMessageId isReplyTo() const;

    /** to make one message a reply to another while sending(to the other side), this is enough. To mark the message for local use, setReplyMessage(...) should be set too.*/
    void setReply( UniqueMessageId replyId );

    /** this does not automatically set the reply-id, it just sets a message-pointer. */
    void setReplyMessage( const MessagePointer& msg );

    ///If the reply is not handled directly by the message that the reply has gone to, the message this is a reply to is stored here.(The message with the id isReplyTo() ), may be invalid
    const MessagePointer& replyToMessage() const;

    void setId( const MessageType& id );

    /**Removes all kept references to important objects like sessions(those might keep the sessions alive too long)
      this should be called before a message is stored permanently */

    void makePersistent();

    ///Returns whether the message was constructed from a stream(if false, it was created locally)
    bool isIncoming() const;

    ///This can be used to set the isIncoming()-flag. The application or messages may use the flag as indication whether the message is incoming or outgoing.
    void setIsIncoming( bool b );
};

/**Since messages can have different IDs in different Message-Sets, the id's cannot be hardcoded and must be given with each creation.
That is simplified by using MessageTypeSet::create which directly fills the correct id
To create and use an own message there's three things to do:
1. Implement a message based on MessageInterface(or simpler based on RawMessage)
2. Within the class-declaration, use the DECLARE_MESSAGE(name, parent, subindex)-macro to define the inheritance used in the message-tree
3. Register the message to the used MessageTypeSet, to register it to globalMessageTypeSet() simply use the REGISTER_MESSAGE(Message)-macro(within an implementation-file).
Now the message can be sent and received. */

class NETWORK_EXPORT MessageInterface : public SafeShared /*, public virtual Serializable*/ {
  public:

    virtual void serialize( OutArchive& target ) = 0;

    ///this function must be thread-safe(so the message does not need to be locked when it is called)
    virtual MessageInfo& info() = 0;

    virtual const MessageInfo& info() const = 0;

    /**This is called once a message has been tried to be send, with the appropriate result, and can be used by the message to give some feedback to the sender. The Session used to send the message is still locked.
    */
    virtual void result( bool success ) = 0;

    virtual ~MessageInterface();
    
    ///This function is thread-safe
    virtual const char* name();

    ///This function must be implemented for all messages, it is atomatically cared about by the DECLARE_MESSAGE(..) macros etc.
    static const char* staticName();

    ///This is just a convenience-function that redirects to info().type()
    const MessageType& type() const;

    ///This is just a convenience-function that redirects to info().uniqueId()
    const UniqueMessageId uniqueId() const;

    ///This should return "true" when the message wants a reply. gotReply will then be called once a reply has been received. The default-implementation returns false.
    virtual bool needReply() const;

    struct ReplyResult {
      bool messageHandled; ///This should indicate that the message was used locally and should not be handled on by higher instances
      bool awaitingMore; ///whether the message is awaiting more replies.(if this is true, the message will not be deleted and needReply will be called again as soon as another reply arrives)
      ReplyResult( bool messageHandled_ = false, bool awaitingMore_ = false );
    }
    ;
    /** p can be invalid, that means that the reply timed out. It is not guaranteed that this function is called. See the description of ReplyResult. */
    virtual ReplyResult gotReply( const MessagePointer& p );
};
}

/**This class is used to derive messages from each other, it automatically creates some necessary entries for building static message-trees
each message must have an explicit sub-id. You still have to care about
the correct constructors.
To create messages even easier and safer, see easymessage.h */

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

#endif

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

//#define NO_SEND_HELPER

#ifndef MESSAGESENDHELPER_H
#define MESSAGESENDHELPER_H

#include "networkfwd.h"
#include "sessioninterface.h"
#include "messageinterface.h"
#include "networkexport.h"

namespace Teamwork {
  class MessageTypeSet;

///A little helper-class that allows creating types with less writing(can be imported into any class by inheritance)
class NETWORK_EXPORT MessageSendHelper {
    MessageTypeSet& types_;
  public:
    MessageSendHelper( MessageTypeSet& types = globalMessageTypeSet() ) : types_( types ) {}

#ifndef NO_SEND_HELPER
    
    /**The following is a set of little helper-functions that are used to reduce the mass of code.
     * They are used to send message, which includes their construction.
     * */
    template <class MessageType>
    inline bool send( SessionInterface* s ) {
      return s->send( new MessageType( types_ ) );
    }

    template <class MessageType, class ParamType1>
    inline bool send( SessionInterface* s, const ParamType1& param ) {
      return s->send( new MessageType( types_, param ) );
    }
    template <class MessageType, class ParamType1, class ParamType2>
    inline bool send( SessionInterface* s, ParamType1& param, ParamType2& param2 ) {
      return s->send( new MessageType( types_, param, param2 ) );
    }

    template <class MessageType, class ParamType1, class ParamType2>
    inline bool send( SessionInterface* s, const ParamType1& param, const ParamType2& param2 ) {
      return s->send( new MessageType( types_, param, param2 ) );
    }

    template <class MessageType, class ParamType1, class ParamType2, class ParamType3>
    inline bool send( SessionInterface* s, ParamType1& param, ParamType2& param2, ParamType3& param3 ) {
      return s->send( new MessageType( types_, param, param2, param3 ) );
    }

    template <class MessageType, class ParamType1, class ParamType2, class ParamType3>
    inline bool send( SessionInterface* s, const ParamType1& param, const ParamType2& param2, const ParamType3& param3 ) {
      return s->send( new MessageType( types_, param, param2, param3 ) );
    }

    template <class MessageType, class ParamType1, class ParamType2, class ParamType3, class ParamType4>
    inline bool send( SessionInterface* s, ParamType1& param, ParamType2& param2, ParamType3& param3, ParamType4& param4 ) {
      return s->send( new MessageType( types_, param, param2, param3, param4 ) );
    }

    template <class MessageType, class ParamType1, class ParamType2, class ParamType3, class ParamType4>
    inline bool send( SessionInterface* s, const ParamType1& param, const ParamType2& param2, const ParamType3& param3, const ParamType4& param4 ) {
      return s->send( new MessageType( types_, param, param2, param3, param4 ) );
    }

    template <class MessageType, class ParamType1, class ParamType2, class ParamType3, class ParamType4, class ParamType5>
    inline bool send( SessionInterface* s, const ParamType1& param, const ParamType2& param2, const ParamType3& param3, const ParamType4& param4, const ParamType5& param5 ) {
      return s->send( new MessageType( types_, param, param2, param3, param4, param5 ) );
    }

    template <class MessageType, class ParamType1, class ParamType2, class ParamType3, class ParamType4, class ParamType5>
    inline bool send( SessionInterface* s, ParamType1& param, ParamType2& param2, ParamType3& param3, ParamType4& param4, ParamType5& param5 ) {
      return s->send( new MessageType( types_, param, param2, param3, param4, param5 ) );
    }

    template <class MessageType>
    inline bool sendReply( MessageInterface* replyTo ) {
      if ( !replyTo->info().session() )
        return false;
      return replyTo->info().session().unsafe()->sendReply( new MessageType( types_ ), replyTo );
    }

    template <class MessageType, class ParamType1>
    inline bool sendReply( MessageInterface* replyTo, const ParamType1& param ) {
      if ( !replyTo->info().session() )
        return false;
      return replyTo->info().session().unsafe()->sendReply( new MessageType( types_, param ), replyTo );
    }

    template <class MessageType, class ParamType1, class ParamType2>
    inline bool sendReply( MessageInterface* replyTo, ParamType1& param, ParamType2& param2 ) {
      if ( !replyTo->info().session() )
        return false;
      return replyTo->info().session().unsafe()->sendReply( new MessageType( types_, param, param2 ), replyTo );
    }

    template <class MessageType, class ParamType1, class ParamType2>
    inline bool sendReply( MessageInterface* replyTo, const ParamType1& param, const ParamType2& param2 ) {
      if ( !replyTo->info().session() )
        return false;
      return replyTo->info().session().unsafe()->sendReply( new MessageType( types_, param, param2 ), replyTo );
    }

    template <class MessageType, class ParamType1, class ParamType2, class ParamType3>
    inline bool sendReply( MessageInterface* replyTo, ParamType1& param, ParamType2& param2, ParamType3& param3 ) {
      if ( !replyTo->info().session() )
        return false;
      return replyTo->info().session().unsafe()->sendReply( new MessageType( types_, param, param2, param3 ), replyTo );
    }
    template <class MessageType, class ParamType1, class ParamType2, class ParamType3>
    inline bool sendReply( MessageInterface* replyTo, const ParamType1& param, const ParamType2& param2, const ParamType3& param3 ) {
      if ( !replyTo->info().session() )
        return false;
      return replyTo->info().session().unsafe()->sendReply( new MessageType( types_, param, param2, param3 ), replyTo );
    }

    template <class MessageType, class ParamType1, class ParamType2, class ParamType3, class ParamType4>
    inline bool sendReply( MessageInterface* replyTo, ParamType1& param, ParamType2& param2, ParamType3& param3, ParamType4& param4 ) {
      if ( !replyTo->info().session() )
        return false;
      return replyTo->info().session().unsafe()->sendReply( new MessageType( types_, param, param2, param3, param4 ), replyTo );
    }
    template <class MessageType, class ParamType1, class ParamType2, class ParamType3, class ParamType4>
    inline bool sendReply( MessageInterface* replyTo, const ParamType1& param, const ParamType2& param2, const ParamType3& param3, const ParamType4& param4 ) {
      if ( !replyTo->info().session() )
        return false;
      return replyTo->info().session().unsafe()->sendReply( new MessageType( types_, param, param2, param3, param4 ), replyTo );
    }
#endif
};

///This can be used to simply send messages using the global message-type-set, @todo remove this
NETWORK_EXPORT MessageSendHelper& globalMessageSendHelper();
}
#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on

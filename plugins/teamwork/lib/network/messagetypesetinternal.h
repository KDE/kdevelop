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
#ifndef MESSAGETYPESETINTERNAL_H
#define MESSAGETYPESETINTERNAL_H

#include "statictree.h"
#include "messageinterface.h"
#include <string>

#define  DISABLEWARNINGS

namespace Teamwork {
using namespace Tree;
class MessageInfo;
struct MessageFactoryInterface {
  virtual MessagePointer buildMessage( InArchive& from , const MessageInfo& inf ) = 0;
  virtual std::string identify() = 0;
#ifdef DISABLEWARNINGS
  virtual ~MessageFactoryInterface() {}
#endif
}
;

template <class MessageType>
struct MessageFactory : public MessageFactoryInterface {
  virtual MessagePointer buildMessage( InArchive& from, const MessageInfo& inf ) {
    return MessagePointer( new MessageType( from, inf ) );
  }
  virtual std::string identify() {
    return MessageType::staticName();
  }

#ifdef DISABLEWARNINGS
  virtual ~MessageFactory() {}
#endif
}
;

template < class Type, class Target >
struct RegisterAllTypes {
  static void reg( Target& targ ) {
    targ.Error_Bad_Type_List();
  };
};

template < class Type, class Parent, uchar preferredSubId, class Target >
struct RegisterAllTypes< Chain< Type, Parent, preferredSubId >, Target > {
  static void reg( Target& targ ) {
    targ.template registerMessageType< Type, Parent > ( preferredSubId );
  };
};

template < class Side1, class Side2, class Target >
struct RegisterAllTypes< Binder< Side1, Side2 >, Target > {
  static void reg( Target& targ ) {
    RegisterAllTypes< Side2, Target >::reg( targ );
    RegisterAllTypes< Side1, Target >::reg( targ );
  };
};
}

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on

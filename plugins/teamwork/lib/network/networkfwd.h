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

#ifndef NETWORKFWD_H
#define NETWORKFWD_H

#include "networkexport.h"

//#define USE_POLYMORPHIC_ARCHIVE
#define USE_TEXT_ARCHIVE

class MessageSerialization;
class NormalSerialization;
class BoostSerialization;

#ifndef SAFESHAREDPTR_HAVE_DEFAULT
#define SAFESHAREDPTR_HAVE_DEFAULT
template<class Type, class Serialization = NormalSerialization>
class SafeSharedPtr;
#endif

#ifndef WEAKSAFESHAREDPTR_HAVE_DEFAULT
#define WEAKSAFESHAREDPTR_HAVE_DEFAULT
template<class Type, class Serialization = NormalSerialization>
class WeakSafeSharedPtr;
#endif

namespace boost {
  namespace archive {
    class binary_oarchive;
    class binary_iarchive;

    class text_oarchive;
    class text_iarchive;

    class xml_oarchive;
    class xml_iarchive;

    class polymorphic_iarchive;
    class polymorphic_oarchive;
  }
}

namespace Teamwork {
  class HandlerInterface;
  class BasicTCPSession;
  class SessionInterface;
  class MultiSession;
  class ForwardSession;
  class BasicServer;
  class MessageInterface;
  class User;
  class UserIdentity;
  class Logger;
  class LoggerPrinter;
  class MessageTypeSet;

  class ClientSessionDesc;
  class ServerInformation;
  class Server;

  typedef SafeSharedPtr<HandlerInterface> HandlerPointer;
  typedef SafeSharedPtr<SessionInterface> SessionPointer;
  typedef SafeSharedPtr<BasicServer> ServerPointer;
  typedef SafeSharedPtr<MessageInterface, MessageSerialization> MessagePointer;
  typedef SafeSharedPtr<User, BoostSerialization> UserPointer;
  typedef SafeSharedPtr<Logger> LoggerPointer;
  typedef SafeSharedPtr<MultiSession> MultiSessionPointer;
  typedef SafeSharedPtr<ForwardSession> ForwardSessionPointer;

  typedef WeakSafeSharedPtr<SessionInterface> WeakSessionPointer;

  #ifdef USE_BINARY_ARCHIVE
  typedef boost::archive::binary_oarchive OutArchive;
  typedef boost::archive::binary_iarchive InArchive;
  #endif
  
  #ifdef USE_POLYMORPHIC_ARCHIVE
  typedef boost::archive::polymorphic_oarchive OutArchive;
  typedef boost::archive::polymorphic_iarchive InArchive;
  #endif

  #ifdef USE_TEXT_ARCHIVE
  typedef boost::archive::text_oarchive OutArchive;
  typedef boost::archive::text_iarchive InArchive;
  #endif
  
  #ifdef USE_XML_ARCHIVE
  typedef boost::archive::xml_iarchive InArchive;
  typedef boost::archive::xml_oarchive OutArchive;
  #endif

  NETWORK_EXPORT MessageTypeSet& globalMessageTypeSet();
}

typedef unsigned int u32;

#endif

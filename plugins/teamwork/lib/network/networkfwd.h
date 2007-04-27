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

#ifndef NETWORKFWD_H
#define NETWORKFWD_H

//#define USE_POLYMORPHIC_ARCHIVE
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
//     namespace detail {
//       template<class Archive>
//       class polymorphic_oarchive_impl;
//       template<class Archive>
//       class polymorphic_iarchive_impl;
//     };
//     class polymorphic_binary_oarchive;
//     class polymorphic_binary_iarchive;

    class binary_oarchive;
    class binary_iarchive;

/*    typedef detail::polymorphic_oarchive_impl< text_oarchive_impl<text_oarchive> > polymorphic_text_oarchive;
    typedef detail::polymorphic_iarchive_impl< text_iarchive_impl<text_iarchive> > polymorphic_text_oarchive;
    class polymorphic_text_iarchive;*/
    class text_oarchive;
    class text_iarchive;
  };
};

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

  #ifndef USE_TEXT_ARCHIVE
   #ifdef USE_POLYMORPHIC_ARCHIVE
  typedef boost::archive::polymorphic_binary_oarchive OutArchive;
  typedef boost::archive::polymorphic_binary_iarchive InArchive;
  #else
  typedef boost::archive::binary_oarchive OutArchive;
  typedef boost::archive::binary_iarchive InArchive;
  #endif

  #else
   #ifdef USE_POLYMORPHIC_ARCHIVE
  typedef boost::archive::polymorphic_text_oarchive OutArchive;
  typedef boost::archive::polymorphic_text_iarchive InArchive;
  #else
  typedef boost::archive::text_oarchive OutArchive;
  typedef boost::archive::text_iarchive InArchive;
  #endif
  #endif

  MessageTypeSet& globalMessageTypeSet();
}

typedef unsigned int u32;

#endif

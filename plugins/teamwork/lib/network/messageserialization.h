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
#ifndef MESSAGESERIALIZATION_H
#define MESSAGESERIALIZATION_H

///This header defines a serialization-class that can be used as template-parameter to SafeSharedPtr so it uses the message-serialization-system and the MessageTypeSet to serialize and deserialize messages(it stores and loads the messages as vectors). It automatically uses the global message-set stored in globalMessageTypeSet(). A hard lock is done while saving. Warning: May throw exceptions on errors

#include "serialization.h"
#include "messagetypeset.h"
#include "helpers.h"
#include  <vector>

#define USE_BUFFER_SERIALIZATION

struct MessageSerialization {
  template <class Type, class Archive, class Target>
  static void load( Target& t, Archive& arch, int /*version*/ ) {
    bool b = false;
    arch >> boost::serialization::make_nvp( "valid", b );
    if ( b ) {
#ifdef USE_BUFFER_SERIALIZATION
#ifdef USE_TEXT_ARCHIVE
      std::string s;
      arch & boost::serialization::make_nvp( "data", s );
      std::vector<char> v;
      v.resize( s.length() + 1 );
      memcpy( &v[ 0 ], s.c_str(), s.length() + 1 );
#else
      std::vector<char> v;
      arch & boost::serialization::make_nvp( "data", v );
#endif

      t = ( ( Teamwork::MessagePointer ) Teamwork::buildMessageFromBuffer( v, Teamwork::globalMessageTypeSet(), 0 ) ).cast<Type>();
#else
      t = ( ( ( Teamwork::MessagePointer ) Teamwork::buildMessageFromArchive( arch, Teamwork::globalMessageTypeSet(), 0 ) ).cast<Type>();
#endif
    }
    else {
      t = 0;
    }
  }

  template <class Type, class Archive, class Target>
  static void save( Target& t, Archive& arch, int /*version*/ ) {
    if ( t ) {
      bool b = true;
#ifdef USE_BUFFER_SERIALIZATION

      arch << boost::serialization::make_nvp( "valid", b );
      std::vector<char> v;
      Teamwork::serializeMessageToBuffer( v, *t.unsafe() );

#ifdef USE_TEXT_ARCHIVE

      std::string s;
      if ( v.size() )
        s = &v[ 0 ];

      arch & boost::serialization::make_nvp( "data", s );
#else
      arch & boost::serialization::make_nvp( "data", v );
#endif

#else
      Teamwork::serializeMessageToArchive( arch, *t.unsafe() );
#endif
    } else {
      bool b = false;
      arch << boost::serialization::make_nvp( "valid", b );
    }
  }
};

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on

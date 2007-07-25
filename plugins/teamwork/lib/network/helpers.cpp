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

#include "helpers.h"
#include "serialization.h"
#include "messagetypeset.h"
#include "basicsession.h"
#include "vector_stream.h"

///We can choose an archive-type of our own
#ifdef USE_POLYMORPHIC_ARCHIVE
// #include <boost/archive/polymorphic_xml_iarchive.hpp>
// #include <boost/archive/polymorphic_xml_oarchive.hpp>
// typedef boost::archive::polymorphic_xml_iarchive InternalIArchive;
// typedef boost::archive::polymorphic_xml_oarchive InternalOArchive;

#include <boost/archive/polymorphic_text_iarchive.hpp>
#include <boost/archive/polymorphic_text_oarchive.hpp>
typedef boost::archive::polymorphic_text_iarchive InternalIArchive;
typedef boost::archive::polymorphic_text_oarchive InternalOArchive;
#else
typedef Teamwork::InArchive InternalIArchive;
typedef Teamwork::OutArchive InternalOArchive;
#endif

std::string formatInt( int i ) {
  ostringstream o;
  o << i;
  return o.str();
}

std::vector<char> binaryInt( int num ) {
  std::vector<char> ret;
  ret.resize( 4 );
  ( *( int* ) ( &ret[ 0 ] ) ) = num;
  return ret;
}

namespace Teamwork {
  MessagePointer buildMessageFromArchive( InArchive& arch, MessageTypeSet& messages  ) {
    return buildMessageFromArchive( arch, messages, 0 );
  }
  MessagePointer buildMessageFromArchive( InArchive& arch, MessageTypeSet& messages, SessionPointer sess  ) {
    MessageInfo inf( arch );
    inf.setSession( sess );
    
    return messages.buildMessage( arch, inf );
  }

  void serializeMessageToArchive( OutArchive& arch, MessageInterface& message ) {
    message.info().serialize( arch );
    
    message.serialize( arch ); ///it must be called like this, because the serialization would add type-information we don't want
  }

  MessagePointer buildMessageFromBuffer( const std::vector<char>& buf, MessageTypeSet& messages, SessionPointer sess ) {
    vector_read_stream str( buf );
    InternalIArchive arch( str );

    return buildMessageFromArchive( arch, messages, sess );
  }

  void serializeMessageToBuffer( std::vector<char>& buf, MessageInterface& message ) {
    vector_stream str( buf );
    InternalOArchive arch( str );

    serializeMessageToArchive( arch, message );
  }

  /*
  MessagePointer buildMessageFromBuffer( std::vector<char> buf, MessageTypeSet& messages, SessionPointer sess  )
  {
    buf.push_back( 0 );
    BinaryStreamBuf b( buf );
    std::ifstream str;

    str.rdbuf( &b );
    
    InArchive arch( str );

    MessageInfo inf( arch );
    inf.setSession( sess );
    
    return messages.buildMessage( arch, inf );
  }

  void serializeMessageToBuffer( std::vector<char>& buf, MessageInterface& message ) {
    BinaryStreamBuf b (buf );
    std::ofstream str;
    str.rdbuf( &b );
    OutArchive arch( str );
    
    message.info().serialize( arch );
    
    message.serialize( arch ); ///it must be called like this, because the serialization would add type-information we don't want
  }
  */
}

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on

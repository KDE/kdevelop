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

#include "helpers.h"
#include "message.h"
#include "basicsession.h"
#include "vector_stream.h"

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

  DispatchableMessage buildMessageFromBuffer( const std::vector<char>& buf, MessageTypeSet& messages, SessionPointer sess ) {
    vector_read_stream str( buf );
    InArchive arch( str );

    return buildMessageFromArchive( arch, messages, sess );
  }

  void serializeMessageToBuffer( std::vector<char>& buf, MessageInterface& message ) {
    vector_stream str( buf );
    OutArchive arch( str );

    serializeMessageToArchive( arch, message );
  }

  /*
  DispatchableMessage buildMessageFromBuffer( std::vector<char> buf, MessageTypeSet& messages, SessionPointer sess  )
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

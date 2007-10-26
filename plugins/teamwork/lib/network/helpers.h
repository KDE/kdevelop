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

#ifndef TEAMWORK_HELPERS
#define TEAMWORK_HELPERS

#include "networkfwd.h"
#include <vector>
#include <string>
#include <cstdlib>


namespace Teamwork {

  class BasicTCPSession;

  MessagePointer buildMessageFromArchive( InArchive& arch, MessageTypeSet& messages, SessionPointer sess  );
  
  ///May throw several exceptions
  void serializeMessageToArchive( OutArchive& arch, MessageInterface& message );

  ///May throw several exceptions
  void serializeMessageToBuffer( std::vector<char>& buf, MessageInterface& message );
  MessagePointer buildMessageFromBuffer( const std::vector<char>& buf, MessageTypeSet& messages, SessionPointer sess  );
  MessagePointer buildMessageFromBuffer( const std::vector<char>& buf, MessageTypeSet& messages);
}

template <class VecType, class Vec2Type>
std::vector<VecType>& operator += ( std::vector<VecType>& lhs, const std::vector<Vec2Type>& rhs ) {
  uint osize = lhs.size();
  lhs.resize( lhs.size() + ( (rhs.size() * sizeof( Vec2Type )) / sizeof(VecType))  );
  memcpy( &lhs[osize], &rhs[0], sizeof(Vec2Type) * rhs.size() );
  return lhs;
}

std::string formatInt( int i );

std::vector<char> binaryInt( int num );

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on

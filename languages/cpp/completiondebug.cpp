
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

#include "completiondebug.h"

namespace CompletionDebug {

DBGStreamType dbgState;

const int completionMaxDepth = 50;

DBGStreamType::KStreamType& dbg() {
 return dbgState.dbg();
}

bool dbgActive() {
	return true;
}

#ifndef NDEBUG
kdbgstream dbgMajor() {
 kdbgstream ret = kdDebug( 9007 );
 dbgState.outputPrefix( ret );
 return ret;
}
#else
kndbgstream dbgMajor() {
  return kndDebug();
};

#endif

#ifndef NDEBUG 
template<>
  KDDebugState<kdbgstream>::KDDebugState() : m_stream ( kdDebug( 9007 ) ) {
  }
#endif

template<>
  KDDebugState<kndbgstream>::KDDebugState() {
  }
}

// kate: indent-mode csands; tab-width 4;

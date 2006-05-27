
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

#ifndef __SAFETYCOUNTER_H__
#define __SAFETYCOUNTER_H__

#include <kdebug.h>

struct SafetyCounter {
  int safetyCounter;
  const int maxSafetyCounter;
  
  SafetyCounter( int max = 10000 ) : safetyCounter(0), maxSafetyCounter(max) {
  }
  
  void init() {
    safetyCounter = 0;
  }
  
  SafetyCounter& operator ++() {
    safetyCounter++;
    return *this;
  }
  
  operator bool() {
    safetyCounter++;
    bool ret = safetyCounter < maxSafetyCounter;
	  if( !ret ) {
      kdDebug( 9007) << "WARNING: Safety-depth-counter reached count > " << maxSafetyCounter << ", operation stopped" << endl;
		  if( safetyCounter == maxSafetyCounter ) kdDebug( 9007 ) << endl << kdBacktrace() << endl;
	  }
    
    return ret;
  }
  
};

#endif

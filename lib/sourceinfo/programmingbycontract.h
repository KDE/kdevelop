/***************************************************************************
                          programmingbycontract.h  -  description
                             -------------------
    begin                : Sat Nov 27 1999
    copyright            : (C) 1999 by Jonas Nordin
    email                : jonas.nordin@syncom.se
   
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/
#ifndef _PROGRAMMINGBYCONTRACT_H_
#define _PROGRAMMINGBYCONTRACT_H_
#include <kdebug.h>
//#ifdef ASSERTIONS
#define REQUIRE( TAG, REQ ) \
if( !(REQ) ) \
{ \
  kdDebug(9005) << "Precondition violation [" << __FILE__ << ":" << __LINE__ << "]" << endl; \
  kdDebug(9005) << "  " << TAG << ": " << #REQ << endl; \
  return; \
}

#define REQUIRE1( TAG, REQ, RETVAL ) \
if( !(REQ) ) \
{ \
  kdDebug(9005) << "Precondition violation [" << __FILE__ << ":" << __LINE__ << "]" << endl; \
  kdDebug(9005) << "  " << TAG << ": " << #REQ << endl; \
  return RETVAL; \
}
/*#else
#define REQUIRE( TAG, REQ )
#define REQUIRE1( TAG, REQ, RETVAL )
#endif

*/

#endif

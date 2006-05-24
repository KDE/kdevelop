
/***************************************************************************
   begin                : Sat Jul 21 2001
   copyright            : (C) 2001 by Victor R�er
   email                : victor_roeder@gmx.de
   copyright            : (C) 2002,2003 by Roberto Raggi
   email                : roberto@kdevelop.org
   copyright            : (C) 2005 by Adam Treat
   email                : manyoso@yahoo.com
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

#ifndef __BITHELPERS_H__
#define __BITHELPERS_H__

namespace BitHelpers {
template <class Type>
inline Type addFlag( Type loc, Type add ) {
 return (Type)( loc | add );
}

template <class Type>
inline Type remFlag( Type loc, Type rem ) {
 return (Type)( loc & (rem ^ 0xffffffff) );
}

template <class Type>
inline Type bitInvert( Type val ) {
 return (Type)(0xffffffff - (unsigned int)val);
}
}
#endif 
// kate: indent-mode csands; tab-width 4;


/***************************************************************************
                          ParsedStruct.cc  -  description
                             -------------------
    begin                : Mon Nov 21 1999
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

#include "ParsedStruct.h"
#include <kdebug.h>

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*-------------------------------------- CParsedStruct::CParsedStruct()
 * CParsedStruct()
 *   Constructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CParsedStruct::CParsedStruct()
{
  setItemType( PIT_STRUCT );
}

/*------------------------------------- CParsedStruct::~CParsedStruct()
 * ~CParsedStruct()
 *   Destructor.
 *
 * Parameters: 
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CParsedStruct::~CParsedStruct()
{
}

/*-------------------------------------- CParsedStruct::CParsedStruct( &cps )
 * CParsedStruct( &cps )
 *   Copy - Constructor.
 *
 * Parameters:
 *   CParsedStruct
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CParsedStruct::CParsedStruct( CParsedStruct& cpm ) : CParsedContainer( cpm )
{
  setItemType( PIT_STRUCT );
}

/*********************************************************************
 *                                                                   *
 *                          PUBLIC METHODS                           *
 *                                                                   *
 ********************************************************************/

/*------------------------------------------------- CClassStore::out()
 * out()
 *   Output this object to stdout.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedStruct::out()
{
  if( !comment.isEmpty() )
    kdDebug() << "   " << comment << "\n";

  kdDebug() << "   " << path() << "\n";
  for( attributeIterator.toFirst(); attributeIterator.current(); ++attributeIterator )
    attributeIterator.current()->out();
}

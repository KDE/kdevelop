/*******************************************************************************
**
** Filename   : diffmodellist.h
** Created on : 24 januari, 2004
** Copyright  : (c) 2004 Otto Bruggeman
** Email      : bruggie@home.nl
**
*******************************************************************************/

/*******************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
*******************************************************************************/

#ifndef DIFFMODELLIST_H
#define DIFFMODELLIST_H

#include <q3valuelist.h> // include for the base class

#include "diffmodel.h"

namespace Diff2
{

typedef Q3ValueListIterator<DiffModel*> DiffModelListIterator;
typedef Q3ValueListConstIterator<DiffModel*> DiffModelListConstIterator;

class DiffModelList : public Q3ValueList<DiffModel*>
{
public:
	DiffModelList() {}
	DiffModelList( const DiffModelList &list ) : Q3ValueList<DiffModel*>( list ) {}
	virtual ~DiffModelList()
	{
		clear();
	}

public:
	virtual void sort();

}; // End of class DiffModelList

} // End of Namespace Diff2

#endif // DIFFMODELLIST_H

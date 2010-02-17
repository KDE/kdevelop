/*******************************************************************************
**
** Filename   : diffmodellist.h
** Created on : 24 januari, 2004
** Copyright 2004-2005, 2009 Otto Bruggeman <bruggie@gmail.com>
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

#include <QtCore/QList> // include for the base class

#include "diffmodel.h"
#include "diff2export.h"

namespace Diff2
{

typedef QList<DiffModel*>::Iterator DiffModelListIterator;
typedef QList<DiffModel*>::ConstIterator DiffModelListConstIterator;

class DIFF2_EXPORT DiffModelList : public QList<DiffModel*>
{
public:
	DiffModelList() {}
	DiffModelList( const DiffModelList &list ) : QList<DiffModel*>( list ) {}
	virtual ~DiffModelList()
	{
		// Memleak as indicated by valgrind
		while ( !isEmpty() )
			delete takeFirst();
	}

public:
	virtual void sort();

}; // End of class DiffModelList

} // End of Namespace Diff2

#endif // DIFFMODELLIST_H

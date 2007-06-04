/*******************************************************************************
**
** Filename   : diffmodellist.cpp
** Created on : 26 march, 2004
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

#include <kdebug.h>
#include <q3tl.h>
#include "diffmodellist.h"

using namespace Diff2;

void DiffModelList::sort()
{
	qHeapSort(*this);
}


/*******************************************************************************
**
** Filename   : diffmodellist.cpp
** Created on : 26 march, 2004
** Copyright 2004 Otto Bruggeman <bruggie@gmail.com>
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

#include "diffmodellist.h"

#include <kdebug.h>

using namespace Diff2;

bool diffModelCompare(DiffModel* model1, DiffModel* model2)
{
	return *model1 < *model2;
}

void DiffModelList::sort()
{
	qSort(begin(), end(), diffModelCompare);
}

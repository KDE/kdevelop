/***************************************************************************
 *   Copyright (C) 2000-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _OUTPUTVIEWSFACTORY_H_
#define _OUTPUTVIEWSFACTORY_H_

#include <kdevgenericfactory.h>

#include "appoutputviewpart.h"
#include "makeviewpart.h"

class KDevPluginInfo;

typedef K_TYPELIST_2( AppOutputViewPart, MakeViewPart ) OutputViews;
typedef KDevGenericFactory< OutputViews > OutputViewsFactory;

const KDevPluginInfo *outputViewsInfo();

#endif

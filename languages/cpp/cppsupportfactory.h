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

#ifndef _CPPSUPPORTFACTORY_H_
#define _CPPSUPPORTFACTORY_H_

#include <kdevgenericfactory.h>
#include "cppsupportpart.h"

class CppSupportFactory : public KDevGenericFactory<CppSupportPart>
{
public:
    CppSupportFactory();

protected:
    virtual KInstance *createInstance();
};

#endif

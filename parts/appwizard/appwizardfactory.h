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

#ifndef _APPWIZARDFACTORY_H_
#define _APPWIZARDFACTORY_H_

#include <kdevgenericfactory.h>

#include "appwizardpart.h"

class KDevPluginInfo;

class AppWizardFactory : public KDevGenericFactory<AppWizardPart>
{
public:
    AppWizardFactory();
    
    static const KDevPluginInfo *info();

protected:
    virtual KInstance *createInstance();
};

#endif

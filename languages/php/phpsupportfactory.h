/*
 * KDevelop PHP Language Support
 *
 * Copyright (c) 2005 Escuder Nicolas <n.escuder@intra-links.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef PHPSUPPORTFACTORY_H
#define PHPSUPPORTFACTORY_H

#include "phplanguagesupport.h"
#include <kdevgenericfactory.h>

class KDevPluginInfo;

class PHPSupportFactory : public KDevGenericFactory<PHPLanguageSupport>
{
public:
    PHPSupportFactory();

    static const KDevPluginInfo *info();

protected:
    virtual KInstance *createInstance();
};

#endif // PHPSUPPORTFACTORY_H

// kate: indent-mode csands; tab-width 4;


/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _PARTLOADER_H_
#define _PARTLOADER_H_

#include <klibloader.h>
#include <ktrader.h>
#include <kservice.h>

class KDevPart;
class KDevApi;


class PartLoader
{
public:
    /**
     * Cleanup: Must be called to destory the factories,
     * Without this, KConfigs of compiled-in parts don't get sync()ed
     */
    static void cleanup();
    /**
     * Loads a KDevelop part representing a service.
     */
    static KDevPart *loadService(KService *service, const char *className,
                                 KDevApi *api, QObject *parent);
};

#endif

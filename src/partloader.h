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
     * Loads a component with a known service name.
     */
    static KDevPart *loadByName(const QString &name, const char *className,
                                KDevApi *api, QObject *parent);
    /**
     * Loads a component by taking the first choice in a trader query.
     */
    static KDevPart *loadByQuery(const QString &serviceType, const QString &constraint, const char *className,
                                 KDevApi *api, QObject *parent);
    /**
     * Loads all components returned by a trader query.
     */
    static QList<KDevPart> loadAllByQuery(const QString &serviceType, const QString &constraint, const char *className,
                                          KDevApi *api, QObject *parent);
};

#endif

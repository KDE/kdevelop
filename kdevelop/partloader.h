/***************************************************************************
 *   Copyright (C) 2000 by Bernd Gehrmann                                  *
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


class PartLoader
{
public:
    /**
     * Loads a component with a known service name.
     */
    static QObject *loadByName(QWidget *parent, const QString &name,
                               const char *className);
    /**
     * Loads a component by taking the first choice in a trader query.
     */
    static QObject *loadByQuery(QWidget *parent, const QString &serviceType, const QString &constraint,
                                const char *className);
    /**
     * Loads all components returned by a trader query.
     */
    static QObjectList loadAllByQuery(QWidget *parent, const QString &serviceType, const QString &constraint,
                                      const char *className);
};

#endif

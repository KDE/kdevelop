/***************************************************************************
 *   Copyright (C) 2003 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef KDEVSOURCEFORMATTER_H
#define KDEVSOURCEFORMATTER_H

#include <kdevplugin.h>

class KDevSourceFormatter : public KDevPlugin
{
public:
    KDevSourceFormatter(const QString& pluginName, const QString& icon, QObject* parent, const char* name);

    ~KDevSourceFormatter();

    virtual QString formatSource(const QString text);
};

#endif

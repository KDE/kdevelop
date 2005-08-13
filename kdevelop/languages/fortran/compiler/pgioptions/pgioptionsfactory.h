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

#ifndef _PGIOPTIONSFACTORY_H_
#define _PGIOPTIONSFACTORY_H_

#include <klibloader.h>


class PgiOptionsFactory : public KLibFactory
{
    Q_OBJECT

public:
    PgiOptionsFactory( QObject *parent=0, const char *name=0 );
    ~PgiOptionsFactory();

    virtual QObject* createObject( QObject *parent, const char *name,
                                   const char *classname, const QStringList &args);
    static KInstance *instance();

private:
    static KInstance *s_instance;
};

#endif

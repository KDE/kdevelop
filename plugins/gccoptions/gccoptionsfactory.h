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

#ifndef _GCCOPTIONSFACTORY_H_
#define _GCCOPTIONSFACTORY_H_

#include <klibloader.h>


class GccOptionsFactory : public KLibFactory
{
    Q_OBJECT

public:
    GccOptionsFactory( QObject *parent=0, const char *name=0 );
    ~GccOptionsFactory();

    virtual QObject* createObject( QObject *parent, const char *name,
                                   const char *classname, const QStringList &args);
    static KInstance *instance();

private:
    static KInstance *s_instance;
};

#endif

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

#include <kdebug.h>
#include <kinstance.h>
#include "main.h"
#include "cvsinterface.h"


extern "C" {

    void *init_libkdevcvsinterface()
    {
        return new CvsFactory;
    }
    
};


CvsFactory::CvsFactory(QObject *parent, const char *name)
    : KLibFactory(parent, name)
{
    instance();
}


CvsFactory::~CvsFactory()
{
    delete s_instance;
    s_instance = 0;
}


QObject *CvsFactory::create(QObject *parent, const char *name,
                            const char *classname, const QStringList &args)
{
    kdDebug(9006) << "Building CvsInterface" << endl;
    
    QObject *obj = new CvsInterface(parent, name);
    emit objectCreated(obj);
    return obj;
}


KInstance *CvsFactory::s_instance = 0;
KInstance *CvsFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevcvsinterface");

    return s_instance;
}

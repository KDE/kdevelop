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
#include "javasupport.h"


extern "C" {

    void *init_libkdevjavasupport()
    {
        return new JavaSupportFactory;
    }
    
};


JavaSupportFactory::JavaSupportFactory(QObject *parent, const char *name)
    : KLibFactory(parent, name)
{
    instance();
}


JavaSupportFactory::~JavaSupportFactory()
{
    delete s_instance;
    s_instance = 0;
}


QObject *JavaSupportFactory::create(QObject *parent, const char *name,
                                   const char */*classname*/, const QStringList &/*args*/)
{
    kdDebug(9007) << "Building JavaSupport" << endl;
    
    QObject *obj = new JavaSupport(parent, name);
    emit objectCreated(obj);
    return obj;
}


KInstance *JavaSupportFactory::s_instance = 0;
KInstance *JavaSupportFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevjavasupport");

    return s_instance;
}
#include "main.moc"

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
#include "classview.h"


extern "C" {

    void *init_libkdevclassview()
    {
        return new ClassFactory;
    }
    
};


ClassFactory::ClassFactory(QObject *parent, const char *name)
    : KLibFactory(parent, name)
{
    instance();
}


ClassFactory::~ClassFactory()
{
    delete s_instance;
    s_instance = 0;
}


QObject *ClassFactory::create(QObject *parent, const char *name,
                              const char */*classname*/, const QStringList &/*args*/)
{
    kdDebug(9001) << "Building ClassView" << endl;
    
    QObject *obj = new ClassView(parent, name);
    emit objectCreated(obj);
    return obj;
}


KInstance *ClassFactory::s_instance = 0;
KInstance *ClassFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevclassview");

    return s_instance;
}
#include "main.moc"

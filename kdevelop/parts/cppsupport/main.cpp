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
#include "cppsupport.h"


extern "C" {

    void *init_libkdevcppsupport()
    {
        return new CppSupportFactory;
    }
    
};


CppSupportFactory::CppSupportFactory(QObject *parent, const char *name)
    : KLibFactory(parent, name)
{
    instance();
}


CppSupportFactory::~CppSupportFactory()
{
    delete s_instance;
    s_instance = 0;
}


QObject *CppSupportFactory::create(QObject *parent, const char *name,
                                   const char */*classname*/, const QStringList &/*args*/)
{
    kdDebug(9007) << "Building CppSupport" << endl;
    
    QObject *obj = new CppSupport(parent, name);
    emit objectCreated(obj);
    return obj;
}


KInstance *CppSupportFactory::s_instance = 0;
KInstance *CppSupportFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevcppsupport");

    return s_instance;
}
#include "main.moc"

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
#include "classstore.h"


extern "C" {

    void *init_libkdevsourceinfo()
    {
        return new SourceInfoFactory;
    }
    
};


SourceInfoFactory::SourceInfoFactory(QObject *parent, const char *name)
    : KLibFactory(parent, name)
{
    instance();
}


SourceInfoFactory::~SourceInfoFactory()
{
    delete s_instance;
    s_instance = 0;
}


QObject *SourceInfoFactory::create(QObject *parent, const char *name,
                            const char *classname, const QStringList &args)
{
    kdDebug(9006) << "Building SourceInfo" << endl;
    
    QObject *obj = new ClassStore();
    emit objectCreated(obj);
    return obj;
}


KInstance *SourceInfoFactory::s_instance = 0;
KInstance *SourceInfoFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevsourceinfo");

    return s_instance;
}
#include "main.moc"

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

#include <kdebug.h>
#include <kinstance.h>

#include "javasupportfactory.h"
#include "javasupportpart.h"


extern "C" {

    void *init_libkdevjavasupport()
    {
        return new JavaSupportFactory;
    }
    
};


JavaSupportFactory::JavaSupportFactory(QObject *parent, const char *name)
    : KDevFactory(parent, name)
{
}


JavaSupportFactory::~JavaSupportFactory()
{
    delete s_instance;
    s_instance = 0;
}


KDevPart *JavaSupportFactory::createPartObject(KDevApi *api, QObject *parent,
                                              const QStringList &/*args*/)
{
    kdDebug(9013) << "Building JavaSupport" << endl;
    return new JavaSupportPart(api, parent, "java support part");
}


KInstance *JavaSupportFactory::s_instance = 0;
KInstance *JavaSupportFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevjavasupport");

    return s_instance;
}

#include "javasupportfactory.moc"

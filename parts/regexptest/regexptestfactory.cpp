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


#include <kdebug.h>
#include <kinstance.h>

#include "regexptestfactory.h"
#include "regexptestpart.h"


extern "C" {

    void *init_libkdevregexptest()
    {
        return new RegexpTestFactory;
    }
    
};


RegexpTestFactory::RegexpTestFactory(QObject *parent, const char *name)
    : KDevFactory(parent, name)
{
}


RegexpTestFactory::~RegexpTestFactory()
{
    delete s_instance;
    s_instance = 0;
}


KDevPart *RegexpTestFactory::createPartObject(KDevApi *api, QObject *parent,
                                              const QStringList &/*args*/)
{
    kdDebug(9023) << "Building RegexpTest" << endl;
    return new RegexpTestPart(api, parent, "regexptest part");
}


KInstance *RegexpTestFactory::s_instance = 0;
KInstance *RegexpTestFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevregexptest");

    return s_instance;
}

#include "regexptestfactory.moc"

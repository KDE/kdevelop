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

#include "cppsupportfactory.h"
#include "cppsupportpart.h"


extern "C" {

    void *init_libkdevcppsupport()
    {
        return new CppSupportFactory;
    }
    
};


CppSupportFactory::CppSupportFactory(QObject *parent, const char *name)
    : KDevFactory(parent, name)
{
}


CppSupportFactory::~CppSupportFactory()
{
    delete s_instance;
    s_instance = 0;
}


KDevPart *CppSupportFactory::createPartObject(KDevApi *api, QObject *parent,
                                              const QStringList &args)
{
    if (args.count() == 1)
        return new CppSupportPart(args[1], api, parent, "cpp support part");
    else {
        kdDebug(9007) << "Wrong args for kdevcppsupport library" << endl;
        return 0;
    }
}


KInstance *CppSupportFactory::s_instance = 0;
KInstance *CppSupportFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevcppsupport");

    return s_instance;
}

#include "cppsupportfactory.moc"

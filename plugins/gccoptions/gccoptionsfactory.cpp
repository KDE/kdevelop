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
#include "gccoptionsplugin.h"
#include "gccoptionsfactory.h"


extern "C" {

    void *init_libkdevgccoptions()
    {
        return new GccOptionsFactory;
    }
    
};


GccOptionsFactory::GccOptionsFactory(QObject *parent, const char *name)
    : KLibFactory(parent, name)
{
    instance();
}


GccOptionsFactory::~GccOptionsFactory()
{
    delete s_instance;
    s_instance = 0;
}


QObject *GccOptionsFactory::createObject(QObject *parent, const char *name,
                                         const char * /*classname*/, const QStringList &args)
{
    if (args.count() > 0 && qstrcmp(args[0].latin1(), "gcc") == 0) {
        kdDebug(9008) << "Building GccOptions" << endl;
        return new GccOptionsPlugin(false, parent, name);
    } else if (args.count() > 0 && qstrcmp(args[0].latin1(), "g++") == 0) {
        kdDebug(9008) << "Building GppOptions" << endl;
        return new GccOptionsPlugin(true, parent, name);
    } else {
        kdDebug(9008) << "Wrong args for kdevgccoptions library" << endl;
        if (args.count() > 0)
            kdDebug(9008) << args[0] << endl;
        return 0;
    }
}


KInstance *GccOptionsFactory::s_instance = 0;
KInstance *GccOptionsFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevgccoptions");

    return s_instance;
}

#include "gccoptionsfactory.moc"

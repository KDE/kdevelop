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
#include "pgioptionsplugin.h"
#include "pgioptionsfactory.h"


extern "C" {

    void *init_libkdevpgioptions()
    {
        return new PgiOptionsFactory;
    }
    
}


PgiOptionsFactory::PgiOptionsFactory(QObject *parent, const char *name)
    : KLibFactory(parent, name)
{
    instance();
}


PgiOptionsFactory::~PgiOptionsFactory()
{
    delete s_instance;
    s_instance = 0;
}


QObject *PgiOptionsFactory::createObject(QObject *parent, const char *name,
                                         const char * /*classname*/, const QStringList &args)
{
    if (args.count() > 0 && qstrcmp(args[0].latin1(), "pghpf") == 0) {
        kdDebug(9021) << "Building PgiOptions for PGHPF" << endl;
        return new PgiOptionsPlugin(PgiOptionsPlugin::PGHPF, parent, name);
    } else if (args.count() > 0 && qstrcmp(args[0].latin1(), "pgf77") == 0) {
        kdDebug(9021) << "Building PgiOptions for PGF77" << endl;
        return new PgiOptionsPlugin(PgiOptionsPlugin::PGF77, parent, name);
    } else {
        kdDebug(9021) << "Wrong args for kdevpgioptions library" << endl;
        if (args.count() > 0)
            kdDebug(9021) << args[0] << endl;
        return 0;
    }
}


KInstance *PgiOptionsFactory::s_instance = 0;
KInstance *PgiOptionsFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevpgioptions");

    return s_instance;
}

#include "pgioptionsfactory.moc"

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

#include "autoprojectfactory.h"
#include "autoprojectpart.h"


extern "C" {

    void *init_libkdevautoproject()
    {
        return new AutoProjectFactory;
    }
    
};


AutoProjectFactory::AutoProjectFactory(QObject *parent, const char *name)
    : KDevFactory(parent, name)
{
}


AutoProjectFactory::~AutoProjectFactory()
{
    delete s_instance;
    s_instance = 0;
}


KDevPart *AutoProjectFactory::createPartObject(KDevApi *api, QObject *parent,
                                               const QStringList &args)
{
    if (args.count() > 0 && qstrcmp(args[0].latin1(), "kde") == 0) {
        kdDebug(9020) << "Build KDEAutoProject" << endl;
        return new AutoProjectPart(api, true, parent, "auto project part");
    } else {
        kdDebug(9020) << "Building AutoProject" << endl;
        return new AutoProjectPart(api, false, parent, "auto project part");
    }
}


KInstance *AutoProjectFactory::s_instance = 0;
KInstance *AutoProjectFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevautoproject");

    return s_instance;
}

#include "autoprojectfactory.moc"

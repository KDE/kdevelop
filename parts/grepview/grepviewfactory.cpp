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

#include "grepviewfactory.h"
#include "grepviewpart.h"


extern "C" {

    void *init_libkdevgrepview()
    {
        return new GrepViewFactory;
    }
    
};


GrepViewFactory::GrepViewFactory(QObject *parent, const char *name)
    : KDevFactory(parent, name)
{
}


GrepViewFactory::~GrepViewFactory()
{
    delete s_instance;
    s_instance = 0;
}


KDevPart *GrepViewFactory::createPartObject(KDevApi *api, QObject *parent,
                                            const QStringList &/*args*/)
{
    kdDebug(9001) << "Building GrepView" << endl;
    return new GrepViewPart(api, parent, "grep view part");
}


KInstance *GrepViewFactory::s_instance = 0;
KInstance *GrepViewFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevgrepview");

    return s_instance;
}

#include "grepviewfactory.moc"

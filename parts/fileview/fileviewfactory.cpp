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

#include "fileviewfactory.h"
#include "fileviewpart.h"


extern "C" {

    void *init_libkdevfileview()
    {
        return new FileViewFactory;
    }
    
};


FileViewFactory::FileViewFactory(QObject *parent, const char *name)
    : KDevFactory(parent, name)
{
}


FileViewFactory::~FileViewFactory()
{
    delete s_instance;
    s_instance = 0;
}


KDevPart *FileViewFactory::createPartObject(KDevApi *api, QObject *parent,
                                             const QStringList &/*args*/)
{
    kdDebug(9017) << "Building FileView" << endl;
    return new FileViewPart(api, parent, "file view part");
}


KInstance *FileViewFactory::s_instance = 0;
KInstance *FileViewFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevfileview");

    return s_instance;
}

#include "fileviewfactory.moc"

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

#include "classviewfactory.h"
#include "classviewpart.h"


extern "C" {

    void *init_libkdevclassview()
    {
        return new ClassViewFactory;
    }
    
};


ClassViewFactory::ClassViewFactory(QObject *parent, const char *name)
    : KDevFactory(parent, name)
{
}


ClassViewFactory::~ClassViewFactory()
{
    delete s_instance;
    s_instance = 0;
}


KDevPart *ClassViewFactory::createPartObject(KDevApi *api, QObject *parent,
                                             const QStringList &/*args*/)
{
    kdDebug(9003) << "Building ClassView" << endl;
    return new ClassViewPart(api, parent, "class view part");
}


KInstance *ClassViewFactory::s_instance = 0;
KInstance *ClassViewFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevclassview");

    return s_instance;
}

#include "classviewfactory.moc"

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

#include "customprojectfactory.h"
#include "customprojectpart.h"


extern "C" {

    void *init_libkdevcustomproject()
    {
        return new CustomProjectFactory;
    }
    
};


CustomProjectFactory::CustomProjectFactory(QObject *parent, const char *name)
    : KDevFactory(parent, name)
{
}


CustomProjectFactory::~CustomProjectFactory()
{
    delete s_instance;
    s_instance = 0;
}


KDevPart *CustomProjectFactory::createPartObject(KDevApi *api, QObject *parent,
                                               const QStringList &args)
{
    kdDebug(9025) << "Building CustomProject" << endl;
    return new CustomProjectPart(api, parent, "custom project part");
}


KInstance *CustomProjectFactory::s_instance = 0;
KInstance *CustomProjectFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevcustomproject");

    return s_instance;
}

#include "customprojectfactory.moc"

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

#include "trollprojectfactory.h"
#include "trollprojectpart.h"


extern "C" {

    void *init_libkdevtrollproject()
    {
        return new TrollProjectFactory;
    }
    
};


TrollProjectFactory::TrollProjectFactory(QObject *parent, const char *name)
    : KDevFactory(parent, name)
{
}


TrollProjectFactory::~TrollProjectFactory()
{
    delete s_instance;
    s_instance = 0;
}


KDevPart *TrollProjectFactory::createPartObject(KDevApi *api, QObject *parent,
                                               const QStringList &args)
{
    kdDebug(9024) << "Building TrollProject" << endl;
    return new TrollProjectPart(api, parent, "troll project part");
}


KInstance *TrollProjectFactory::s_instance = 0;
KInstance *TrollProjectFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevtrollproject");

    return s_instance;
}

#include "trollprojectfactory.moc"

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

#include "doxygenfactory.h"
#include "doxygenpart.h"


extern "C" {

    void *init_libkdevdoxygen()
    {
        return new DoxygenFactory;
    }
    
};


DoxygenFactory::DoxygenFactory(QObject *parent, const char *name)
    : KDevFactory(parent, name)
{
}


DoxygenFactory::~DoxygenFactory()
{
    delete s_instance;
    s_instance = 0;
}


KDevPart *DoxygenFactory::createPartObject(KDevApi *api, QObject *parent,
                                         const QStringList &/*args*/)
{
    kdDebug(9026) << "Building Doxygen" << endl;
    return new DoxygenPart(api, parent, "doxygen part");
}


KInstance *DoxygenFactory::s_instance = 0;
KInstance *DoxygenFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevdoxygen");

    return s_instance;
}

#include "doxygenfactory.moc"

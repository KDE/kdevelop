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

#include "pythonfactory.h"
#include "pythonpart.h"


extern "C" {

    void *init_libkdevpython()
    {
        return new PythonFactory;
    }
    
};


PythonFactory::PythonFactory(QObject *parent, const char *name)
    : KDevFactory(parent, name)
{
}


PythonFactory::~PythonFactory()
{
    delete s_instance;
    s_instance = 0;
}


KDevPart *PythonFactory::createPartObject(KDevApi *api, QObject *parent,
                                          const QStringList &/*args*/)
{
    kdDebug(9001) << "Building Python" << endl;
    return new PythonPart(api, parent, "python part");
}


KInstance *PythonFactory::s_instance = 0;
KInstance *PythonFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevpython");

    return s_instance;
}

#include "pythonfactory.moc"

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

#include "pythonsupportfactory.h"
#include "pythonsupportpart.h"


extern "C" {

    void *init_libkdevpythonsupport()
    {
        return new PythonSupportFactory;
    }
    
};


PythonSupportFactory::PythonSupportFactory(QObject *parent, const char *name)
    : KDevFactory(parent, name)
{
}


PythonSupportFactory::~PythonSupportFactory()
{
    delete s_instance;
    s_instance = 0;
}


KDevPart *PythonSupportFactory::createPartObject(KDevApi *api, QObject *parent,
                                                 const QStringList &/*args*/)
{
    kdDebug(9014) << "Building PythonSupport" << endl;
    return new PythonSupportPart(api, parent, "python support part");
}


KInstance *PythonSupportFactory::s_instance = 0;
KInstance *PythonSupportFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevpythonsupport");

    return s_instance;
}

#include "pythonsupportfactory.moc"

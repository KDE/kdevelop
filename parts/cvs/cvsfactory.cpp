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

#include "cvsfactory.h"
#include "cvspart.h"


extern "C" {

    void *init_libkdevcvs()
    {
        return new CvsFactory;
    }
    
};


CvsFactory::CvsFactory(QObject *parent, const char *name)
    : KDevFactory(parent, name)
{
}


CvsFactory::~CvsFactory()
{
    delete s_instance;
    s_instance = 0;
}


KDevPart *CvsFactory::createPartObject(KDevApi *api, QObject *parent,
                                         const QStringList &/*args*/)
{
    kdDebug(9027) << "Building Cvs" << endl;
    return new CvsPart(api, parent, "cvs part");
}


KInstance *CvsFactory::s_instance = 0;
KInstance *CvsFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevcvs");

    return s_instance;
}

#include "cvsfactory.moc"

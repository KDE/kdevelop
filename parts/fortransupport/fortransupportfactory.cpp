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

#include "fortransupportfactory.h"
#include "fortransupportpart.h"


extern "C" {

    void *init_libkdevfortransupport()
    {
        return new FortranSupportFactory;
    }
    
};


FortranSupportFactory::FortranSupportFactory(QObject *parent, const char *name)
    : KDevFactory(parent, name)
{
}


FortranSupportFactory::~FortranSupportFactory()
{
    delete s_instance;
    s_instance = 0;
}


KDevPart *FortranSupportFactory::createPartObject(KDevApi *api, QObject *parent,
                                                 const QStringList &/*args*/)
{
    kdDebug(9014) << "Building FortranSupport" << endl;
    return new FortranSupportPart(api, parent, "fortran support part");
}


KInstance *FortranSupportFactory::s_instance = 0;
KInstance *FortranSupportFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevfortransupport");

    return s_instance;
}

#include "fortransupportfactory.moc"

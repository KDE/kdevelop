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

#include "perlsupportfactory.h"
#include "perlsupportpart.h"


extern "C" {

    void *init_libkdevperlsupport()
    {
        return new PerlSupportFactory;
    }
    
};


PerlSupportFactory::PerlSupportFactory(QObject *parent, const char *name)
    : KDevFactory(parent, name)
{
}


PerlSupportFactory::~PerlSupportFactory()
{
    delete s_instance;
    s_instance = 0;
}


KDevPart *PerlSupportFactory::createPartObject(KDevApi *api, QObject *parent,
                                               const QStringList &/*args*/)
{
    kdDebug(9016) << "Building PerlSupport" << endl;
    return new PerlSupportPart(api, parent, "perl support part");
}


KInstance *PerlSupportFactory::s_instance = 0;
KInstance *PerlSupportFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevperlsupport");

    return s_instance;
}

#include "perlsupportfactory.moc"

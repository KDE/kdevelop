/***************************************************************************
 *   Copyright (C) 2002 by Bernd Gehrmann                                  *
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
#include <kstandarddirs.h>

#include "abbrevfactory.h"
#include "abbrevpart.h"


extern "C" {

    void *init_libkdevabbrev()
    {
        return new AbbrevFactory;
    }
    
};


AbbrevFactory::AbbrevFactory(QObject *parent, const char *name)
    : KDevFactory(parent, name)
{
}


AbbrevFactory::~AbbrevFactory()
{
    delete s_instance;
    s_instance = 0;
}


KDevPart *AbbrevFactory::createPartObject(KDevApi *api, QObject *parent,
                                          const QStringList &/*args*/)
{
    kdDebug(9028) << "Building Abbrev" << endl;
    return new AbbrevPart(api, parent, "abbrev part");
}


KInstance *AbbrevFactory::s_instance = 0;
KInstance *AbbrevFactory::instance()
{
    if (!s_instance) {
        s_instance = new KInstance("kdevabbrev");
        KStandardDirs *dirs = s_instance->dirs();
        dirs->addResourceType("codetemplates", KStandardDirs::kde_default("data") + "kdevabbrev/templates/");
    }

    return s_instance;
}

#include "abbrevfactory.moc"

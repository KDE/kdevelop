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

#include "ctagsfactory.h"
#include "ctagspart.h"


extern "C" {

    void *init_libkdevctags()
    {
        return new CTagsFactory;
    }
    
};


CTagsFactory::CTagsFactory(QObject *parent, const char *name)
    : KDevFactory(parent, name)
{
}


CTagsFactory::~CTagsFactory()
{
    delete s_instance;
    s_instance = 0;
}


KDevPart *CTagsFactory::createPartObject(KDevApi *api, QObject *parent,
                                         const QStringList &/*args*/)
{
    kdDebug(9022) << "Building CTags" << endl;
    return new CTagsPart(api, parent, "ctags part");
}


KInstance *CTagsFactory::s_instance = 0;
KInstance *CTagsFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevctags");

    return s_instance;
}

#include "ctagsfactory.moc"

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

#include "scriptprojectfactory.h"
#include "scriptprojectpart.h"


extern "C" {

    void *init_libkdevscriptproject()
    {
        return new ScriptProjectFactory;
    }
    
};


ScriptProjectFactory::ScriptProjectFactory(QObject *parent, const char *name)
    : KDevFactory(parent, name)
{
}


ScriptProjectFactory::~ScriptProjectFactory()
{
    delete s_instance;
    s_instance = 0;
}


KDevPart *ScriptProjectFactory::createPartObject(KDevApi *api, QObject *parent,
                                                 const QStringList &/*args*/)
{
    kdDebug(9015) << "Building ScriptProject" << endl;
    return new ScriptProjectPart(api, parent, "script project part");
}


KInstance *ScriptProjectFactory::s_instance = 0;
KInstance *ScriptProjectFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevscriptproject");

    return s_instance;
}

#include "scriptprojectfactory.moc"

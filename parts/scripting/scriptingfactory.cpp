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

#include "scriptingfactory.h"
#include "scriptingpart.h"


extern "C" {

    void *init_libkdevscripting()
    {
        return new ScriptingFactory;
    }
    
};


ScriptingFactory::ScriptingFactory(QObject *parent, const char *name)
    : KDevFactory(parent, name)
{
}


ScriptingFactory::~ScriptingFactory()
{
    delete s_instance;
    s_instance = 0;
}


KDevPart *ScriptingFactory::createPartObject(KDevApi *api, QObject *parent,
                                             const QStringList &/*args*/)
{
    kdDebug(9001) << "Building Scripting" << endl;
    return new ScriptingPart(api, parent, "scripting part");
}


KInstance *ScriptingFactory::s_instance = 0;
KInstance *ScriptingFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevscripting");

    return s_instance;
}

#include "scriptingfactory.moc"

/***************************************************************************
 *   Copyright (C) 2000-2001 by Bernd Gehrmann                             *
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

#include "debuggerfactory.h"
#include "debuggerpart.h"


extern "C" {

    void *init_libkdevdebugger()
    {
        return new DebuggerFactory;
    }
    
};


DebuggerFactory::DebuggerFactory(QObject *parent, const char *name)
    : KDevFactory(parent, name)
{
}


DebuggerFactory::~DebuggerFactory()
{
    delete s_instance;
    s_instance = 0;
}


KDevPart *DebuggerFactory::createPartObject(KDevApi *api, QObject *parent,
                                            const QStringList &/*args*/)
{
    kdDebug(9012) << "Building Debugger" << endl;
    return new DebuggerPart(api, parent, "debugger part");
}


KInstance *DebuggerFactory::s_instance = 0;
KInstance *DebuggerFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevdebugger");

    return s_instance;
}

#include "debuggerfactory.moc"

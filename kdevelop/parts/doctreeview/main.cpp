/***************************************************************************
 *   Copyright (C) 2000 by Bernd Gehrmann                                  *
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
#include "main.h"
#include "doctreeview.h"


extern "C" {

    void *init_libkdevdoctreeview()
    {
        return new DocTreeFactory;
    }
    
};


DocTreeFactory::DocTreeFactory(QObject *parent, const char *name)
    : KLibFactory(parent, name)
{
    instance();
}


DocTreeFactory::~DocTreeFactory()
{
    delete s_instance;
    s_instance = 0;
}


QObject *DocTreeFactory::create(QObject *parent, const char *name,
                                const char *classname, const QStringList &args)
{
    kdDebug(9002) << "Building DocTreeView" << endl;
    
    QObject *obj = new DocTreeView(parent, name);
    emit objectCreated(obj);
    return obj;
}


KInstance *DocTreeFactory::s_instance = 0;
KInstance *DocTreeFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevdoctreeview");

    return s_instance;
}

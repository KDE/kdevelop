/***************************************************************************
 *   Copyright (C) 2000 by Sandy Meier                                     *
 *   smeier@kdevelop.org                                                   *
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
#include "projectview.h"


extern "C" {

    void *init_libkdevprojectview()
    {
        return new ProjectViewFactory;
    }
    
};


ProjectViewFactory::ProjectViewFactory(QObject *parent, const char *name)
    : KLibFactory(parent, name)
{
    instance();
}


ProjectViewFactory::~ProjectViewFactory()
{
    delete s_instance;
    s_instance = 0;
}


QObject *ProjectViewFactory::create(QObject *parent, const char *name,
                              const char *classname, const QStringList &args)
{
    kdDebug(9001) << "Building ProjectView" << endl;
    
    QObject *obj = new ProjectView(parent, name);
    emit objectCreated(obj);
    return obj;
}


KInstance *ProjectViewFactory::s_instance = 0;
KInstance *ProjectViewFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevrprojectview");

    return s_instance;
}
#include "main.moc"

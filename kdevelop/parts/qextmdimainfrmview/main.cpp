/***************************************************************************
         mdimainfrmfactory.cpp  - the library factory
                             -------------------
    begin                : Thu Jul 27 2000
    copyright            : (C) 2000 by Falk Brettschneider
    email                : <Falk Brettschneider> falk@kdevelop.org
 ***************************************************************************/

/***************************************************************************
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
#include "mdimainfrmcomponent.h"


extern "C" {

    void *init_libkdevqextmdimainfrmview()
    {
        return new MdiMainFrmFactory;
    }
    
};


MdiMainFrmFactory::MdiMainFrmFactory(QObject *parent, const char *name)
    : KLibFactory(parent, name)
{
    instance();
}


MdiMainFrmFactory::~MdiMainFrmFactory()
{
    delete s_instance;
    s_instance = 0;
}


QObject *MdiMainFrmFactory::create(QObject *parent, const char *name,
                                const char *classname, const QStringList &args)
{
    kdDebug(9005) << "running MdiMainFrmFactory::create..." << endl;

    QObject *obj = new MdiMainFrmComponent(parent, name);
    emit objectCreated(obj);
    return obj;
}


KInstance *MdiMainFrmFactory::s_instance = 0;
KInstance *MdiMainFrmFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevqextmdimainfrmview");

    return s_instance;
}
#include "main.moc"

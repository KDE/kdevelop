/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Sun Dec 10 2000
    copyright            : (C) 2000 by Ivan Hawkes
    email                : linuxgroupie@ivanhawkes.com
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
#include "buglist.h"
#include "buglistcomponent.h"


extern "C"
{
    void *init_libkdevbuglist()
    {
        return new BugListFactory;
    }

};


BugListFactory::BugListFactory(QObject *parent, const char *name)
: KLibFactory(parent, name)
{
    instance();
}


BugListFactory::~BugListFactory()
{
    delete s_instance;
    s_instance = 0;
}


QObject *BugListFactory::create(QObject *parent, const char *name,
                             const char */*classname*/, const QStringList &/*args*/)
{
    kdDebug(9001) << "Building BugList" << endl;

    QObject *obj = new BugListComponent(parent, name);
    emit objectCreated(obj);
    return obj;
}


KInstance *BugListFactory::s_instance = 0;
KInstance *BugListFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevbuglist");

    return s_instance;
}

#include "main.moc"

/***************************************************************************
                             editorfactory.cpp
                             -----------------
    copyright            : (C) 2000 by KDevelop team
    email                : kdevelop_team@kdevelop.org

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
#include "editorfactory.h"
#include "editormgr.h"


extern "C" {

    void *init_libkdeveditorpart()
    {
        return new EditorFactory;
    }
    
};


EditorFactory::EditorFactory(QObject *parent, const char *name)
    : KLibFactory(parent, name)
{
    instance();
}


EditorFactory::~EditorFactory()
{
    delete s_instance;
    s_instance = 0;
}


QObject *EditorFactory::create(QObject *parent, const char *name,
                               const char */*classname*/, const QStringList &/*args*/)
{
    kdDebug(9000) << "Building editor part" << endl;
    
    QObject *obj = new EditorManager(parent, name);
    emit objectCreated(obj);
    return obj;
}


KInstance *EditorFactory::s_instance = 0;
KInstance *EditorFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdeveditorpart");

    return s_instance;
}

#include "editorfactory.moc"

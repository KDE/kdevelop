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
#include <kstddirs.h>
#include "doctreeviewfactory.h"
#include "doctreeviewpart.h"


extern "C" {

    void *init_libkdevdoctreeview()
    {
        return new DocTreeViewFactory;
    }
    
};


DocTreeViewFactory::DocTreeViewFactory(QObject *parent, const char *name)
    : KDevFactory(parent, name)
{
}


DocTreeViewFactory::~DocTreeViewFactory()
{
    delete s_instance;
    s_instance = 0;
}


KDevPart *DocTreeViewFactory::createPartObject(KDevApi *api, QObject *parent,
                                               const QStringList &/*args*/)
{
    kdDebug(9002) << "Building DocTreeView" << endl;
    return new DocTreeViewPart(api, parent, "doc tree view part");
}


KInstance *DocTreeViewFactory::s_instance = 0;
KInstance *DocTreeViewFactory::instance()
{
    if (!s_instance) {
        s_instance = new KInstance("kdevdoctreeview");
        KStandardDirs *dirs = s_instance->dirs();
        dirs->addResourceType("docindices", KStandardDirs::kde_default("data") + "kdevdoctreeview/indices/");
        dirs->addResourceType("doctocs", KStandardDirs::kde_default("data") + "kdevdoctreeview/tocs/");
    }

    return s_instance;
}

#include "doctreeviewfactory.moc"

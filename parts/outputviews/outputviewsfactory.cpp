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

#include "outputviewsfactory.h"
#include "makeviewpart.h"
#include "appoutputviewpart.h"


extern "C" {

    void *init_libkdevoutputviews()
    {
        return new OutputViewsFactory;
    }
    
};


OutputViewsFactory::OutputViewsFactory(QObject *parent, const char *name)
    : KDevFactory(parent, name)
{
}


OutputViewsFactory::~OutputViewsFactory()
{
    delete s_instance;
    s_instance = 0;
}


KDevPart *OutputViewsFactory::createPartObject(KDevApi *api, QObject *parent,
                                               const QStringList &args)
{
    if (args.count() > 0 && qstrcmp(args[0].latin1(), "AppOutputView") == 0) {
        kdDebug(9004) << "Building AppOutputView" << endl;
        return new AppOutputViewPart(api, parent, "app output view part");
    } else if (args.count() > 0 && qstrcmp(args[0].latin1(), "MakeView") == 0) {
        kdDebug(9004) << "Building MakeView" << endl;
        return new MakeViewPart(api, parent, "make view part");
    } else {
        kdDebug(9004) << "Wrong args for kdevoutputviews library" << endl;
        if (args.count() > 0)
            kdDebug(9004) << args[0] << endl;
        return 0;
    }
}


KInstance *OutputViewsFactory::s_instance = 0;
KInstance *OutputViewsFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevoutputviews");

    return s_instance;
}

#include "outputviewsfactory.moc"

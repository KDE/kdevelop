/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann,Sandy Meier                      *
 *   bernd@kdevelop.org,smeier@kdevelop.org                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kdebug.h>
#include <kinstance.h>

#include "phpsupportfactory.h"
#include "phpsupportpart.h"


extern "C" {

    void *init_libkdevphpsupport()
    {
        return new PHPSupportFactory;
    }
    
};


PHPSupportFactory::PHPSupportFactory(QObject *parent, const char *name)
    : KDevFactory(parent, name)
{
}


PHPSupportFactory::~PHPSupportFactory()
{
    delete s_instance;
    s_instance = 0;
}


KDevPart *PHPSupportFactory::createPartObject(KDevApi *api, QObject *parent,
                                               const QStringList &/*args*/)
{
    kdDebug(9016) << "Building PHPSupport" << endl;
    return new PHPSupportPart(api, parent, "php support part");
}


KInstance *PHPSupportFactory::s_instance = 0;
KInstance *PHPSupportFactory::instance()
{
    if (!s_instance)
      s_instance = new KInstance("kdevphpsupport");
    
    return s_instance;
}

#include "phpsupportfactory.moc"

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

#include "kdevfactory.h"
#include "kdevpart.h"


KDevFactory::KDevFactory(QObject *parent, const char *name)
    : KLibFactory(parent, name)
{}


KDevFactory::~KDevFactory()
{}


KDevPart *KDevFactory::createPart(KDevApi *api, QObject *parent, const QStringList &args)
{
    KDevPart *part = createPartObject(api, parent, args);
    if (part)
        emit objectCreated(part);
    return part;
}

#include "kdevfactory.moc"

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

#ifndef _FILEVIEWFACTORY_H_
#define _FILEVIEWFACTORY_H_

#include "kdevfactory.h"


class FileViewFactory : public KDevFactory
{
    Q_OBJECT

public:
    FileViewFactory( QObject *parent=0, const char *name=0 );
    ~FileViewFactory();

    virtual KDevPart *createPartObject(KDevApi *api, QObject *parent, const QStringList &args);
    static KInstance *instance();

private:
    static KInstance *s_instance;
};

#endif

/***************************************************************************
 *   Copyright (C) 2000 by Omid Givi                                       *
 *   omid@givi.nl                                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _SOURCEINFOFACTORY_H_
#define _SOURCEINFOFACTORY_H_

#include <klibloader.h>


class SourceInfoFactory : public KLibFactory
{
    Q_OBJECT

public:
    SourceInfoFactory( QObject *parent=0, const char *name=0 );
    ~SourceInfoFactory();

    virtual QObject* create( QObject *parent, const char *name,
                             const char *classname, const QStringList &args);
    static KInstance *instance();

private:
    static KInstance *s_instance;
};

#endif

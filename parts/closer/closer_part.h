/***************************************************************************
 *   Copyright (C) 2003 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __KDEVPART_CLOSER_H__
#define __KDEVPART_CLOSER_H__


#include <kdevplugin.h>
#include <kparts/part.h>
#include <kurl.h>


class CloserPart : public KDevPlugin
{
    Q_OBJECT

public:
    CloserPart(QObject *parent, const char *name, const QStringList &);
    ~CloserPart();

public slots:
    void openDialog();

};


#endif

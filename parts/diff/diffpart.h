/***************************************************************************
 *   Copyright (C) 2001 by Harald Fernengel                                *
 *   harry@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _DIFFPART_H_
#define _DIFFPART_H_

#include "kdevplugin.h"

class DiffPart : public KDevPlugin
{
    Q_OBJECT

public:
    DiffPart( QObject *parent, const char *name, const QStringList & );
    ~DiffPart();

public slots:
    void slotExecDiff();

};

#endif

/***************************************************************************
 *   Copyright (C) 2002 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _FILTERPART_H_
#define _FILTERPART_H_

#include <kdevgenericfactory.h>
#include "kdevplugin.h"

class ShellInsertDialog;
class ShellFilterDialog;


class FilterPart : public KDevPlugin
{
    Q_OBJECT

public:
    FilterPart( QObject *parent, const char *name, const QStringList & );
    ~FilterPart();

public slots:
    void slotShellInsert();
    void slotShellFilter();
    
private:
    ShellInsertDialog *m_insertDialog;
    ShellFilterDialog *m_filterDialog;
};

typedef KDevGenericFactory<FilterPart> FilterFactory;

#endif

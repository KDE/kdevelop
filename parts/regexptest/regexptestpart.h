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

#ifndef _REGEXPTESTPART_H_
#define _REGEXPTESTPART_H_

#include <qguardedptr.h>
#include <kdialogbase.h>
#include "kdevplugin.h"

class RegexpTestDialog;


class RegexpTestPart : public KDevPlugin
{
    Q_OBJECT

public:
    RegexpTestPart( QObject *parent, const char *name, const QStringList & );
    ~RegexpTestPart();

private slots:
    void slotRegexpTest();
    
private:
    RegexpTestDialog *m_dialog;
};

#endif

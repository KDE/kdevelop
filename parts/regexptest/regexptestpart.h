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
#include "kdevpart.h"

class RegexpTestDialog;


class RegexpTestPart : public KDevPart
{
    Q_OBJECT

public:
    RegexpTestPart( KDevApi *api, QObject *parent=0, const char *name=0 );
    ~RegexpTestPart();

private slots:
    void slotRegexpTest();
    
private:
    RegexpTestDialog *m_dialog;
};

#endif

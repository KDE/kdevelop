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

#ifndef _CTAGSPART_H_
#define _CTAGSPART_H_

#include <qguardedptr.h>
#include <kdialogbase.h>
#include "kdevpart.h"

class CTagsDialog;


class CTagsPart : public KDevPart
{
    Q_OBJECT

public:
    CTagsPart( KDevApi *api, QObject *parent=0, const char *name=0 );
    ~CTagsPart();

private slots:
    void slotTags();
    
private:
    CTagsDialog *m_dialog;
};

#endif

/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _CVSPART_H_
#define _CVSPART_H_

class Context;

#include "kdevpart.h"


class CvsPart : public KDevPart
{
    Q_OBJECT

public:
    CvsPart( KDevApi *api, QObject *parent=0, const char *name=0 );
    ~CvsPart();

private slots:
    void contextMenu(QPopupMenu *popup, const Context *context);
    void slotCommit();
    void slotUpdate();
    void slotAdd();
    void slotRemove();

private:
    QString popupfile;
};

#endif

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

#include "kdevplugin.h"

class Context;
class QPopupMenu;


class CvsPart : public KDevPlugin
{
    Q_OBJECT

public:
    CvsPart( QObject *parent, const char *name, const QStringList & );
    ~CvsPart();

private slots:
    void contextMenu(QPopupMenu *popup, const Context *context);
    void slotCommit();
    void slotUpdate();
    void slotAdd();
    void slotRemove();
    void slotLog();

private:
    QString popupfile;
};

#endif

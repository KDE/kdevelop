/***************************************************************************
 *   Copyright (C) 2003 by Roberto Raggi                                   *
 *   roberto@kdevelop.org                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __KDEVPART_FULLSCREEN_H__
#define __KDEVPART_FULLSCREEN_H__

#include <qguardedptr.h>
#include <kdevplugin.h>

class KAction;

class FullScreenPart : public KDevPlugin
{
    Q_OBJECT
public:
    FullScreenPart(QObject *parent, const char *name, const QStringList &);
    virtual ~FullScreenPart();

public slots:
    void slotToggleFullScreen();

private:
    KToggleFullScreenAction* m_pFullScreen;
    bool m_bFullScreen;
};


#endif

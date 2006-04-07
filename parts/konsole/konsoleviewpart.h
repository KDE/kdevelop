/***************************************************************************
 *   Copyright (C) 2003 by KDevelop Authors                                *
 *   kdevelop-devel@kde.org                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __KONSOLEVIEWPART_H__
#define __KONSOLEVIEWPART_H__


#include <qguardedptr.h>
#include "kdevplugin.h"


class KDevShellWidget;

class KonsoleViewPart : public KDevPlugin
{
    Q_OBJECT
public:
    KonsoleViewPart(QObject *parent, const char *name, const QStringList &);
    virtual ~KonsoleViewPart();

private:
    QGuardedPtr<KDevShellWidget> m_widget;
};

#endif

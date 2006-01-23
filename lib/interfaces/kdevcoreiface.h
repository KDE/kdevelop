/* This file is part of the KDE project
   Copyright (C) 2001 Bernd Gehrmann <bernd@kdevelop.org>
   Copyright (C) 2001-2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2002 Roberto Raggi <roberto@kdevelop.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef _KDEVCOREIFACE_H_
#define _KDEVCOREIFACE_H_

#include <dcopobject.h>
#include <dcopref.h>

class KDevCore;


class KDevCoreIface : public QObject, public DCOPObject
{
    Q_OBJECT
    K_DCOP
    
public:
    
    KDevCoreIface( KDevCore *core );
    ~KDevCoreIface();
    
k_dcop:
    void openProject(const QString& projectFileName);

private slots:
    void forwardProjectOpened();
    void forwardProjectClosed();
    
private:
    KDevCore *m_core;
};

#endif

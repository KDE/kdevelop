/* This file is part of the KDE project
   Copyright (C) 2001 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2002 Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2002 Bernd Gehrmann <bernd@kdevelop.org>
   Copyright (C) 2003 Amilcar do Carmo Lucas <amilcar@ida.ing.tu-bs.de>

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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef _KDEVAPPFRONTENDIFACE_H_
#define _KDEVAPPFRONTENDIFACE_H_

#include <dcopobject.h>

class KDevAppFrontend;

class KDevAppFrontendIface : public DCOPObject
{
    K_DCOP
    
public:
    
    KDevAppFrontendIface( KDevAppFrontend *appFrontend );
    ~KDevAppFrontendIface();

k_dcop:
    void startAppCommand(const QString &directory, const QString &command, bool inTerminal);
    void stopApplication();
    bool isRunning();
    void clearView();
    void insertStderrLine(const QString &line);
    void insertStdoutLine(const QString &line);
    
private:
    KDevAppFrontend *m_appFrontend;
};

#endif

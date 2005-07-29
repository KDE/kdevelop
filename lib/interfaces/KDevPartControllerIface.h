/* This file is part of the KDE project
   Copyright (C) 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>

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
#ifndef _KDEVPARTCONTROLLERIFACE_H_
#define _KDEVPARTCONTROLLERIFACE_H_

#include <dcopobject.h>
#include <dcopref.h>
#include <kurl.h>

class KDevPartController;

class KDevPartControllerIface : public QObject, public DCOPObject
{
  Q_OBJECT
  K_DCOP
    
public:
    
  KDevPartControllerIface(KDevPartController *pc);
  ~KDevPartControllerIface();


k_dcop:

  void editDocument(const QString &url, int lineNum);
  void showDocument(const QString &url, bool newWin);

  void saveAllFiles();
  void revertAllFiles();

  bool closeAllFiles();
  uint documentState( const KURL & url );

private slots:
	
  void forwardLoadedFile(const KURL &fileName);
  void forwardSavedFile(const KURL &fileName);
  void forwardClosedFile(const KURL &fileName);  

private:
  
  KDevPartController *m_controller;
    
};


#endif

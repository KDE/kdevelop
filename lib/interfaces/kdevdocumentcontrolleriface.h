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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef _KDEVDOCUMENTCONTROLLERIFACE_H_
#define _KDEVDOCUMENTCONTROLLERIFACE_H_

#include <dcopobject.h>
#include <dcopref.h>
#include <kurl.h>

class KDevDocumentController;

class KDevDocumentControllerIface : public QObject, public DCOPObject
{
  Q_OBJECT
  K_DCOP

public:

  KDevDocumentControllerIface(KDevDocumentController *pc);
  ~KDevDocumentControllerIface();


k_dcop:

  void editDocument(const QString &url, int lineNum);
  void showDocument(const QString &url, bool newWin);

  void saveAllDocuments();
  void reloadAllDocuments();

  bool closeAllDocuments();
  uint documentState( const KUrl &url );

private slots:

  void forwardLoadedDocument(const KUrl &url);
  void forwardSavedDocument(const KUrl &url);
  void forwardClosedDocument(const KUrl &url);

private:

  KDevDocumentController *m_controller;

};

#endif

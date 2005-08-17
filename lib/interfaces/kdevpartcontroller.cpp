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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#include "KDevPartControllerIface.h"
#include "kdevpartcontroller.h"

#ifndef NO_KATE_DOCUMENTMANAGER
#include "katedocumentmanagerinterface.h"
#endif

KDevPartController::KDevPartController(QWidget *parent)
  : KParts::PartManager(parent)
{
  new KDevPartControllerIface(this);
#ifndef NO_KATE_DOCUMENTMANAGER
  new KateDocumentManagerInterface(this);
#endif
}

#include "kdevpartcontroller.moc"

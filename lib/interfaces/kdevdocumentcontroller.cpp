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
#include "kdevdocumentcontroller.h"
#include <dbus/qdbus.h>

KDevDocumentController::KDevDocumentController(QWidget *parent)
  : KParts::PartManager(parent)
{
    QDBus::sessionBus().registerObject("/org/kdevelop/DocumentController",
                                       this, QDBusConnection::ExportSlots);
}

KUrl KDevDocumentController::activeDocumentUrl( ) const
{
    if (activeDocument())
        return activeDocument()->url();

    return KUrl();
}

KParts::Part * KDevDocumentController::partForUrl( const KUrl & url ) const
{
    if (KDevDocument* file = documentForUrl(url))
        return file->part();

    return 0L;
}

KDevDocument * KDevDocumentController::documentForUrl( const KUrl & url ) const
{
    foreach (KDevDocument* document, openDocuments())
        if (document->url() == url)
            return document;

    return 0L;
}

#include "kdevdocumentcontroller.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on

/*  This file is part of KDevelop
    Copyright 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

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

#ifndef KDEVELOPSESSIONSOBSERVER_H
#define KDEVELOPSESSIONSOBSERVER_H

// library
#include <kdevelopsessionswatch_export.h>
#include "kdevelopsessiondata.h"
// Qt
#include <QObject>
#include <QVector>

// The interface observers of list of existing KDevelop sessions should implement
class KDEVELOPSESSIONSWATCH_EXPORT KDevelopSessionsObserver
{
public:
    virtual ~KDevelopSessionsObserver();

public:
    /// Implement as a slot, as it will be called via QMetaObject::invokeMethod by a queued connection
    /// which ensures the update arrives in the event loop of the thread the object belongs to
    /// so the data update of the working copy is not e.g. happening during a paint operation
    /// @param sessionDataList sorted by id
    virtual void setSessionDataList(const QVector<KDevelopSessionData>& sessionDataList) = 0;
};

Q_DECLARE_INTERFACE(KDevelopSessionsObserver, "org.kdevelop.KDevelopSessionsObserver")

#endif

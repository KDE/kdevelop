/*
    SPDX-FileCopyrightText: 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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

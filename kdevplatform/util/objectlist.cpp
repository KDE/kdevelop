/*
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "objectlist.h"

using namespace KDevelop;

class KDevelop::ObjectListTrackerPrivate
{
public:
    /// List of tracked objects (contains unique items only)
    QList<QObject*> m_list;
    ObjectListTracker::CleanupBehavior m_cleanupBehavior;
};

ObjectListTracker::ObjectListTracker(CleanupBehavior behavior, QObject* parent)
    : QObject(parent)
    , d_ptr(new ObjectListTrackerPrivate {{}, behavior})
{
}

ObjectListTracker::~ObjectListTracker()
{
    Q_D(ObjectListTracker);

    if (d->m_cleanupBehavior == CleanupWhenDone) {
        deleteAll();
    }
}

const QList<QObject*>& ObjectListTracker::data() const
{
    Q_D(const ObjectListTracker);

    return d->m_list;
}

void ObjectListTracker::objectDestroyed(QObject* object)
{
    Q_D(ObjectListTracker);

    bool success = d->m_list.removeOne(object);
    Q_ASSERT(success);
    Q_UNUSED(success);
}

void ObjectListTracker::append(QObject* object)
{
    Q_D(ObjectListTracker);

    if (!object || d->m_list.contains(object)) {
        return;
    }

    d->m_list.append(object);
    connect(object, &QObject::destroyed, this, &ObjectListTracker::objectDestroyed);
}

bool ObjectListTracker::remove(QObject* object)
{
    Q_D(ObjectListTracker);

    if (!object) {
        return false;
    }

    disconnect(object, &QObject::destroyed, this, &ObjectListTracker::objectDestroyed);
    return d->m_list.removeOne(object);
}

void ObjectListTracker::deleteAll()
{
    Q_D(ObjectListTracker);

    qDeleteAll(d->m_list);
    d->m_list.clear();
}

#include "moc_objectlist.cpp"

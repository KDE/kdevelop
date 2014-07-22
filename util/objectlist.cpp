/*
 * Copyright 2014 Kevin Funk <kfunk@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "objectlist.h"

using namespace KDevelop;

struct ObjectListTracker::Private
{
    /// List of tracked objects (contains unique items only)
    QList<QObject*> m_list;
    CleanupBehavior m_cleanupBehavior;

    void objectDestroyed(QObject* object);
};

ObjectListTracker::ObjectListTracker(CleanupBehavior behavior, QObject* parent)
    : QObject(parent)
    , d(new Private{{}, behavior})
{
}

ObjectListTracker::~ObjectListTracker()
{
    if (d->m_cleanupBehavior == CleanupWhenDone) {
        deleteAll();
    }
}

const QList<QObject*>& ObjectListTracker::data() const
{
    return d->m_list;
}

void ObjectListTracker::Private::objectDestroyed(QObject* object)
{
    bool success = m_list.removeOne(object);
    Q_ASSERT(success);
    Q_UNUSED(success);
}

void ObjectListTracker::append(QObject* object)
{
    if (!object || d->m_list.contains(object)) {
        return;
    }

    d->m_list.append(object);
    connect(object, SIGNAL(destroyed(QObject*)), this, SLOT(objectDestroyed(QObject*)));
}

bool ObjectListTracker::remove(QObject* object)
{
    if (!object) {
        return false;
    }

    disconnect(object, SIGNAL(destroyed(QObject *)), this, SLOT(objectDestroyed(QObject*)));
    return d->m_list.removeOne(object);
}

void ObjectListTracker::deleteAll()
{
    qDeleteAll(d->m_list);
    d->m_list.clear();
}

#include "objectlist.moc"

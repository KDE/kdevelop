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

#ifndef KDEVPLATFORM_OBJECTLIST_H
#define KDEVPLATFORM_OBJECTLIST_H

#include <QObject>
#include <QList>

#include "utilexport.h"

namespace KDevelop {

/**
 * @brief Class for tracking a set of alive objects inheriting from QObject.
 *
 * Add individual objects via the append() method to have their lifetime being monitored by this class.
 * In case one of the tracked objects is destroyed, it is removed from the list.
 *
 * This means this class always contains a set of valid pointers to QObject instances.
 * The up-to-date list can be access via the data() method.
 *
 * @note You are *not* being notified if an item is removed from the list.
 * The purpose of this class is to provide a simple mechanism to keep track of a set of *alive* objects
 *
 * @sa append()
 * @sa data()
 */
class KDEVPLATFORMUTIL_EXPORT ObjectListTracker : public QObject
{
    Q_OBJECT

public:
    enum CleanupBehavior {
        NoCleanupWhenDone,  ///< Do nothing when this object is destructed
        CleanupWhenDone     ///< Delete list of items when this object is destructed
    };

    ObjectListTracker(CleanupBehavior behavior = NoCleanupWhenDone, QObject* parent = 0);
    virtual ~ObjectListTracker();

    /**
     * Append and track object @p object
     *
     * In case @p object is destroyed, it gets removed from the list
     *
     * @note If @p object is already tracked, this operation is a no-op
     */
    void append(QObject* object);
    /**
     * Remove and no longer track object @p object
     *
     * @return False if object @p object wasn't tracked or null, otherwise true
     */
    bool remove(QObject* object);
    /**
     * Delete all objects currently tracked and clears the list
     */
    void deleteAll();

    /**
     * Accessor towards to the internal list of currently tracked objects
     */
    const QList<QObject*>& data() const;

private:
    void objectDestroyed(QObject*);

private:
    struct Private;
    QScopedPointer<Private> const d;
};

/**
 * @brief Template-based wrapper around ObjectListTracker for tracking a set of objects inheriting from QObject
 *
 * Provides a type-safe way to access and mutate ObjectListTracker
 *
 * @sa KDevelop::ObjectListTracker
 */
template<class T>
class ObjectList
{
public:
    ObjectList(ObjectListTracker::CleanupBehavior behavior = ObjectListTracker::NoCleanupWhenDone)
        : m_tracker(behavior)
    {
    };

    void append(T* object)
    {
        m_tracker.append(object);
    }
    bool remove(T* object)
    {
        return m_tracker.remove(object);
    }
    void deleteAll()
    {
        m_tracker.deleteAll();
    }

    /**
     * Accessor to the up-to-date list inside the object tracker
     */
    QList<T*> data() const
    {
        // This is considered safe, as QList<X*> and QList<Y*> have the same memory layout
        // also see http://comments.gmane.org/gmane.comp.lib.qt.general/38943
        return *reinterpret_cast<const QList<T*>*>(&m_tracker.data());
    }

private:
    ObjectListTracker m_tracker;
};

}

#endif // KDEVPLATFORM_OBJECTLIST_H

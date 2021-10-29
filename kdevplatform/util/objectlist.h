/*
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KDEVPLATFORM_OBJECTLIST_H
#define KDEVPLATFORM_OBJECTLIST_H

#include <QObject>
#include <QList>

#include "utilexport.h"

namespace KDevelop {
class ObjectListTrackerPrivate;

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

    explicit ObjectListTracker(CleanupBehavior behavior = NoCleanupWhenDone, QObject* parent = nullptr);
    ~ObjectListTracker() override;

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
    const QScopedPointer<class ObjectListTrackerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(ObjectListTracker)
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
    explicit ObjectList(ObjectListTracker::CleanupBehavior behavior = ObjectListTracker::NoCleanupWhenDone)
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

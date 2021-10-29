/*
    SPDX-FileCopyrightText: 2009 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_KSHAREDOBJECT_H
#define KDEVPLATFORM_KSHAREDOBJECT_H

#include <QSharedData>
#include <QObject>

namespace KDevelop {

struct FakeAtomic
{
    inline FakeAtomic(QObject& object, QSharedData& real) : m_object(object)
        , m_real(real)
    {
    }
    inline operator int() const
    {
        const int value = m_real.ref.loadAcquire();
        if (value == 0)
            return 1; //Always return true, because we handle the deleting by ourself using deleteLater
        return value;
    }

    inline bool ref()
    {
        return m_real.ref.ref();
    }

    inline bool deref()
    {
        bool ret = m_real.ref.deref();
        if (!ret)
            m_object.deleteLater();

        return true; //Always return true, because we handle the deleting by ourself
    }

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    inline int loadRelaxed() const
    {
        return m_real.ref.loadRelaxed();
    }
#else
    inline int load() const
    {
        return m_real.ref.load();
    }
#endif

    QObject& m_object;
    QSharedData& m_real;
};

/**
 * Wrapper around QSharedData for use with KSharedPtr when the object is based on QObject as well.
 * Instead of deleting the object once the reference-count reaches zero, QObject::deleteLater() is called.
 * This prevents a possible crash when the reference-count reaches zero during event-processing while the queue
 * contains events to be delivered to that object.
 *
 * Notice however that the object will not be deleted immediately, which may lead to unintended behavior.
 */
struct KSharedObject : public QSharedData
{
    inline explicit KSharedObject(QObject& object) : ref(object, *this)
    {
    }

    mutable FakeAtomic ref;
};

}

#endif

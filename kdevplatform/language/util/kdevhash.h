/*
    SPDX-FileCopyrightText: 2012 Olivier de Gaalon <olivier.jg@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_KDEVHASH_H
#define KDEVPLATFORM_KDEVHASH_H

#include <QHash>

/**
 * A helper class to implement hashing for aggregate types.
 *
 * @code
 * class MyType {
 *     ...
 *     uint hash() const
 *     {
 *         KDevHash hash;
 *         return hash << m_1 << m_2 << ...;
 *     }
 * };
 * @endcode
 */
class KDevHash
{
public:
    enum {
        DEFAULT_SEED = 2166136261u
    };

    explicit KDevHash(uint hash = DEFAULT_SEED)
        : m_hash(hash)
    {}

    KDevHash(const KDevHash&) = delete;
    KDevHash& operator=(const KDevHash&) = delete;

    inline operator uint() const
    {
        return m_hash;
    }

    template <typename T>
    inline KDevHash& operator<<(T value)
    {
        m_hash = hash_combine(m_hash, qHash(value));
        return *this;
    }

    static inline uint hash_combine(uint seed, uint hash)
    {
        // this is copied from boost::hash_combine
        return seed ^ (hash + 0x9e3779b9 + (seed << 6) + (seed >> 2));
    }

private:
    uint m_hash;
};

#endif //KDEVPLATFORM_KDEVHASH_H

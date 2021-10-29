/*
    SPDX-FileCopyrightText: 2009 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2020 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_REFERENCECOUNTING_H
#define KDEVPLATFORM_REFERENCECOUNTING_H

#include "serializationexport.h"

#include <QtGlobal>

#include <cstddef>

//When this is enabled, the duchain unloading is disabled as well, and you should start
//with a cleared ~/.kdevduchain
// #define TEST_REFERENCE_COUNTING

namespace KDevelop {
///Since shouldDoDUChainReferenceCounting is called extremely often, we export some internals into the header here,
///so the reference-counting code can be inlined.
class KDEVPLATFORMSERIALIZATION_EXPORT DUChainReferenceCounting
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
    Q_DISABLE_COPY_MOVE(DUChainReferenceCounting)
#else
    Q_DISABLE_COPY(DUChainReferenceCounting)
#endif
public:
    using Pointer = const std::byte*;

    bool shouldDo(Pointer item) const noexcept;
    void enable(Pointer start, unsigned size);
    void disable(Pointer start, unsigned size);

    static DUChainReferenceCounting& instance() noexcept
    {
        static thread_local DUChainReferenceCounting duchainReferenceCounting;
        return duchainReferenceCounting;
    }

private:
    // This defaulted default constructor is implicitly noexcept. Marking it as noexcept explicitly, however,
    // doesn't compile with Clang version < 9.0 because of https://bugs.llvm.org/show_bug.cgi?id=33736.
    constexpr DUChainReferenceCounting() = default;

    struct Interval {
        Pointer start;
        unsigned size;
        unsigned refCount;

        constexpr bool contains(Pointer item) const noexcept { return item >= start && item < start + size; }
        void assign(Pointer newStart, unsigned newSize) noexcept;
    };

    Interval* findInterval(Pointer start, unsigned size) noexcept;

    // I have never encountered more than 2 intervals at a time during my tests.
    // So the maximum interval count of 3 should be more than enough for every practical use.
    static constexpr std::size_t maxIntervalCount = 3;

    std::size_t count = 0;
    Interval intervals[maxIntervalCount] = {};
};

inline bool DUChainReferenceCounting::shouldDo(Pointer item) const noexcept
{
    // count == 0 means that no place has been marked for reference counting, occurs in ~99% of cases.
    // Q_UNLIKELY somewhat speeds up BenchIndexedString::bench_qhashIndexedString(),
    // slightly speeds up BenchIndexedString::bench_create() and BenchIndexedString::bench_destroy()
    // but substantially slows down BenchItemRepository::shouldDoReferenceCounting(enabled).
    // However, while the three affected BenchIndexedString benchmarks are more or less realistic,
    // BenchItemRepository::shouldDoReferenceCounting(enabled) is highly synthetic and extremely
    // sensitive to the performance of this function.
    for (std::size_t i = 0; Q_UNLIKELY(i != count); ++i) {
        if (intervals[i].contains(item)) {
            return true;
        }
    }
    return false;
}

KDEVPLATFORMSERIALIZATION_EXPORT void initReferenceCounting();

///This is used by indexed items to decide whether they should do reference-counting
inline bool shouldDoDUChainReferenceCounting(const void* item) noexcept
{
    return DUChainReferenceCounting::instance().shouldDo(reinterpret_cast<DUChainReferenceCounting::Pointer>(item));
}

///Enable reference-counting for the given range
///You should only enable the reference-counting for the time it's really needed,
///and it always has to be enabled too when the items are deleted again, else
///it will lead to inconsistencies in the repository.
///@warning If you are not working on the duchain internal storage mechanism, you should
///not care about this stuff at all.
///@param start Position where to start the reference-counting
///@param size Size of the area in bytes
KDEVPLATFORMSERIALIZATION_EXPORT void enableDUChainReferenceCounting(const void* start, unsigned size);
///Must be called as often as enableDUChainReferenceCounting, with the same ranges
///Must never be called for the same range twice, and not for overlapping ranges
///@param start Position where the reference-counting was started
///@param size Size of the area where the reference-counting was started in bytes
KDEVPLATFORMSERIALIZATION_EXPORT void disableDUChainReferenceCounting(const void* start, unsigned size);

class DUChainReferenceCountingEnabler
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
    Q_DISABLE_COPY_MOVE(DUChainReferenceCountingEnabler)
#else
    Q_DISABLE_COPY(DUChainReferenceCountingEnabler)
#endif
public:
    explicit DUChainReferenceCountingEnabler(const void* start, unsigned size)
        : m_start{start}
        , m_size{size}
    {
        enableDUChainReferenceCounting(m_start, m_size);
    }

    ~DUChainReferenceCountingEnabler()
    {
        disableDUChainReferenceCounting(m_start, m_size);
    }

private:
    const void* const m_start;
    const unsigned m_size;
};

template <bool markForReferenceCounting>
struct OptionalDUChainReferenceCountingEnabler
{
    explicit OptionalDUChainReferenceCountingEnabler(const void*, unsigned) {}
};

template<>
struct OptionalDUChainReferenceCountingEnabler<true> : DUChainReferenceCountingEnabler
{
    using DUChainReferenceCountingEnabler::DUChainReferenceCountingEnabler;
};

///Use this as local variable within the object that maintains the reference-count,
///and use
struct ReferenceCountManager
{
    #ifndef TEST_REFERENCE_COUNTING
    inline void increase(uint& ref, uint /*targetId*/)
    {
        ++ref;
    }
    inline void decrease(uint& ref, uint /*targetId*/)
    {
        Q_ASSERT(ref);
        --ref;
    }

    #else

    ReferenceCountManager();
    ~ReferenceCountManager();

    ReferenceCountManager(const ReferenceCountManager& rhs);
    ReferenceCountManager& operator=(const ReferenceCountManager& rhs);

    void increase(uint& ref, uint targetId);
    void decrease(uint& ref, uint targetId);

//     bool hasReferenceCount() const;

    uint m_id;
    #endif
};
}

#endif

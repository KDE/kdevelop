/*
    SPDX-FileCopyrightText: 2026 Jarmo Tiitto <jarmo.tiitto@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/
#ifndef KDEVPLATFORM_UTIL_HASHVALUE_H
#define KDEVPLATFORM_UTIL_HASHVALUE_H

#include "utilexport.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <type_traits>

namespace KDevelop {

/**
 * Compute a 32-bit hash value for a object or an array of such.
 *
 * @brief The provided functionality in this header is usually enough. If necessary, a custom
 * HashValue can be derived from this class to implement more functionality. Such a derived type value
 * shall immediately be converted to the base HashValue.
 */
class HashValue
{
protected:
    std::uint32_t m_hash;

    static constexpr std::uint32_t InitialSeed = 0x87c37b91;

    /**
     * The primitive hash function, without finalization.
     * @param data pointer to data of length @p size.
     * @param size length of data. If zero, @p data is ignored.
     * @param seed return value of this function or InitialSeed.
     */
    static std::uint32_t KDEVPLATFORMUTIL_EXPORT hashProcess(const void* data, std::size_t size,
                                                             std::uint32_t seed) noexcept;

    /**
     * Finalize a hash.
     */
    static constexpr std::uint32_t hashFinalize(std::uint32_t seed, std::size_t size) noexcept
    {
        // Murmur3 32-Bit hash fmix32 finalization part.
        seed ^= size;
        seed ^= seed >> 16;
        seed *= 0x85ebca6b;
        seed ^= seed >> 13;
        seed *= 0xc2b2ae35;
        seed ^= seed >> 16;
        return seed;
    }

    /**
     * One-shot hashFinalize(hashProcess(..., size), size)
     */
    static std::uint32_t KDEVPLATFORMUTIL_EXPORT hashRange(const void* data, std::size_t size) noexcept;

    /**
     * Mix hash values.
     * @param seed any value
     * @param hash any value.
     * @return A well distributed hash value.
     */
    static constexpr std::uint32_t hashCombine(std::uint32_t seed, std::uint32_t hash) noexcept
    {
        // Based on boost::hash_combine() v1.90
        constexpr std::uint32_t m1 = 0x21f0aaad;
        constexpr std::uint32_t m2 = 0xf35a2d97;
        // This magic number m0 is pow(2,32) / ((1 + sqrt(5)) / 2). (reciprocal of the golden ratio)
        constexpr std::uint32_t m0 = 0x9e3779b9;
        seed += hash + m0;
        seed ^= seed >> 16;
        seed *= m1;
        seed ^= seed >> 15;
        seed *= m2;
        seed ^= seed >> 15;
        return seed;
    }

    struct tag_t
    {
    };
    static constexpr auto tag = tag_t{};
    /**
     * Delegate constructor for derived classes.
     */
    constexpr HashValue(const tag_t&, std::uint32_t hash)
        : m_hash(hash)
    {
    }

    /**
     * Check if type T can be hashed safely:
     * 1. Cannot be a HashValue or pointer type.
     * 2. Must satisfy std::is_standard_layout<>.
     * 3. Must satisfy std::has_unique_object_representations<> (C++17)
     */
    template<typename T>
    struct hashable_t
        : std::conjunction<std::negation<std::disjunction<std::is_pointer<T>, std::is_same<T, HashValue>>>,
                           std::is_standard_layout<T>, std::has_unique_object_representations<T>>
    {
    };

    template<typename... Args>
    static constexpr bool is_hashable_v = std::conjunction_v<hashable_t<std::decay_t<Args>>...>;

    template<typename... Args>
    using if_hashable = std::enable_if_t<is_hashable_v<Args...>, bool>;

public:
    // HashValue is never "empty".
    HashValue() = delete;

    template<typename T, if_hashable<T> = true>
    HashValue(const T* data, std::size_t length) noexcept
        : m_hash(hashRange(data, length * sizeof(T)))
    {
    }

    template<typename T, std::size_t extent, std::enable_if_t<extent && is_hashable_v<T>, bool> = true>
    HashValue(const T (&data)[extent]) noexcept
        : HashValue(data, extent)
    {
    }

    /**
     * Implicit conversion from any derived HashValue type.
     */
    template<typename T, std::enable_if_t<std::is_base_of_v<HashValue, T>, bool> = true>
    constexpr HashValue(T&& hash) noexcept
        : m_hash(hash)
    {
    }

    template<typename T>
    constexpr std::enable_if_t<std::is_base_of_v<HashValue, T>, HashValue&> operator=(const T& hash) noexcept
    {
        m_hash = hash;
        return *this;
    }

    /**
     * Compute a initial hash value for further combining.
     */
    template<typename T, if_hashable<T> = true>
    explicit constexpr HashValue(T&& value, tag_t* = nullptr) noexcept
        : m_hash(HashValue(tag, 0) << value)
    {
        // (This ctor would conflict without tag_t with the earlier, which cannot have it)
    }

    /**
     * Combine hashes.
     */
    constexpr HashValue& operator<<(HashValue hash) noexcept
    {
        m_hash = hashCombine(m_hash, hash.m_hash);
        return *this;
    }

    /**
     * Compute a hash value and combine.
     */
    template<typename T, if_hashable<T> = true>
    constexpr HashValue& operator<<(T&& value) noexcept
    {
        // Murmur3 is too slow below 12 bytes.
        if constexpr (sizeof(T) <= 4) {
            std::uint32_t x = 0;
            std::memcpy(&x, &value, sizeof(T));
            m_hash = hashCombine(m_hash, x);
        } else if constexpr (sizeof(T) <= 8) {
            std::uint32_t x[2] = {0, 0};
            std::memcpy(x, &value, sizeof(T));
            m_hash = hashCombine(hashCombine(m_hash, x[0]), x[1]);
        } else if constexpr (sizeof(T) <= 12) {
            std::uint32_t x[3] = {0, 0, 0};
            std::memcpy(x, &value, sizeof(T));
            m_hash = hashCombine(hashCombine(hashCombine(m_hash, x[0]), x[1]), x[2]);
        } else {
            // Murmur3's finalization is not done here because hashCombine() distributes the
            // bits equally well, if not better, and doesn't care the hash came from Murmur3.
            m_hash = hashCombine(m_hash, hashProcess(&value, sizeof(T), InitialSeed) ^ sizeof(T));
        }
        return *this;
    }

    /**
     * Combine multiple HashValues.
     */
    explicit constexpr HashValue(const std::initializer_list<HashValue> hashvalues) noexcept
        : m_hash(*hashvalues.begin())
    {
        for (auto h = hashvalues.begin() + 1; h != hashvalues.end(); ++h) {
            m_hash = hashCombine(m_hash, h->m_hash);
        }
    }

    /**
     * Convert to the integer type.
     */
    constexpr operator std::uint32_t() const noexcept
    {
        return m_hash;
    }
};

}

#endif

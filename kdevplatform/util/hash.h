/*
    SPDX-FileCopyrightText: 2026 Jarmo Tiitto <jarmo.tiitto@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/
#ifndef KDEVPLATFORM_UTIL_HASHVALUE_H
#define KDEVPLATFORM_UTIL_HASHVALUE_H

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace KDevelop {

class HashValue
{
    std::uint32_t m_hash;

    /**
     * The general hash function.
     * @param data pointer to data of length @p size.
     * @param size length of data. If zero, @p data is ignored.
     */
    static constexpr std::uint32_t hashRange(const void* data, std::size_t size) noexcept
    {
        // DJB2 hash:
        std::uint32_t hash = 5381u;
        auto bytes = static_cast<const char*>(data);
        for (std::size_t i = 0; i < size; ++i) {
            hash = ((hash << 5) + hash) + *bytes++; /* hash * 33 + c */
        }
        return hash;
    }

public:
    HashValue() = delete;

    /**
     * Compute a hash value for a object or an array of such. T must satisfy
     * std::is_standard_layout_v<T>.
     */
    template<typename T, typename = std::enable_if_t<std::is_standard_layout_v<T>>>
    HashValue(const T* data, std::size_t length) noexcept
        : m_hash(hashRange(data, length * sizeof(T)))
    {
    }

    template<typename T, std::size_t extent, typename = std::enable_if_t<std::is_standard_layout_v<T>>>
    HashValue(const T (&data)[extent]) noexcept
        : HashValue(data, extent)
    {
    }

    [[nodiscard]] operator std::uint32_t() const noexcept
    {
        return m_hash;
    }
};

}

#endif

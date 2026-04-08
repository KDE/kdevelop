/*
    SPDX-FileCopyrightText: 2026 Jarmo Tiitto <jarmo.tiitto@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/
#include "hash.h"

namespace KDevelop {

std::uint32_t HashValue::hashProcess(const void* data, std::size_t size, std::uint32_t seed) noexcept
{
    // Murmur3 32-Bit hash with finalization part split to hashFinalize()
    // Heavily adapted from the smhasher/MurmurHash3.cpp reference implementation.
    const std::uint32_t c1 = 0xcc9e2d51;
    const std::uint32_t c2 = 0x1b873593;

    auto rotl32 = [](std::uint32_t x, std::int8_t r) {
        return (x << r) | (x >> (32 - r));
    };

    const int nblocks = size / 4;
    auto blocks = static_cast<const std::uint32_t*>(data) + nblocks;
    for (int i = -nblocks; i; i++) {
        auto k1 = blocks[i];
        k1 *= c1;
        k1 = rotl32(k1, 15);
        k1 *= c2;

        seed ^= k1;
        seed = rotl32(seed, 13);
        seed = seed * 5 + 0xe6546b64;
    }

    auto tail = static_cast<const std::uint8_t*>(data) + nblocks * 4;
    std::uint32_t k1 = 0;
    switch (size & 3) {
    case 3:
        k1 ^= tail[2] << 16;
        [[fallthrough]];
    case 2:
        k1 ^= tail[1] << 8;
        [[fallthrough]];
    case 1:
        k1 ^= tail[0];
        k1 *= c1;
        k1 = rotl32(k1, 15);
        k1 *= c2;
        seed ^= k1;
    };
    return seed;
}

std::uint32_t HashValue::hashRange(const void* data, std::size_t size) noexcept
{
    // Murmur3 32-Bit hash, with finalization.
    return hashFinalize(hashProcess(data, size, InitialSeed), size);
}
}

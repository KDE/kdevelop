/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_STRINGREPOSITORY_H
#define KDEVPLATFORM_STRINGREPOSITORY_H

#include <QString>

#include "itemrepository.h"
#include "indexedstring.h"

namespace Repositories {
using namespace KDevelop;

struct StringData
{
    unsigned short length;

    StringData& operator=(const StringData& rhs) = delete;

    unsigned int itemSize() const
    {
        return sizeof(StringData) + length;
    }
    unsigned int hash() const
    {
        IndexedString::RunningHash running;
        const char* str = reinterpret_cast<const char*>(this) + sizeof(StringData);
        for (int a = length - 1; a >= 0; --a) {
            running.append(*str);
            ++str;
        }

        return running.hash;
    }
};

struct StringRepositoryItemRequest
{
    //The text is supposed to be utf8 encoded
    StringRepositoryItemRequest(const char* text, unsigned int hash, unsigned short length) : m_hash(hash)
        , m_length(length)
        , m_text(text)
    {
    }

    enum {
        AverageSize = 10 //This should be the approximate average size of an Item
    };

    using HashType = unsigned int;

    //Should return the hash-value associated with this request(For example the hash of a string)
    HashType hash() const
    {
        return m_hash;
    }

    //Should return the size of an item created with createItem
    uint itemSize() const
    {
        return sizeof(StringData) + m_length;
    }
    //Should create an item where the information of the requested item is permanently stored. The pointer
    //@param item equals an allocated range with the size of itemSize().
    void createItem(StringData* item) const
    {
        item->length = m_length;
        void* itemText = reinterpret_cast<void*>(item + 1);
        memcpy(itemText, m_text, m_length);
    }

    static void destroy(StringData*, KDevelop::AbstractItemRepository&)
    {
    }

    static bool persistent(const StringData*)
    {
        //Reference-counting not supported in the normal string repository
        return true;
    }

    //Should return whether the here requested item equals the given item
    bool equals(const StringData* item) const
    {
        return item->length == m_length && (memcmp(++item, m_text, m_length) == 0);
    }
    unsigned int m_hash;
    unsigned short m_length;
    const char* m_text;
};

using StringRepository = ItemRepository<StringData, StringRepositoryItemRequest, false>;

///@param item must be valid(nonzero)
inline QString stringFromItem(const StringData* item)
{
    const unsigned short* textPos = ( unsigned short* )(item + 1);
    return QString::fromUtf8(reinterpret_cast<const char*>(textPos), item->length);
}

inline QByteArray arrayFromItem(const StringData* item)
{
    const unsigned short* textPos = ( unsigned short* )(item + 1);
    return QByteArray(reinterpret_cast<const char*>(textPos), item->length);
}
}
#endif

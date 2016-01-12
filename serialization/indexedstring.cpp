/* This file is part of KDevelop

   Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>
   Copyright 2016 Milian Wolff <mail@milianw.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "indexedstring.h"
#include "serialization/stringrepository.h"

#include "referencecounting.h"

using namespace KDevelop;

namespace {

struct IndexedStringData
{
    unsigned short length;
    uint refCount;

    uint itemSize() const
    {
        return sizeof(IndexedStringData) + length;
    }

    uint hash() const
    {
        IndexedString::RunningHash running;
        const char* str = ((const char*)this) + sizeof(IndexedStringData);
        for (int a = length - 1; a >= 0; --a) {
            running.append(*str);
            ++str;
        }
        return running.hash;
    }
};

inline void increase(uint& val)
{
    ++val;
}

inline void decrease(uint& val)
{
    --val;
}

struct IndexedStringRepositoryItemRequest
{
    //The text is supposed to be utf8 encoded
    IndexedStringRepositoryItemRequest(const char* text, uint hash, unsigned short length)
        : m_hash(hash)
        , m_length(length)
        , m_text(text)
    {
    }

    enum {
        AverageSize = 10 //This should be the approximate average size of an Item
    };

    typedef uint HashType;

    //Should return the hash-value associated with this request(For example the hash of a string)
    HashType hash() const
    {
        return m_hash;
    }

    //Should return the size of an item created with createItem
    uint itemSize() const
    {
        return sizeof(IndexedStringData) + m_length;
    }

    //Should create an item where the information of the requested item is permanently stored. The pointer
    //@param item equals an allocated range with the size of itemSize().
    void createItem(IndexedStringData* item) const
    {
        item->length = m_length;
        item->refCount = 0;
        ++item;
        memcpy(item, m_text, m_length);
    }

    static void destroy(IndexedStringData* item, AbstractItemRepository&)
    {
        Q_UNUSED(item);
        //Nothing to do here (The object is not intelligent)
    }

    static bool persistent(const IndexedStringData* item)
    {
        return (bool)item->refCount;
    }

    //Should return whether the here requested item equals the given item
    bool equals(const IndexedStringData* item) const
    {
        return item->length == m_length && (memcmp(++item, m_text, m_length) == 0);
    }

    uint m_hash;
    unsigned short m_length;
    const char* m_text;
};

inline const char* c_strFromItem(const IndexedStringData* item)
{
    return reinterpret_cast<const char*>(item + 1);
}

///@param item must be valid(nonzero)
inline QString stringFromItem(const IndexedStringData* item)
{
    return QString::fromUtf8(c_strFromItem(item), item->length);
}

inline QByteArray arrayFromItem(const IndexedStringData* item)
{
    return QByteArray(c_strFromItem(item), item->length);
}

inline bool isSingleCharIndex(uint index)
{
    return (index & 0xffff0000) == 0xffff0000;
}

inline uint charToIndex(char c)
{
    return 0xffff0000 | c;
}

inline char indexToChar(uint index)
{
    Q_ASSERT(isSingleCharIndex(index));
    return static_cast<char>(index & 0xff);
}

using IndexedStringRepository = ItemRepository<IndexedStringData, IndexedStringRepositoryItemRequest, false, true>;
using GlobalIndexedStringRepository = RepositoryManager<IndexedStringRepository>;
GlobalIndexedStringRepository& getGlobalIndexedStringRepository()
{
    static GlobalIndexedStringRepository globalIndexedStringRepository("String Index");
    return globalIndexedStringRepository;
}

}

IndexedString::IndexedString()
    : m_index(0)
{
}

///@param str must be a utf8 encoded string, does not need to be 0-terminated.
///@param length must be its length in bytes.
IndexedString::IndexedString(const char* str, unsigned short length, uint hash)
{
    if (!length) {
        m_index = 0;
    } else if (length == 1) {
        m_index = charToIndex(str[0]);
    } else {
        QMutexLocker lock(getGlobalIndexedStringRepository()->mutex());

        m_index = getGlobalIndexedStringRepository()->index(IndexedStringRepositoryItemRequest(str, hash ? hash : hashString(str, length), length));

        if (shouldDoDUChainReferenceCounting(this))
            increase(getGlobalIndexedStringRepository()->dynamicItemFromIndexSimple(m_index)->refCount);
    }
}

IndexedString::IndexedString(char c)
    : m_index(charToIndex(c))
{}

IndexedString::IndexedString(const QUrl& url)
    : IndexedString(url.isLocalFile() ? url.toLocalFile() : url.toString())
{
    Q_ASSERT(url.isEmpty() || !url.isRelative());
}

IndexedString::IndexedString(const QString& string)
    : IndexedString(string.toUtf8())
{}

IndexedString::IndexedString(const char* str)
    : IndexedString(str, str ? strlen(str) : 0)
{}

IndexedString::IndexedString(const QByteArray& str)
    : IndexedString(str.constData(), str.length())
{}

IndexedString::~IndexedString()
{
    if (m_index && !isSingleCharIndex(m_index)) {
        if (shouldDoDUChainReferenceCounting(this)) {
            QMutexLocker lock(getGlobalIndexedStringRepository()->mutex());

            decrease(getGlobalIndexedStringRepository()->dynamicItemFromIndexSimple(m_index)->refCount);
        }
    }
}

IndexedString::IndexedString(const IndexedString& rhs)
    : m_index(rhs.m_index)
{
    if (m_index && !isSingleCharIndex(m_index)) {
        if (shouldDoDUChainReferenceCounting(this)) {
            QMutexLocker lock(getGlobalIndexedStringRepository()->mutex());
            increase(getGlobalIndexedStringRepository()->dynamicItemFromIndexSimple(m_index)->refCount);
        }
    }
}

IndexedString& IndexedString::operator=(const IndexedString& rhs)
{
    if (m_index == rhs.m_index) {
        return *this;
    }

    if (m_index && !isSingleCharIndex(m_index)) {

        if (shouldDoDUChainReferenceCounting(this)) {
            QMutexLocker lock(getGlobalIndexedStringRepository()->mutex());
            decrease(getGlobalIndexedStringRepository()->dynamicItemFromIndexSimple(m_index)->refCount);
        }
    }

    m_index = rhs.m_index;

    if (m_index && !isSingleCharIndex(m_index)) {
        if (shouldDoDUChainReferenceCounting(this)) {
            QMutexLocker lock(getGlobalIndexedStringRepository()->mutex());
            increase(getGlobalIndexedStringRepository()->dynamicItemFromIndexSimple(m_index)->refCount);
        }
    }

    return *this;
}

QUrl IndexedString::toUrl() const
{
    if (isEmpty()) {
        return {};
    }
    QUrl ret = QUrl::fromUserInput(str());
    Q_ASSERT(!ret.isRelative());
    return ret;
}

QString IndexedString::str() const
{
    if (!m_index) {
        return QString();
    } else if (isSingleCharIndex(m_index)) {
        return QString(QLatin1Char(indexToChar(m_index)));
    } else {
        return stringFromItem(getGlobalIndexedStringRepository()->itemFromIndex(m_index));
    }
}

int IndexedString::length() const
{
    return lengthFromIndex(m_index);
}

int IndexedString::lengthFromIndex(uint index)
{
    if (!index) {
        return 0;
    } else if (isSingleCharIndex(index)) {
        return 1;
    } else {
        return getGlobalIndexedStringRepository()->itemFromIndex(index)->length;
    }
}

const char* IndexedString::c_str() const
{
    if (!m_index) {
        return 0;
    } else if (isSingleCharIndex(m_index)) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
        const uint offset = 0;
#else
        const uint offset = 3;
#endif
        return reinterpret_cast<const char*>(&m_index) + offset;
    } else {
        return c_strFromItem(getGlobalIndexedStringRepository()->itemFromIndex(m_index));
    }

}

QByteArray IndexedString::byteArray() const
{
    if (!m_index) {
        return QByteArray();
    } else if (isSingleCharIndex(m_index)) {
        return QByteArray(1, indexToChar(m_index));
    } else {
        return arrayFromItem(getGlobalIndexedStringRepository()->itemFromIndex(m_index));
    }
}

uint IndexedString::hashString(const char* str, unsigned short length)
{
    RunningHash running;
    for (int a = length - 1; a >= 0; --a) {
        running.append(*str);
        ++str;
    }
    return running.hash;
}

uint IndexedString::indexForString(const char* str, short unsigned length, uint hash)
{
    if (!length) {
        return 0;
    } else if (length == 1) {
        return charToIndex(str[0]);
    } else {
        QMutexLocker lock(getGlobalIndexedStringRepository()->mutex());
        return getGlobalIndexedStringRepository()->index(IndexedStringRepositoryItemRequest(str, hash ? hash : hashString(str, length), length));
    }
}

uint IndexedString::indexForString(const QString& str, uint hash)
{
    const QByteArray array(str.toUtf8());
    return indexForString(array.constBegin(), array.size(), hash);
}

QDebug operator<<(QDebug s, const IndexedString& string)
{
    s.nospace() << string.str();
    return s.space();
}

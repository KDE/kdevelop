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

using IndexedStringRepository = ItemRepository<IndexedStringData, IndexedStringRepositoryItemRequest, false, false>;
using IndexedStringRepositoryManagerBase = RepositoryManager<IndexedStringRepository, true, false>;
class IndexedStringRepositoryManager : public IndexedStringRepositoryManagerBase
{
public:
    IndexedStringRepositoryManager()
      : IndexedStringRepositoryManagerBase(QStringLiteral("String Index"))
    {
        repository()->setMutex(&m_mutex);
    }

private:
    // non-recursive mutex to increase speed
    QMutex m_mutex;
};

IndexedStringRepository* globalIndexedStringRepository()
{
    static IndexedStringRepositoryManager manager;
    return manager.repository();
}

template<typename ReadAction>
auto readRepo(ReadAction action) -> decltype(action(globalIndexedStringRepository()))
{
    const auto* repo = globalIndexedStringRepository();
    QMutexLocker lock(repo->mutex());
    return action(repo);
}

template<typename EditAction>
auto editRepo(EditAction action) -> decltype(action(globalIndexedStringRepository()))
{
    auto* repo = globalIndexedStringRepository();
    QMutexLocker lock(repo->mutex());
    return action(repo);
}

inline void ref(IndexedString* string)
{
    const uint index = string->index();
    if (index && !isSingleCharIndex(index)) {
        if (shouldDoDUChainReferenceCounting(string)) {
            editRepo([index] (IndexedStringRepository* repo) {
                increase(repo->dynamicItemFromIndexSimple(index)->refCount);
            });
        }
    }
}

inline void deref(IndexedString* string)
{
    const uint index = string->index();
    if (index && !isSingleCharIndex(index)) {
        if (shouldDoDUChainReferenceCounting(string)) {
            editRepo([index] (IndexedStringRepository* repo) {
                decrease(repo->dynamicItemFromIndexSimple(index)->refCount);
            });
        }
    }
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
        const auto request = IndexedStringRepositoryItemRequest(str, hash ? hash : hashString(str, length), length);
        bool refcount = shouldDoDUChainReferenceCounting(this);
        m_index = editRepo([request, refcount] (IndexedStringRepository* repo) {
            auto index = repo->index(request);
            if (refcount) {
                increase(repo->dynamicItemFromIndexSimple(index)->refCount);
            }
            return index;
        });
    }
}

IndexedString::IndexedString(char c)
    : m_index(charToIndex(c))
{}

IndexedString::IndexedString(const QUrl& url)
    : IndexedString(url.isLocalFile() ? url.toLocalFile() : url.toString())
{
    Q_ASSERT(url.isEmpty() || !url.isRelative());
#if !defined(QT_NO_DEBUG)
    if (url != url.adjusted(QUrl::NormalizePathSegments)) {
      qWarning() << "wrong url" << url << url.adjusted(QUrl::NormalizePathSegments);
    }
#endif
    Q_ASSERT(url == url.adjusted(QUrl::NormalizePathSegments));
}

IndexedString::IndexedString(const QString& string)
    : IndexedString(string.toUtf8())
{}

IndexedString::IndexedString(const char* str)
    : IndexedString(str, str ? qstrlen(str) : 0)
{}

IndexedString::IndexedString(const QByteArray& str)
    : IndexedString(str.constData(), str.length())
{}

IndexedString::~IndexedString()
{
    deref(this);
}

IndexedString::IndexedString(const IndexedString& rhs)
    : m_index(rhs.m_index)
{
    ref(this);
}

IndexedString& IndexedString::operator=(const IndexedString& rhs)
{
    if (m_index == rhs.m_index) {
        return *this;
    }

    deref(this);

    m_index = rhs.m_index;

    ref(this);

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
        const uint index = m_index;
        return readRepo([index] (const IndexedStringRepository* repo) {
            return stringFromItem(repo->itemFromIndex(index));
        });
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
        return readRepo([index] (const IndexedStringRepository* repo) {
            return repo->itemFromIndex(index)->length;
        });
    }
}

const char* IndexedString::c_str() const
{
    if (!m_index) {
        return nullptr;
    } else if (isSingleCharIndex(m_index)) {
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
        const uint offset = 0;
#else
        const uint offset = 3;
#endif
        return reinterpret_cast<const char*>(&m_index) + offset;
    } else {
        const uint index = m_index;
        return readRepo([index] (const IndexedStringRepository* repo) {
            return c_strFromItem(repo->itemFromIndex(index));
        });
    }

}

QByteArray IndexedString::byteArray() const
{
    if (!m_index) {
        return QByteArray();
    } else if (isSingleCharIndex(m_index)) {
        return QByteArray(1, indexToChar(m_index));
    } else {
        const uint index = m_index;
        return readRepo([index] (const IndexedStringRepository* repo) {
            return arrayFromItem(repo->itemFromIndex(index));
        });
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
        const auto request = IndexedStringRepositoryItemRequest(str, hash ? hash : hashString(str, length), length);
        return editRepo([request] (IndexedStringRepository* repo) {
            return repo->index(request);
        });
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

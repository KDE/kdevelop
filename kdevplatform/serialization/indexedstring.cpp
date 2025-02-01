/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2016 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "indexedstring.h"

#include "itemrepository.h"
#include "referencecounting.h"
#include "repositorymanager.h"

#include <utility>

using namespace KDevelop;

namespace {
QString urlToString(const QUrl& url)
{
    Q_ASSERT(url.isEmpty() || !url.isRelative());
#if !defined(QT_NO_DEBUG)
    if (url != url.adjusted(QUrl::NormalizePathSegments)) {
        qWarning() << "wrong url" << url << url.adjusted(QUrl::NormalizePathSegments);
    }
#endif
    Q_ASSERT(url == url.adjusted(QUrl::NormalizePathSegments));

    return url.toString(QUrl::PreferLocalFile);
}

struct IndexedStringData
{
    unsigned short length;
    uint refCount;

    IndexedStringData& operator=(const IndexedStringData& rhs) = delete;

    uint itemSize() const
    {
        return sizeof(IndexedStringData) + length;
    }

    uint hash() const
    {
        IndexedString::RunningHash running;
        const char* str = reinterpret_cast<const char*>(this) + sizeof(IndexedStringData);
        for (int a = length - 1; a >= 0; --a) {
            running.append(*str);
            ++str;
        }

        return running.hash;
    }
};

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

    using HashType = uint;

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
        void* itemText = reinterpret_cast<void*>(item + 1);
        memcpy(itemText, m_text, m_length);
    }

    static void destroy(IndexedStringData* item, AbstractItemRepository&)
    {
        Q_UNUSED(item);
        //Nothing to do here (The object is not intelligent)
    }

    static bool persistent(const IndexedStringData* item)
    {
        return ( bool )item->refCount;
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

using IndexedStringRepository = ItemRepository<IndexedStringData, IndexedStringRepositoryItemRequest, false>;
}

namespace KDevelop
{
template<>
class ItemRepositoryFor<IndexedString>
{
    friend struct LockedItemRepository;
    static IndexedStringRepository& repo()
    {
        static QMutex mutex;
        static RepositoryManager<IndexedStringRepository, true, false> manager { QStringLiteral("String Index"),
                                                                                 &mutex };
        return *manager.repository();
    }
};
}

namespace
{
class ReferenceCountChanger
{
public:
    static ReferenceCountChanger increase(unsigned index)
    {
        return {index, 1};
    }
    static ReferenceCountChanger decrease(unsigned index)
    {
        return {index, static_cast<Summand>(-1)}; // unsigned integer overflow is fine
    }

    void editRepo() const
    {
        if (m_index && !isSingleCharIndex(m_index)) {
            LockedItemRepository::write<IndexedString>(*this);
        }
    }

    void operator()(IndexedStringRepository& repo) const
    {
        repo.dynamicItemFromIndexSimple(m_index)->refCount += m_summand;
    }

private:
    using Summand = decltype(IndexedStringData::refCount);

    ReferenceCountChanger(unsigned i, Summand s)
        : m_index{i}
        , m_summand{s}
    {}

    unsigned m_index;
    Summand m_summand;
};
inline void ref(unsigned index)
{
    ReferenceCountChanger::increase(index).editRepo();
}
inline void deref(unsigned index)
{
    ReferenceCountChanger::decrease(index).editRepo();
}
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
        m_index = LockedItemRepository::write<IndexedString>([request, refcount](IndexedStringRepository& repo) {
            auto index = repo.index(request);
            if (refcount) {
                ReferenceCountChanger::increase(index)(repo);
            }
            return index;
        });
    }
}

IndexedString::IndexedString(char c)
    : m_index(charToIndex(c))
{}

IndexedString::IndexedString(const QUrl& url)
    : IndexedString(urlToString(url))
{
}

IndexedString::IndexedString(QStringView string)
    : IndexedString(string.toUtf8())
{}

IndexedString::IndexedString(const char* str)
    : IndexedString(str, str ? qstrlen(str) : 0)
{}

IndexedString::IndexedString(const QByteArray& str)
    : IndexedString(str.constData(), str.length())
{}

// NOTE: the definitions of ref() and deref() are so complex that they can throw exceptions
// for many reasons. Yet the functions below, which call ref() and/or deref(), are
// implicitly (the destructor) or explicitly (the rest) noexcept. The noexcept-ness of
// these functions is important for correctness and performance. This is safe at the moment,
// because the entire KDevPlatformSerialization library, that contains IndexedString, is
// compiled with exceptions disabled (-fno-exceptions), which already prevents exception
// propagation to a caller of any non-inline function in this library.

IndexedString::~IndexedString()
{
    if (shouldDoDUChainReferenceCounting(this)) {
        deref(m_index);
    }
}

IndexedString::IndexedString(const IndexedString& rhs) noexcept
    : m_index(rhs.m_index)
{
    if (shouldDoDUChainReferenceCounting(this)) {
        ref(m_index);
    }
}

IndexedString& IndexedString::operator=(const IndexedString& rhs) noexcept
{
    if (m_index == rhs.m_index) {
        return *this;
    }

    if (shouldDoDUChainReferenceCounting(this)) {
        deref(m_index);
        ref(rhs.m_index);
    }

    m_index = rhs.m_index;
    return *this;
}

namespace KDevelop {
void swap(IndexedString& a, IndexedString& b) noexcept
{
    using std::swap;

    if (a.m_index == b.m_index) {
        return;
    }
    swap(a.m_index, b.m_index);

    const bool aRc = shouldDoDUChainReferenceCounting(&a);
    const bool bRc = shouldDoDUChainReferenceCounting(&b);

    if (aRc == bRc) {
        return;
    }

    auto noLongerRefCountedIndex = b.m_index;
    auto newlyRefCountedIndex = a.m_index;
    if (bRc) {
        swap(noLongerRefCountedIndex, newlyRefCountedIndex);
    }
    deref(noLongerRefCountedIndex);
    ref(newlyRefCountedIndex);
}
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
        return LockedItemRepository::read<IndexedString>([index](const IndexedStringRepository& repo) {
            return stringFromItem(repo.itemFromIndex(index));
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
        return LockedItemRepository::read<IndexedString>([index](const IndexedStringRepository& repo) {
            return repo.itemFromIndex(index)->length;
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
        return LockedItemRepository::read<IndexedString>([index](const IndexedStringRepository& repo) {
            return c_strFromItem(repo.itemFromIndex(index));
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
        return LockedItemRepository::read<IndexedString>([index](const IndexedStringRepository& repo) {
            return arrayFromItem(repo.itemFromIndex(index));
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
        return LockedItemRepository::write<IndexedString>([request](IndexedStringRepository& repo) {
            return repo.index(request);
        });
    }
}

uint IndexedString::indexForString(const QString& str, uint hash)
{
    const QByteArray array(str.toUtf8());
    return indexForString(array.constBegin(), array.size(), hash);
}

uint IndexedString::indexForUrl(const QUrl& url)
{
    return indexForString(urlToString(url));
}

QDebug operator<<(QDebug s, const IndexedString& string)
{
    s.nospace() << string.str();
    return s.space();
}

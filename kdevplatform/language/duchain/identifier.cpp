/*
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "identifier.h"

#include <QHash>
#include "stringhelpers.h"
#include "appendedlist_static.h"
#include "serialization/itemrepository.h"
#include "serialization/itemrepositoryreferencecounting.h"
#include <serialization/repositorymanager.h>
#include "util/kdevhash.h"
#include <debug.h>

#include <serialization/indexedstring.h>
#include <utility>

#define ifDebug(x) void()

namespace KDevelop {
template <bool dynamic = false>
class IdentifierPrivate
{
public:
    IdentifierPrivate()
    {
    }

    template <bool rhsDynamic>
    explicit IdentifierPrivate(const IdentifierPrivate<rhsDynamic>& rhs)
        : m_unique(rhs.m_unique)
        , m_identifier(rhs.m_identifier)
        , m_refCount(0)
        , m_hash(rhs.m_hash)
    {
        copyListsFrom(rhs);
    }

    ~IdentifierPrivate()
    {
        templateIdentifiersList.free(const_cast<IndexedTypeIdentifier*>(templateIdentifiers()));
    }

    IdentifierPrivate& operator=(const IdentifierPrivate& rhs) = delete;

    //Flags the stored hash-value invalid
    void clearHash()
    {
        //This is always called on an object private to an Identifier, so there is no threading-problem.
        Q_ASSERT(dynamic);
        m_hash = 0;
    }

    size_t hash() const
    {
        // Since this only needs reading and the data needs not to be private, this may be called by
        // multiple threads simultaneously, so computeHash() must be thread-safe.
        if (!m_hash && dynamic)
            computeHash();
        return m_hash;
    }

    int m_unique = 0;
    IndexedString m_identifier;
    uint m_refCount = 0;

    START_APPENDED_LISTS_STATIC(IdentifierPrivate)

    APPENDED_LIST_FIRST_STATIC(IndexedTypeIdentifier, templateIdentifiers)

    END_APPENDED_LISTS_STATIC(templateIdentifiers)

    uint itemSize() const
    {
        return sizeof(IdentifierPrivate<false> ) + lastOffsetBehind();
    }

    void computeHash() const
    {
        Q_ASSERT(dynamic);
        //this must stay thread-safe(may be called by multiple threads at a time)
        //The thread-safety is given because all threads will have the same result, and it will only be written once at the end.
        KDevHash kdevhash;
        kdevhash << m_identifier.hash() << m_unique;
        FOREACH_FUNCTION_STATIC(const IndexedTypeIdentifier &templateIdentifier, templateIdentifiers)
        kdevhash << templateIdentifier.hash();
        m_hash = kdevhash;
    }

    mutable size_t m_hash = 0;
};

using DynamicIdentifierPrivate = IdentifierPrivate<true>;
using ConstantIdentifierPrivate = IdentifierPrivate<false>;

struct IdentifierItemRequest
{
    IdentifierItemRequest(const DynamicIdentifierPrivate& identifier)
        : m_identifier(identifier)
    {
        identifier.hash(); //Make sure the hash is valid by calling this
    }

    enum {
        AverageSize = sizeof(IdentifierPrivate<false> ) + 4
    };

    //Should return the hash-value associated with this request(For example the hash of a string)
    size_t hash() const
    {
        return m_identifier.hash();
    }

    //Should return the size of an item created with createItem
    uint itemSize() const
    {
        return m_identifier.itemSize();
    }
    //Should create an item where the information of the requested item is permanently stored. The pointer
    //@param item equals an allocated range with the size of itemSize().
    void createItem(ConstantIdentifierPrivate* item) const
    {
        new (item) ConstantIdentifierPrivate(m_identifier);
    }

    static bool persistent(const ConstantIdentifierPrivate* item)
    {
        return ( bool )item->m_refCount;
    }

    static void destroy(ConstantIdentifierPrivate* item, AbstractItemRepository&)
    {
        item->~ConstantIdentifierPrivate();
    }

    //Should return whether the here requested item equals the given item
    bool equals(const ConstantIdentifierPrivate* item) const
    {
        return item->m_hash == m_identifier.m_hash
               && item->m_unique == m_identifier.m_unique
               && item->m_identifier == m_identifier.m_identifier
               && m_identifier.listsEqual(*item);
    }

    const DynamicIdentifierPrivate& m_identifier;
};

using IdentifierRepository = ItemRepository<ConstantIdentifierPrivate, IdentifierItemRequest, true, QRecursiveMutex>;
using IdentifierRepositoryManager = RepositoryManager<IdentifierRepository, false>;

template <>
class ItemRepositoryFor<IndexedIdentifier>
{
    friend struct LockedItemRepository;
    static IdentifierRepository& repo()
    {
        static QRecursiveMutex mutex;
        static IdentifierRepositoryManager manager(QStringLiteral("Identifier Repository"), &mutex);
        return *manager.repository();
    }

public:
    static auto* mutex() { return repo().mutex(); }
};

static uint emptyConstantIdentifierPrivateIndex()
{
    static const uint index = LockedItemRepository::write<IndexedIdentifier>(
        [](IdentifierRepository& repo) { return repo.index(DynamicIdentifierPrivate()); });
    return index;
}

static const ConstantIdentifierPrivate* emptyConstantIdentifierPrivate()
{
    static const ConstantIdentifierPrivate item;
    return &item;
}

bool IndexedIdentifier::isEmpty() const
{
    return m_index == emptyConstantIdentifierPrivateIndex();
}

/**
 * Before something is modified in QualifiedIdentifierPrivate, it must be made sure that
 * it is private to the QualifiedIdentifier it is used in(@see QualifiedIdentifier::prepareWrite)
 */
template <bool dynamic>
class QualifiedIdentifierPrivate
{
public:
    QualifiedIdentifierPrivate()
        : m_explicitlyGlobal(false)
        , m_isExpression(false)

    {
    }

    template <bool rhsDynamic>
    explicit QualifiedIdentifierPrivate(const QualifiedIdentifierPrivate<rhsDynamic>& rhs)
        : m_explicitlyGlobal(rhs.m_explicitlyGlobal)
        , m_isExpression(rhs.m_isExpression)
        , m_hash(rhs.m_hash)
        , m_refCount(0)
    {
        copyListsFrom(rhs);
    }

    ~QualifiedIdentifierPrivate()
    {
        identifiersList.free(const_cast<IndexedIdentifier*>(identifiers()));
    }

    QualifiedIdentifierPrivate& operator=(const QualifiedIdentifierPrivate& rhs) = delete;

    bool m_explicitlyGlobal : 1;
    bool m_isExpression : 1;
    mutable uint m_hash = 0;
    uint m_refCount = 0;

    START_APPENDED_LISTS_STATIC(QualifiedIdentifierPrivate)

    APPENDED_LIST_FIRST_STATIC(IndexedIdentifier, identifiers)

    END_APPENDED_LISTS_STATIC(identifiers)

    uint itemSize() const
    {
        return sizeof(QualifiedIdentifierPrivate<false> ) + lastOffsetBehind();
    }

    //Constructs m_identifiers
    void splitIdentifiers(QStringView str, int start)
    {
        Q_ASSERT(dynamic);
        uint currentStart = start;

        while (currentStart < ( uint )str.length()) {
            identifiersList.append(IndexedIdentifier(Identifier(str, currentStart, &currentStart)));
            while (currentStart < ( uint )str.length() && (str[currentStart] == QLatin1Char(' ')))
                ++currentStart;
            currentStart += 2; //Skip "::"
        }
    }

    inline void clearHash() const
    {
        m_hash = 0;
    }

    size_t hash() const
    {
        if (m_hash == 0) {
            KDevHash hash;

            quint32 bitfields = static_cast<quint32>(m_explicitlyGlobal)
                                | (m_isExpression << 1);
            hash << bitfields << identifiersSize();
            FOREACH_FUNCTION_STATIC(const IndexedIdentifier &identifier, identifiers) {
                hash << identifier.index();
            }

            m_hash = hash;
        }
        return m_hash;
    }
};

using DynamicQualifiedIdentifierPrivate = QualifiedIdentifierPrivate<true>;
using ConstantQualifiedIdentifierPrivate = QualifiedIdentifierPrivate<false>;

struct QualifiedIdentifierItemRequest
{
    QualifiedIdentifierItemRequest(const DynamicQualifiedIdentifierPrivate& identifier)
        : m_identifier(identifier)
    {
        identifier.hash(); //Make sure the hash is valid by calling this
    }

    enum {
        AverageSize = sizeof(QualifiedIdentifierPrivate<false> ) + 8
    };

    //Should return the hash-value associated with this request(For example the hash of a string)
    size_t hash() const
    {
        return m_identifier.hash();
    }

    //Should return the size of an item created with createItem
    uint itemSize() const
    {
        return m_identifier.itemSize();
    }

    /**
     * Should create an item where the information of the requested item is permanently stored. The pointer
     * @param item equals an allocated range with the size of itemSize().
     */
    void createItem(ConstantQualifiedIdentifierPrivate* item) const
    {
        Q_ASSERT(shouldDoDUChainReferenceCounting(item));
        Q_ASSERT(shouldDoDUChainReferenceCounting(reinterpret_cast<char*>(item) + (itemSize() - 1)));
        new (item) ConstantQualifiedIdentifierPrivate(m_identifier);
    }

    static bool persistent(const ConstantQualifiedIdentifierPrivate* item)
    {
        return ( bool )item->m_refCount;
    }

    static void destroy(ConstantQualifiedIdentifierPrivate* item, AbstractItemRepository&)
    {
        Q_ASSERT(shouldDoDUChainReferenceCounting(item));
        item->~ConstantQualifiedIdentifierPrivate();
    }

    //Should return whether the here requested item equals the given item
    bool equals(const ConstantQualifiedIdentifierPrivate* item) const
    {
        return item->m_explicitlyGlobal == m_identifier.m_explicitlyGlobal
               && item->m_isExpression == m_identifier.m_isExpression
               && item->m_hash == m_identifier.m_hash
               && m_identifier.listsEqual(*item);
    }

    const DynamicQualifiedIdentifierPrivate& m_identifier;
};

using QualifiedIdentifierRepository
    = ItemRepository<ConstantQualifiedIdentifierPrivate, QualifiedIdentifierItemRequest, true, QRecursiveMutex>;
using QualifiedIdentifierRepositoryManager = RepositoryManager<QualifiedIdentifierRepository, false>;

template <>
class ItemRepositoryFor<IndexedQualifiedIdentifier>
{
    friend struct LockedItemRepository;
    static QualifiedIdentifierRepository& repo()
    {
        static QualifiedIdentifierRepositoryManager manager(QStringLiteral("Qualified Identifier Repository"),
                                                            ItemRepositoryFor<IndexedIdentifier>::mutex());
        return *manager.repository();
    }
};

static uint emptyConstantQualifiedIdentifierPrivateIndex()
{
    static const uint index = LockedItemRepository::write<IndexedQualifiedIdentifier>(
        [](QualifiedIdentifierRepository& repo) { return repo.index(DynamicQualifiedIdentifierPrivate()); });
    return index;
}

static const ConstantQualifiedIdentifierPrivate* emptyConstantQualifiedIdentifierPrivate()
{
    static const ConstantQualifiedIdentifierPrivate item;
    return &item;
}

Identifier::Identifier(const Identifier& rhs)
{
    rhs.makeConstant();
    cd = rhs.cd;
    m_index = rhs.m_index;
}

Identifier::Identifier(uint index)
    : m_index(index)
{
    Q_ASSERT(m_index);
    cd = LockedItemRepository::read<IndexedIdentifier>(
        [index](const IdentifierRepository& repo) { return repo.itemFromIndex(index); });
}

Identifier::Identifier(const IndexedString& str)
{
    if (str.isEmpty()) {
        m_index = emptyConstantIdentifierPrivateIndex();
        cd = emptyConstantIdentifierPrivate();
    } else {
        m_index = 0;
        dd = new IdentifierPrivate<true>;
        dd->m_identifier = str;
    }
}

Identifier::Identifier(QStringView id, uint start, uint* takenRange)
{
    if (id.isEmpty()) {
        m_index = emptyConstantIdentifierPrivateIndex();
        cd = emptyConstantIdentifierPrivate();
        return;
    }

    m_index = 0;
    dd = new IdentifierPrivate<true>;

    ///Extract template-parameters
    ParamIterator paramIt(u"<>:", id, start);
    dd->m_identifier = IndexedString(paramIt.prefix().trimmed());
    while (paramIt) {
        appendTemplateIdentifier(IndexedTypeIdentifier(IndexedQualifiedIdentifier(QualifiedIdentifier(*paramIt))));
        ++paramIt;
    }

    if (takenRange)
        *takenRange = paramIt.position();
}

Identifier::Identifier()
    : m_index(emptyConstantIdentifierPrivateIndex())
    , cd(emptyConstantIdentifierPrivate())
{
}

Identifier& Identifier::operator=(const Identifier& rhs)
{
    if (dd == rhs.dd && cd == rhs.cd)
        return *this;

    if (!m_index)
        delete dd;
    dd = nullptr;

    rhs.makeConstant();
    cd = rhs.cd;
    m_index = rhs.m_index;
    Q_ASSERT(cd);
    return *this;
}

Identifier::Identifier(Identifier&& rhs) Q_DECL_NOEXCEPT
    : m_index(rhs.m_index)
{
    if (m_index) {
        cd = rhs.cd;
    } else {
        dd = rhs.dd;
    }
    rhs.cd = emptyConstantIdentifierPrivate();
    rhs.m_index = emptyConstantIdentifierPrivateIndex();
}

Identifier& Identifier::operator=(Identifier&& rhs) Q_DECL_NOEXCEPT
{
    if (dd == rhs.dd && cd == rhs.cd)
        return *this;

    if (!m_index) {
        delete dd;
        dd = nullptr;
    }

    m_index = rhs.m_index;

    if (m_index) {
        cd = rhs.cd;
    } else {
        dd = rhs.dd;
    }
    rhs.cd = emptyConstantIdentifierPrivate();
    rhs.m_index = emptyConstantIdentifierPrivateIndex();

    return *this;
}

Identifier::~Identifier()
{
    if (!m_index)
        delete dd;
}

bool Identifier::nameEquals(const Identifier& rhs) const
{
    return identifier() == rhs.identifier();
}

size_t Identifier::hash() const
{
    if (!m_index)
        return dd->hash();
    else
        return cd->hash();
}

bool Identifier::isEmpty() const
{
    if (!m_index)
        return dd->m_identifier.isEmpty() && dd->m_unique == 0 && dd->templateIdentifiersSize() == 0;
    else
        return cd->m_identifier.isEmpty() && cd->m_unique == 0 && cd->templateIdentifiersSize() == 0;
}

Identifier Identifier::unique(int token)
{
    Identifier ret;
    ret.setUnique(token);
    return ret;
}

bool Identifier::isUnique() const
{
    if (!m_index)
        return dd->m_unique;
    else
        return cd->m_unique;
}

int Identifier::uniqueToken() const
{
    if (!m_index)
        return dd->m_unique;
    else
        return cd->m_unique;
}

void Identifier::setUnique(int token)
{
    if (token != uniqueToken()) {
        prepareWrite();
        dd->m_unique = token;
    }
}

const IndexedString Identifier::identifier() const
{
    if (!m_index)
        return dd->m_identifier;
    else
        return cd->m_identifier;
}

void Identifier::setIdentifier(QStringView identifier)
{
    IndexedString id(identifier);
    if (id != this->identifier()) {
        prepareWrite();
        dd->m_identifier = std::move(id);
    }
}

void Identifier::setIdentifier(const IndexedString& identifier)
{
    if (identifier != this->identifier()) {
        prepareWrite();
        dd->m_identifier = identifier;
    }
}

IndexedTypeIdentifier Identifier::templateIdentifier(int num) const
{
    if (!m_index)
        return dd->templateIdentifiers()[num];
    else
        return cd->templateIdentifiers()[num];
}

uint Identifier::templateIdentifiersCount() const
{
    if (!m_index)
        return dd->templateIdentifiersSize();
    else
        return cd->templateIdentifiersSize();
}

void Identifier::appendTemplateIdentifier(const IndexedTypeIdentifier& identifier)
{
    prepareWrite();
    dd->templateIdentifiersList.append(identifier);
}

void Identifier::clearTemplateIdentifiers()
{
    prepareWrite();
    dd->templateIdentifiersList.clear();
}

uint Identifier::index() const
{
    makeConstant();
    Q_ASSERT(m_index);
    return m_index;
}

bool Identifier::inRepository() const
{
    return m_index;
}

void Identifier::setTemplateIdentifiers(const QList<IndexedTypeIdentifier>& templateIdentifiers)
{
    prepareWrite();
    dd->templateIdentifiersList.clear();
    for (const IndexedTypeIdentifier& id : templateIdentifiers) {
        dd->templateIdentifiersList.append(id);
    }
}

QString Identifier::toString(IdentifierStringFormattingOptions options) const
{
    QString ret = identifier().str();

    if (!options.testFlag(RemoveTemplateInformation) && templateIdentifiersCount()) {
        QStringList templateIds;
        templateIds.reserve(templateIdentifiersCount());
        for (uint i = 0; i < templateIdentifiersCount(); ++i) {
            templateIds.append(templateIdentifier(i).toString(options));
        }

        ret += QLatin1String("< ") + templateIds.join(QLatin1String(", ")) + QLatin1String(" >");
    }

    return ret;
}

bool Identifier::isReserved() const
{
    const auto str = identifier().str();
    constexpr QLatin1Char underscore{'_'};
    return str.size() >= 2 && str[0] == underscore && (str[1] == underscore || str[1].isUpper());
}

bool Identifier::operator==(const Identifier& rhs) const
{
    return index() == rhs.index();
}

bool Identifier::operator!=(const Identifier& rhs) const
{
    return !operator==(rhs);
}

uint QualifiedIdentifier::index() const
{
    makeConstant();
    Q_ASSERT(m_index);
    return m_index;
}

void Identifier::makeConstant() const
{
    if (m_index)
        return;

    LockedItemRepository::write<IndexedIdentifier>(
        [&, request = IdentifierItemRequest(*dd)](IdentifierRepository& repo) {
            m_index = repo.index(request);
            delete dd;
            cd = repo.itemFromIndex(m_index);
        });
}

void Identifier::prepareWrite()
{
    if (m_index) {
        const IdentifierPrivate<false>* oldCc = cd;
        dd = new IdentifierPrivate<true>;
        dd->m_hash = oldCc->m_hash;
        dd->m_unique = oldCc->m_unique;
        dd->m_identifier = oldCc->m_identifier;
        dd->copyListsFrom(*oldCc);
        m_index = 0;
    }

    dd->clearHash();
}

bool QualifiedIdentifier::inRepository() const
{
    if (m_index)
        return true;

    return LockedItemRepository::read<IndexedQualifiedIdentifier>(
        [&, request = QualifiedIdentifierItemRequest(*dd)](const QualifiedIdentifierRepository& repo) {
            return static_cast<bool>(repo.findIndex(request));
        });
}

QualifiedIdentifier::QualifiedIdentifier(uint index)
    : m_index(index)
    , cd(LockedItemRepository::read<IndexedQualifiedIdentifier>(
          [index](const QualifiedIdentifierRepository& repo) { return repo.itemFromIndex(index); }))
{
}

QualifiedIdentifier::QualifiedIdentifier(QStringView id, bool isExpression)
{
    if (id.isEmpty()) {
        m_index = emptyConstantQualifiedIdentifierPrivateIndex();
        cd = emptyConstantQualifiedIdentifierPrivate();
        return;
    }

    m_index = 0;
    dd = new DynamicQualifiedIdentifierPrivate;

    if (isExpression) {
        setIsExpression(true);
        if (!id.isEmpty()) {
            //Prevent tokenization, since we may lose information there
            Identifier finishedId;
            finishedId.setIdentifier(id);
            push(finishedId);
        }
    } else {
        if (id.startsWith(QLatin1String("::"))) {
            dd->m_explicitlyGlobal = true;
            dd->splitIdentifiers(id, 2);
        } else {
            dd->m_explicitlyGlobal = false;
            dd->splitIdentifiers(id, 0);
        }
    }
}

QualifiedIdentifier::QualifiedIdentifier(const Identifier& id)
{
    if (id.isEmpty()) {
        m_index = emptyConstantQualifiedIdentifierPrivateIndex();
        cd = emptyConstantQualifiedIdentifierPrivate();
        return;
    }

    m_index = 0;
    dd = new DynamicQualifiedIdentifierPrivate;

    if (id.dd->m_identifier.str().isEmpty()) {
        dd->m_explicitlyGlobal = true;
    } else {
        dd->m_explicitlyGlobal = false;
        dd->identifiersList.append(IndexedIdentifier(id));
    }
}

QualifiedIdentifier::QualifiedIdentifier()
    : m_index(emptyConstantQualifiedIdentifierPrivateIndex())
    , cd(emptyConstantQualifiedIdentifierPrivate())
{
}

QualifiedIdentifier::QualifiedIdentifier(const QualifiedIdentifier& id)
{
    if (id.m_index) {
        m_index = id.m_index;
        cd = id.cd;
    } else {
        m_index = 0;
        dd = new QualifiedIdentifierPrivate<true>(*id.dd);
    }
}

QualifiedIdentifier::QualifiedIdentifier(QualifiedIdentifier&& rhs) Q_DECL_NOEXCEPT
    : m_index(rhs.m_index)
{
    if (m_index) {
        cd = rhs.cd;
    } else {
        dd = rhs.dd;
    }
    rhs.m_index = emptyConstantQualifiedIdentifierPrivateIndex();
    rhs.cd = emptyConstantQualifiedIdentifierPrivate();
}

QualifiedIdentifier& QualifiedIdentifier::operator=(const QualifiedIdentifier& rhs)
{
    if (dd == rhs.dd && cd == rhs.cd)
        return *this;

    if (!m_index)
        delete dd;
    rhs.makeConstant();
    cd = rhs.cd;
    m_index = rhs.m_index;
    return *this;
}

QualifiedIdentifier& QualifiedIdentifier::operator=(QualifiedIdentifier&& rhs) Q_DECL_NOEXCEPT
{
    if (!m_index)
        delete dd;
    m_index = rhs.m_index;
    if (m_index) {
        cd = rhs.cd;
    } else {
        dd = rhs.dd;
    }
    rhs.cd = emptyConstantQualifiedIdentifierPrivate();
    rhs.m_index = emptyConstantQualifiedIdentifierPrivateIndex();
    return *this;
}

QualifiedIdentifier::~QualifiedIdentifier()
{
    if (!m_index)
        delete dd;
}

QStringList QualifiedIdentifier::toStringList(IdentifierStringFormattingOptions options) const
{
    QStringList ret;
    ret.reserve(explicitlyGlobal() + count());
    if (explicitlyGlobal())
        ret.append(QString());

    if (m_index) {
        ret.reserve(ret.size() + cd->identifiersSize());
        FOREACH_FUNCTION_STATIC(const IndexedIdentifier &index, cd->identifiers)
        ret << index.identifier().toString(options);
    } else {
        ret.reserve(ret.size() + dd->identifiersSize());
        FOREACH_FUNCTION_STATIC(const IndexedIdentifier &index, dd->identifiers)
        ret << index.identifier().toString(options);
    }

    return ret;
}

QString QualifiedIdentifier::toString(IdentifierStringFormattingOptions options) const
{
    const QString doubleColon = QStringLiteral("::");

    QString ret;
    if (!options.testFlag(RemoveExplicitlyGlobalPrefix) && explicitlyGlobal())
        ret = doubleColon;

    QStringList identifiers;
    if (m_index) {
        identifiers.reserve(cd->identifiersSize());
        FOREACH_FUNCTION_STATIC(const IndexedIdentifier &index, cd->identifiers)
        {
            identifiers += index.identifier().toString(options);
        }
    } else {
        identifiers.reserve(dd->identifiersSize());
        FOREACH_FUNCTION_STATIC(const IndexedIdentifier &index, dd->identifiers)
        {
            identifiers += index.identifier().toString(options);
        }
    }

    return ret + identifiers.join(doubleColon);
}

QualifiedIdentifier QualifiedIdentifier::merge(const QualifiedIdentifier& base) const
{
    QualifiedIdentifier ret(base);
    ret.push(*this);
    return ret;
}

QualifiedIdentifier QualifiedIdentifier::operator+(const QualifiedIdentifier& rhs) const
{
    return rhs.merge(*this);
}

QualifiedIdentifier& QualifiedIdentifier::operator+=(const QualifiedIdentifier& rhs)
{
    push(rhs);
    return *this;
}

QualifiedIdentifier QualifiedIdentifier::operator+(const Identifier& rhs) const
{
    QualifiedIdentifier ret(*this);
    ret.push(rhs);
    return ret;
}

QualifiedIdentifier& QualifiedIdentifier::operator+=(const Identifier& rhs)
{
    push(rhs);
    return *this;
}

QualifiedIdentifier QualifiedIdentifier::operator+(const IndexedIdentifier& rhs) const
{
    QualifiedIdentifier ret(*this);
    ret.push(rhs);
    return ret;
}

QualifiedIdentifier& QualifiedIdentifier::operator+=(const IndexedIdentifier& rhs)
{
    push(rhs);
    return *this;
}

bool QualifiedIdentifier::isExpression() const
{
    if (m_index)
        return cd->m_isExpression;
    else
        return dd->m_isExpression;
}

void QualifiedIdentifier::setIsExpression(bool is)
{
    if (is != isExpression()) {
        prepareWrite();
        dd->m_isExpression = is;
    }
}

bool QualifiedIdentifier::explicitlyGlobal() const
{
    // True if started with "::"
    if (m_index)
        return cd->m_explicitlyGlobal;
    else
        return dd->m_explicitlyGlobal;
}

void QualifiedIdentifier::setExplicitlyGlobal(bool eg)
{
    if (eg != explicitlyGlobal()) {
        prepareWrite();
        dd->m_explicitlyGlobal = eg;
    }
}

bool QualifiedIdentifier::sameIdentifiers(const QualifiedIdentifier& rhs) const
{
    if (m_index && rhs.m_index)
        return cd->listsEqual(*rhs.cd);
    else if (m_index && !rhs.m_index)
        return cd->listsEqual(*rhs.dd);
    else if (!m_index && !rhs.m_index)
        return dd->listsEqual(*rhs.dd);
    else
        return dd->listsEqual(*rhs.cd);
}

bool QualifiedIdentifier::operator==(const QualifiedIdentifier& rhs) const
{
    if (cd == rhs.cd)
        return true;
    return hash() == rhs.hash() && sameIdentifiers(rhs);
}

bool QualifiedIdentifier::operator!=(const QualifiedIdentifier& rhs) const
{
    return !operator==(rhs);
}

bool QualifiedIdentifier::beginsWith(const QualifiedIdentifier& other) const
{
    uint c = count();
    uint oc = other.count();

    for (uint i = 0; i < c && i < oc; ++i)
        if (at(i) == other.at(i)) {
            continue;
        } else {
            return false;
        }

    return true;
}

struct Visitor
{
    Visitor(KDevVarLengthArray<QualifiedIdentifier>& target, size_t hash)
        : target(target)
        , hash(hash)
    {
    }

    bool operator()(const ConstantQualifiedIdentifierPrivate* item, uint index) const
    {
        if (item->m_hash == hash)
            target.append(QualifiedIdentifier(index));
        return true;
    }

    KDevVarLengthArray<QualifiedIdentifier>& target;
    const size_t hash;
};

size_t QualifiedIdentifier::hash() const
{
    if (m_index)
        return cd->hash();
    else
        return dd->hash();
}

size_t qHash(const IndexedTypeIdentifier& id)
{
    return id.hash();
}

size_t qHash(const QualifiedIdentifier& id)
{
    return id.hash();
}

size_t qHash(const Identifier& id)
{
    return id.hash();
}

bool QualifiedIdentifier::isQualified() const
{
    return count() > 1 || explicitlyGlobal();
}

void QualifiedIdentifier::push(const Identifier& id)
{
    if (id.isEmpty())
        return;

    push(IndexedIdentifier(id));
}

void QualifiedIdentifier::push(const IndexedIdentifier& id)
{
    if (id.isEmpty()) {
        return;
    }

    prepareWrite();

    dd->identifiersList.append(id);
}

void QualifiedIdentifier::push(const QualifiedIdentifier& id)
{
    if (id.isEmpty()) {
        return;
    }

    prepareWrite();

    if (id.m_index) {
        dd->identifiersList.append(id.cd->identifiers(), id.cd->identifiersSize());
    } else {
        dd->identifiersList.append(id.dd->identifiers(), id.dd->identifiersSize());
    }

    if (id.explicitlyGlobal()) {
        setExplicitlyGlobal(true);
    }
}

void QualifiedIdentifier::pop()
{
    prepareWrite();
    if (!dd->identifiersSize())
        return;
    dd->identifiersList.resize(dd->identifiersList.size() - 1);
}

void QualifiedIdentifier::clear()
{
    prepareWrite();
    dd->identifiersList.clear();
    dd->m_explicitlyGlobal = false;
    dd->m_isExpression = false;
}

bool QualifiedIdentifier::isEmpty() const
{
    if (m_index)
        return cd->identifiersSize() == 0;
    else
        return dd->identifiersSize() == 0;
}

int QualifiedIdentifier::count() const
{
    if (m_index)
        return cd->identifiersSize();
    else
        return dd->identifiersSize();
}

Identifier QualifiedIdentifier::first() const
{
    return indexedFirst().identifier();
}

IndexedIdentifier QualifiedIdentifier::indexedFirst() const
{
    if ((m_index && cd->identifiersSize() == 0) || (!m_index && dd->identifiersSize() == 0))
        return IndexedIdentifier();
    else
        return indexedAt(0);
}

Identifier QualifiedIdentifier::last() const
{
    return indexedLast().identifier();
}

IndexedIdentifier QualifiedIdentifier::indexedLast() const
{
    uint c = count();
    if (c)
        return indexedAt(c - 1);
    else
        return IndexedIdentifier();
}

Identifier QualifiedIdentifier::top() const
{
    return last();
}

QualifiedIdentifier QualifiedIdentifier::mid(int pos, int len) const
{
    QualifiedIdentifier ret;
    if (pos == 0)
        ret.setExplicitlyGlobal(explicitlyGlobal());

    int cnt = ( int )count();

    if (len == -1)
        len = cnt - pos;

    if (pos + len > cnt)
        len -= cnt - (pos + len);

    for (int a = pos; a < pos + len; a++)
        ret.push(at(a));

    return ret;
}

Identifier QualifiedIdentifier::at(int i) const
{
    return indexedAt(i).identifier();
}

IndexedIdentifier QualifiedIdentifier::indexedAt(int i) const
{
    if (m_index) {
        Q_ASSERT(i >= 0 && i < ( int )cd->identifiersSize());
        return cd->identifiers()[i];
    } else {
        Q_ASSERT(i >= 0 && i < ( int )dd->identifiersSize());
        return dd->identifiers()[i];
    }
}

void QualifiedIdentifier::makeConstant() const
{
    if (m_index)
        return;

    LockedItemRepository::write<IndexedQualifiedIdentifier>(
        [&, request = QualifiedIdentifierItemRequest(*dd)](QualifiedIdentifierRepository& repo) {
            m_index = repo.index(request);
            delete dd;
            cd = repo.itemFromIndex(m_index);
        });
}

void QualifiedIdentifier::prepareWrite()
{
    if (m_index) {
        const QualifiedIdentifierPrivate<false>* oldCc = cd;
        dd = new QualifiedIdentifierPrivate<true>;
        dd->m_explicitlyGlobal = oldCc->m_explicitlyGlobal;
        dd->m_isExpression = oldCc->m_isExpression;
        dd->m_hash = oldCc->m_hash;

        dd->copyListsFrom(*oldCc);
        m_index = 0;
    }

    dd->clearHash();
}

size_t IndexedTypeIdentifier::hash() const
{
    quint32 bitfields = static_cast<quint32>(m_isConstant)
                        | (m_isReference << 1)
                        | (m_isRValue << 2)
                        | (m_isVolatile << 3)
                        | (m_pointerDepth << 4)
                        | (m_pointerConstMask << 9);
    return KDevHash() << m_identifier.index() << bitfields;
}

bool IndexedTypeIdentifier::operator==(const IndexedTypeIdentifier& rhs) const
{
    return m_identifier == rhs.m_identifier
           && m_isConstant == rhs.m_isConstant
           && m_isReference == rhs.m_isReference
           && m_isRValue == rhs.m_isRValue
           && m_isVolatile == rhs.m_isVolatile
           && m_pointerConstMask == rhs.m_pointerConstMask
           && m_pointerDepth == rhs.m_pointerDepth;
}

bool IndexedTypeIdentifier::operator!=(const IndexedTypeIdentifier& rhs) const
{
    return !operator==(rhs);
}

bool IndexedTypeIdentifier::isReference() const
{
    return m_isReference;
}

void IndexedTypeIdentifier::setIsReference(bool isRef)
{
    m_isReference = isRef;
}

bool IndexedTypeIdentifier::isRValue() const
{
    return m_isRValue;
}

void IndexedTypeIdentifier::setIsRValue(bool isRVal)
{
    m_isRValue = isRVal;
}

bool IndexedTypeIdentifier::isConstant() const
{
    return m_isConstant;
}

void IndexedTypeIdentifier::setIsConstant(bool isConst)
{
    m_isConstant = isConst;
}

bool IndexedTypeIdentifier::isVolatile() const
{
    return m_isVolatile;
}

void IndexedTypeIdentifier::setIsVolatile(bool isVolatile)
{
    m_isVolatile = isVolatile;
}

int IndexedTypeIdentifier::pointerDepth() const
{
    return m_pointerDepth;
}

void IndexedTypeIdentifier::setPointerDepth(int depth)
{
    Q_ASSERT(depth <= 23 && depth >= 0);
    ///Clear the mask in removed fields
    for (int s = depth; s < ( int )m_pointerDepth; ++s)
        setIsConstPointer(s, false);

    m_pointerDepth = depth;
}

bool IndexedTypeIdentifier::isConstPointer(int depthNumber) const
{
    return m_pointerConstMask & (1 << depthNumber);
}

void IndexedTypeIdentifier::setIsConstPointer(int depthNumber, bool constant)
{
    if (constant)
        m_pointerConstMask |= (1 << depthNumber);
    else
        m_pointerConstMask &= (~(1 << depthNumber));
}

QString IndexedTypeIdentifier::toString(IdentifierStringFormattingOptions options) const
{
    QString ret;
    if (isConstant())
        ret += QLatin1String("const ");
    if (isVolatile())
        ret += QLatin1String("volatile ");

    ret += m_identifier.identifier().toString(options);
    for (int a = 0; a < pointerDepth(); ++a) {
        ret += QLatin1Char('*');
        if (isConstPointer(a))
            ret += QLatin1String("const");
    }

    if (isRValue())
        ret += QLatin1String("&&");
    else if (isReference())
        ret += QLatin1Char('&');
    return ret;
}

IndexedTypeIdentifier::IndexedTypeIdentifier(const IndexedQualifiedIdentifier& identifier)
    : m_identifier(identifier)
    , m_isConstant(false)
    , m_isReference(false)
    , m_isRValue(false)
    , m_isVolatile(false)
    , m_pointerDepth(0)
    , m_pointerConstMask(0)
{ }

IndexedTypeIdentifier::IndexedTypeIdentifier(QStringView identifier, bool isExpression)
    : m_identifier(QualifiedIdentifier(identifier, isExpression))
    , m_isConstant(false)
    , m_isReference(false)
    , m_isRValue(false)
    , m_isVolatile(false)
    , m_pointerDepth(0)
    , m_pointerConstMask(0)
{ }

// NOTE: the definitions of ItemRepositoryReferenceCounting's inc(), dec() and setIndex() are so
// complex that they can throw exceptions for many reasons. Yet some special member functions of
// Indexed[Qualified]Identifier, which call them, are implicitly (the destructors) or explicitly
// noexcept. The noexcept-ness of these functions is important for correctness and performance.
// This is safe at the moment, because the entire KDevPlatformLanguage library, that contains these
// classes, is compiled with exceptions disabled (-fno-exceptions), which already prevents exception
// propagation to a caller of any non-inline function in this library.

IndexedIdentifier::IndexedIdentifier(unsigned int index)
    : m_index(index)
{
    ItemRepositoryReferenceCounting::inc(this);
}

IndexedIdentifier::IndexedIdentifier()
    : IndexedIdentifier(emptyConstantIdentifierPrivateIndex())
{
}

IndexedIdentifier::IndexedIdentifier(const Identifier& id)
    : IndexedIdentifier(id.index())
{
}

IndexedIdentifier::IndexedIdentifier(const IndexedIdentifier& rhs) noexcept
    : IndexedIdentifier(rhs.m_index)
{
}

IndexedIdentifier::~IndexedIdentifier()
{
    ItemRepositoryReferenceCounting::dec(this);
}

IndexedIdentifier& IndexedIdentifier::operator=(unsigned int index)
{
    ItemRepositoryReferenceCounting::setIndex(this, m_index, index);
    return *this;
}

IndexedIdentifier& IndexedIdentifier::operator=(const Identifier& id)
{
    return operator=(id.index());
}

IndexedIdentifier& IndexedIdentifier::operator=(const IndexedIdentifier& id) noexcept
{
    return operator=(id.m_index);
}

bool IndexedIdentifier::operator==(const IndexedIdentifier& rhs) const
{
    return m_index == rhs.m_index;
}

bool IndexedIdentifier::operator!=(const IndexedIdentifier& rhs) const
{
    return m_index != rhs.m_index;
}

bool IndexedIdentifier::operator==(const Identifier& id) const
{
    return m_index == id.index();
}

Identifier IndexedIdentifier::identifier() const
{
    return Identifier(m_index);
}

IndexedIdentifier::operator Identifier() const
{
    return Identifier(m_index);
}

bool IndexedQualifiedIdentifier::isValid() const
{
    return m_index != emptyConstantQualifiedIdentifierPrivateIndex();
}

bool IndexedQualifiedIdentifier::isEmpty() const
{
    return m_index == emptyConstantQualifiedIdentifierPrivateIndex();
}

int cnt = 0;

IndexedQualifiedIdentifier IndexedTypeIdentifier::identifier() const
{
    return m_identifier;
}

void IndexedTypeIdentifier::setIdentifier(const IndexedQualifiedIdentifier& id)
{
    m_identifier = id;
}

IndexedQualifiedIdentifier::IndexedQualifiedIdentifier(unsigned int index)
    : m_index(index)
{
    ifDebug(qCDebug(LANGUAGE) << "(" << ++cnt << ") Creating" << identifier().toString() << m_index);

    ItemRepositoryReferenceCounting::inc(this);
}

IndexedQualifiedIdentifier::IndexedQualifiedIdentifier()
    : IndexedQualifiedIdentifier(emptyConstantQualifiedIdentifierPrivateIndex())
{
}

IndexedQualifiedIdentifier::IndexedQualifiedIdentifier(const QualifiedIdentifier& id)
    : IndexedQualifiedIdentifier(id.index())
{
}

IndexedQualifiedIdentifier::IndexedQualifiedIdentifier(const IndexedQualifiedIdentifier& id) noexcept
    : IndexedQualifiedIdentifier(id.m_index)
{
}

IndexedQualifiedIdentifier& IndexedQualifiedIdentifier::operator=(unsigned int index)
{
    ifDebug(qCDebug(LANGUAGE) << "(" << ++cnt << ") Assigning to" << identifier().toString() << m_index);

    ItemRepositoryReferenceCounting::setIndex(this, m_index, index);
    return *this;
}

IndexedQualifiedIdentifier& IndexedQualifiedIdentifier::operator=(const QualifiedIdentifier& id)
{
    return operator=(id.index());
}

IndexedQualifiedIdentifier& IndexedQualifiedIdentifier::operator=(const IndexedQualifiedIdentifier& rhs) noexcept
{
    return operator=(rhs.m_index);
}

IndexedQualifiedIdentifier::~IndexedQualifiedIdentifier()
{
    ifDebug(qCDebug(LANGUAGE) << "(" << ++cnt << ") Destroying" << identifier().toString() << m_index);
    ItemRepositoryReferenceCounting::dec(this);
}

bool IndexedQualifiedIdentifier::operator==(const IndexedQualifiedIdentifier& rhs) const
{
    return m_index == rhs.m_index;
}

bool IndexedQualifiedIdentifier::operator==(const QualifiedIdentifier& id) const
{
    return m_index == id.index();
}

QualifiedIdentifier IndexedQualifiedIdentifier::identifier() const
{
    return QualifiedIdentifier(m_index);
}

IndexedQualifiedIdentifier::operator QualifiedIdentifier() const
{
    return QualifiedIdentifier(m_index);
}

void initIdentifierRepository()
{
    emptyConstantIdentifierPrivateIndex();
    emptyConstantIdentifierPrivate();
    emptyConstantQualifiedIdentifierPrivateIndex();
    emptyConstantQualifiedIdentifierPrivate();
}
}

QDebug operator<<(QDebug s, const KDevelop::Identifier& identifier)
{
    s.nospace() << identifier.toString();
    return s.space();
}

QDebug operator<<(QDebug s, const KDevelop::QualifiedIdentifier& identifier)
{
    s.nospace() << identifier.toString();
    return s.space();
}

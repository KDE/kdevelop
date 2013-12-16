/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>
    Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "identifier.h"

#include <QtCore/QHash>
#include "stringhelpers.h"
#include "indexedstring.h"
#include "appendedlist_static.h"
#include "repositories/itemrepository.h"
#include "util/kdevhash.h"

#define ifDebug(x)

namespace KDevelop
{
template<bool dynamic = false>
class IdentifierPrivate
{
public:
  IdentifierPrivate()
    : m_unique(0)
    , m_refCount(0)
    , m_hash(0)
  {
  }

  template<bool rhsDynamic>
  IdentifierPrivate(const IdentifierPrivate<rhsDynamic>& rhs)
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

  //Flags the stored hash-value invalid
  void clearHash()
  {
    //This is always called on an object private to an Identifier, so there is no threading-problem.
    Q_ASSERT(dynamic);
    m_hash = 0;
  }

  uint hash() const
  {
    // Since this only needs reading and the data needs not to be private, this may be called by
    // multiple threads simultaneously, so computeHash() must be thread-safe.
    if( !m_hash && dynamic )
      computeHash();
    return m_hash;
  }

  int m_unique;
  IndexedString m_identifier;
  uint m_refCount;

  START_APPENDED_LISTS_STATIC(IdentifierPrivate)

  APPENDED_LIST_FIRST_STATIC(IndexedTypeIdentifier, templateIdentifiers)

  END_APPENDED_LISTS_STATIC(templateIdentifiers)

  uint itemSize() const
  {
    return sizeof(IdentifierPrivate<false>) + lastOffsetBehind();
  }

  void computeHash() const
  {
    Q_ASSERT(dynamic);
    //this must stay thread-safe(may be called by multiple threads at a time)
    //The thread-safety is given because all threads will have the same result, and it will only be written once at the end.
    KDevHash kdevhash;
    kdevhash << m_identifier.hash() << m_unique;
    FOREACH_FUNCTION_STATIC(const IndexedTypeIdentifier& templateIdentifier, templateIdentifiers)
      kdevhash << IndexedTypeIdentifier(templateIdentifier).hash();
    m_hash = kdevhash;
  }

  mutable uint m_hash;
};

typedef IdentifierPrivate<true> DynamicIdentifierPrivate;
typedef IdentifierPrivate<false> ConstantIdentifierPrivate;

struct IdentifierItemRequest
{
  IdentifierItemRequest(const DynamicIdentifierPrivate& identifier)
    : m_identifier(identifier)
  {
    identifier.hash(); //Make sure the hash is valid by calling this
  }

  enum {
    AverageSize = sizeof(IdentifierPrivate<false>)+4
  };

  //Should return the hash-value associated with this request(For example the hash of a string)
  uint hash() const
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
    return (bool)item->m_refCount;
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

using IdentifierRepository = RepositoryManager< ItemRepository<ConstantIdentifierPrivate, IdentifierItemRequest>, false>;
static IdentifierRepository& identifierRepository()
{
  static IdentifierRepository identifierRepositoryObject("Identifier Repository");
  return identifierRepositoryObject;
}

static uint emptyConstantIdentifierPrivateIndex()
{
  static const uint index = identifierRepository()->index(DynamicIdentifierPrivate());
  return index;
}

static const ConstantIdentifierPrivate* emptyConstantIdentifierPrivate()
{
  static const ConstantIdentifierPrivate item;
  return &item;
}

bool IndexedIdentifier::isEmpty() const
{
  return index == emptyConstantIdentifierPrivateIndex();
}


/**
 * Before something is modified in QualifiedIdentifierPrivate, it must be made sure that
 * it is private to the QualifiedIdentifier it is used in(@see QualifiedIdentifier::prepareWrite)
 */
template<bool dynamic>
class QualifiedIdentifierPrivate
{
public:
  QualifiedIdentifierPrivate()
    : m_explicitlyGlobal(false)
    , m_isExpression(false)
    , m_hash(0)
    , m_refCount(0)
  {
  }

  template<bool rhsDynamic>
  QualifiedIdentifierPrivate(const QualifiedIdentifierPrivate<rhsDynamic>& rhs)
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

  bool m_explicitlyGlobal:1;
  bool m_isExpression:1;
  mutable uint m_hash;
  uint m_refCount;

  START_APPENDED_LISTS_STATIC(QualifiedIdentifierPrivate)

  APPENDED_LIST_FIRST_STATIC(IndexedIdentifier, identifiers)

  END_APPENDED_LISTS_STATIC(identifiers)

  uint itemSize() const
  {
    return sizeof(QualifiedIdentifierPrivate<false>) + lastOffsetBehind();
  }

  //Constructs m_identifiers
  void splitIdentifiers( const QString& str, int start )
  {
    Q_ASSERT(dynamic);
    uint currentStart = start;

    while( currentStart < (uint)str.length() ) {
      identifiersList.append(IndexedIdentifier(Identifier( str, currentStart, &currentStart )));
      while( currentStart < (uint)str.length() && (str[currentStart] == ' ' ) )
        ++currentStart;
      currentStart += 2; //Skip "::"
    }
  }

  inline void clearHash() const
  {
    m_hash = 0;
  }

  uint hash() const
  {
    if( m_hash == 0 )
    {
      uint mhash = 0;
      FOREACH_FUNCTION_STATIC( const IndexedIdentifier& identifier, identifiers )
        mhash = 11*mhash + Identifier(identifier).hash();

      if(mhash != m_hash)
        m_hash = mhash;//The local class may be in read-only memory, so only  change m_hash if it's actually a change
    }
    return m_hash;
  }
};

typedef QualifiedIdentifierPrivate<true> DynamicQualifiedIdentifierPrivate;
typedef QualifiedIdentifierPrivate<false> ConstantQualifiedIdentifierPrivate;

struct QualifiedIdentifierItemRequest
{
  QualifiedIdentifierItemRequest(const DynamicQualifiedIdentifierPrivate& identifier)
    : m_identifier(identifier)
  {
    identifier.hash(); //Make sure the hash is valid by calling this
  }

  enum {
    AverageSize = sizeof(QualifiedIdentifierPrivate<false>)+8
  };

  //Should return the hash-value associated with this request(For example the hash of a string)
  uint hash() const
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
    Q_ASSERT(shouldDoDUChainReferenceCounting(((char*)item) + (itemSize()-1)));
    new (item) ConstantQualifiedIdentifierPrivate(m_identifier);
  }

  static bool persistent(const ConstantQualifiedIdentifierPrivate* item)
  {
    return (bool)item->m_refCount;
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

using QualifiedIdentifierRepository = RepositoryManager< ItemRepository<ConstantQualifiedIdentifierPrivate, QualifiedIdentifierItemRequest>, false>;

static QualifiedIdentifierRepository& qualifiedidentifierRepository()
{
  static QualifiedIdentifierRepository repo("Qualified Identifier Repository", 1, [] () -> AbstractRepositoryManager* { return &identifierRepository(); });
  return repo;
}

static uint emptyConstantQualifiedIdentifierPrivateIndex()
{
  static const uint index = qualifiedidentifierRepository()->index(DynamicQualifiedIdentifierPrivate());
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
  cd = identifierRepository()->itemFromIndex(index);
}

Identifier::Identifier(const IndexedString& str)
  : m_index(0)
  , dd(new IdentifierPrivate<true>)
{
  dd->m_identifier = str;
}

Identifier::Identifier(const QString& id, uint start, uint* takenRange)
  : m_index(0)
  , dd(new IdentifierPrivate<true>)
{
  ///Extract template-parameters
  ParamIterator paramIt("<>:", id, start);
  dd->m_identifier = IndexedString(paramIt.prefix().trimmed());
  while( paramIt ) {
    appendTemplateIdentifier( IndexedTypeIdentifier(IndexedQualifiedIdentifier(QualifiedIdentifier(*paramIt))) );
    ++paramIt;
  }

  if( takenRange )
    *takenRange = paramIt.position();
}

Identifier::Identifier()
  : m_index(emptyConstantIdentifierPrivateIndex())
  , cd(emptyConstantIdentifierPrivate())
{
}

Identifier& Identifier::operator=(const Identifier& rhs)
{
  if(dd == rhs.dd && cd == rhs.cd)
    return *this;

  if(!m_index)
    delete dd;
  dd = 0;

  rhs.makeConstant();
  cd = rhs.cd;
  m_index = rhs.m_index;
  Q_ASSERT(cd);
  return *this;
}

Identifier::~Identifier()
{
  if(!m_index)
    delete dd;
}

bool Identifier::nameEquals(const Identifier& rhs) const
{
  return identifier() == rhs.identifier();
}

uint Identifier::hash() const
{
  if(!m_index)
    return dd->hash();
  else
    return cd->hash();
}

bool Identifier::isEmpty() const
{
  if(!m_index)
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
  if(!m_index)
    return dd->m_unique;
  else
    return cd->m_unique;
}

int Identifier::uniqueToken() const
{
  if(!m_index)
    return dd->m_unique;
  else
    return cd->m_unique;
}

void Identifier::setUnique(int token)
{
  prepareWrite();
  dd->m_unique = token;
}

const IndexedString Identifier::identifier() const
{
  if(!m_index)
    return dd->m_identifier;
  else
    return cd->m_identifier;
}

void Identifier::setIdentifier(const QString& identifier)
{
  prepareWrite();
  dd->m_identifier = IndexedString(identifier);
}

void Identifier::setIdentifier(const IndexedString& identifier)
{
  prepareWrite();
  dd->m_identifier = identifier;
}

IndexedTypeIdentifier Identifier::templateIdentifier(int num) const
{
  if(!m_index)
    return dd->templateIdentifiers()[num];
  else
    return cd->templateIdentifiers()[num];
}

uint Identifier::templateIdentifiersCount() const
{
  if(!m_index)
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

void Identifier::setTemplateIdentifiers(const QList<IndexedTypeIdentifier>& templateIdentifiers)
{
  prepareWrite();
  dd->templateIdentifiersList.clear();
  foreach(const IndexedTypeIdentifier& id, templateIdentifiers)
    dd->templateIdentifiersList.append(id);
}

QString Identifier::toString() const
{
  QString ret = identifier().str();

  if (templateIdentifiersCount()) {
    ret.append("< ");
    for (uint i = 0; i < templateIdentifiersCount(); ++i) {
      ret.append(templateIdentifier(i).toString());
      if (i != templateIdentifiersCount() - 1)
        ret.append(", ");
    }
    ret.append(" >");
  }

  return ret;
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
  if(m_index)
    return;
  m_index = identifierRepository()->index( IdentifierItemRequest(*dd) );
  delete dd;
  cd = identifierRepository()->itemFromIndex( m_index );
}

void Identifier::prepareWrite()
{
  if(m_index) {
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
  if(m_index)
    return true;
  else
    return (bool)qualifiedidentifierRepository()->findIndex( QualifiedIdentifierItemRequest(*dd) );
}

QualifiedIdentifier::QualifiedIdentifier(uint index)
  : m_index(index)
  , cd( qualifiedidentifierRepository()->itemFromIndex(index) )
{
}

QualifiedIdentifier::QualifiedIdentifier(const QString& id, bool isExpression)
  : m_index(0)
  , dd(new DynamicQualifiedIdentifierPrivate)
{
  if(isExpression) {
    setIsExpression(true);
    if(!id.isEmpty()) {
      //Prevent tokenization, since we may lose information there
      Identifier finishedId;
      finishedId.setIdentifier(id);
      push(finishedId);
    }
  }else{
    if (id.startsWith("::")) {
      dd->m_explicitlyGlobal = true;
      dd->splitIdentifiers(id, 2);
    } else {
      dd->m_explicitlyGlobal = false;
      dd->splitIdentifiers(id, 0);
    }
  }
}

QualifiedIdentifier::QualifiedIdentifier(const Identifier& id)
  : m_index(0)
  , dd(new DynamicQualifiedIdentifierPrivate)
{
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

QualifiedIdentifier::~QualifiedIdentifier()
{
  if(!m_index)
    delete dd;
}

QualifiedIdentifier::QualifiedIdentifier(const QualifiedIdentifier& id)
{
  if(id.m_index) {
    m_index = id.m_index;
    cd = id.cd;
  }else{
    m_index = 0;
    dd = new QualifiedIdentifierPrivate<true>(*id.dd);
  }
}

QStringList QualifiedIdentifier::toStringList() const
{
  QStringList ret;
  if (explicitlyGlobal())
    ret.append(QString());

  if(m_index) {
    FOREACH_FUNCTION_STATIC(const IndexedIdentifier& index, cd->identifiers)
      ret << index.identifier().toString();
  }else{
    FOREACH_FUNCTION_STATIC(const IndexedIdentifier& index, dd->identifiers)
      ret << index.identifier().toString();
  }

  return ret;
}

QString QualifiedIdentifier::toString(bool ignoreExplicitlyGlobal) const
{
  QString ret;
  if( !ignoreExplicitlyGlobal && explicitlyGlobal() )
    ret = "::";

  bool first = true;
  if(m_index) {
    FOREACH_FUNCTION_STATIC(const IndexedIdentifier& index, cd->identifiers)
    {
      if( !first )
        ret += "::";
      else
        first = false;

      ret += index.identifier().toString();
    }
  }else{
    FOREACH_FUNCTION_STATIC(const IndexedIdentifier& index, dd->identifiers)
    {
      if( !first )
        ret += "::";
      else
        first = false;

      ret += index.identifier().toString();
    }
  }

  return ret;
}

QualifiedIdentifier QualifiedIdentifier::merge(const QualifiedIdentifier& base) const
{
  QualifiedIdentifier ret(base);
  ret.prepareWrite();

  if(m_index)
    ret.dd->identifiersList.append(cd->identifiers(), cd->identifiersSize());
  else
    ret.dd->identifiersList.append(dd->identifiers(), dd->identifiersSize());

  if( explicitlyGlobal() )
    ret.setExplicitlyGlobal(true);

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

bool QualifiedIdentifier::isExpression() const
{
  if(m_index)
    return cd->m_isExpression;
  else
    return dd->m_isExpression;
}

void QualifiedIdentifier::setIsExpression(bool is)
{
  prepareWrite();
  dd->m_isExpression = is;
}

bool QualifiedIdentifier::explicitlyGlobal() const
{
  // True if started with "::"
  if(m_index)
    return cd->m_explicitlyGlobal;
  else
    return dd->m_explicitlyGlobal;
}

void QualifiedIdentifier::setExplicitlyGlobal(bool eg)
{
  prepareWrite();
  dd->m_explicitlyGlobal = eg;
}

bool QualifiedIdentifier::sameIdentifiers(const QualifiedIdentifier& rhs) const
{
  if(m_index && rhs.m_index)
    return cd->listsEqual(*rhs.cd);
  else if(m_index && !rhs.m_index)
    return cd->listsEqual(*rhs.dd);
  else if(!m_index && !rhs.m_index)
    return dd->listsEqual(*rhs.dd);
  else
    return dd->listsEqual(*rhs.cd);
}

bool QualifiedIdentifier::operator==(const QualifiedIdentifier& rhs) const
{
  if( cd == rhs.cd )
    return true;
  return hash() == rhs.hash() && sameIdentifiers(rhs);
}

bool QualifiedIdentifier::operator!=(const QualifiedIdentifier& rhs) const
{
  return !operator==(rhs);
}

QualifiedIdentifier& QualifiedIdentifier::operator=(const QualifiedIdentifier& rhs)
{
  if(!m_index)
    delete dd;
  rhs.makeConstant();
  cd = rhs.cd;
  m_index = rhs.m_index;
  return *this;
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
  Visitor(KDevVarLengthArray<QualifiedIdentifier>& target, uint hash)
    : target(target)
    , hash(hash)
  {
  }

  bool operator()(const ConstantQualifiedIdentifierPrivate* item, uint index) const
  {
    if(item->m_hash == hash)
      target.append(QualifiedIdentifier(index));
    return true;
  }

  KDevVarLengthArray<QualifiedIdentifier>& target;
  const uint hash;
};

void QualifiedIdentifier::findByHash(uint hash, KDevVarLengthArray<QualifiedIdentifier>& target)
{
  Visitor v(target, hash);
  qualifiedidentifierRepository()->visitItemsWithHash<Visitor>(v, hash);
}

uint QualifiedIdentifier::hash() const
{
  if(m_index)
    return cd->hash();
  else
    return dd->hash();
}

uint qHash(const IndexedTypeIdentifier& id)
{
  return id.hash();
}

uint qHash(const QualifiedIdentifier& id)
{
  return id.hash();
}

uint qHash(const Identifier& id)
{
  return id.hash();
}

bool QualifiedIdentifier::isQualified() const
{
  return count() > 1 || explicitlyGlobal();
}

void QualifiedIdentifier::push(const Identifier& id)
{
  if(id.isEmpty())
    return;

  prepareWrite();

  dd->identifiersList.append(IndexedIdentifier(id));
}

void QualifiedIdentifier::push(const QualifiedIdentifier& id)
{
  if(id.isEmpty())
    return;

  prepareWrite();
  id.makeConstant();

  dd->identifiersList.append(id.cd->identifiers(), id.cd->identifiersSize());
}

void QualifiedIdentifier::pop()
{
  prepareWrite();
  if(!dd->identifiersSize())
    return;
  dd->identifiersList.resize(dd->identifiersList.size()-1);
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
  if(m_index)
    return cd->identifiersSize() == 0;
  else
    return dd->identifiersSize() == 0;
}

int QualifiedIdentifier::count() const
{
  if(m_index)
    return cd->identifiersSize();
  else
    return dd->identifiersSize();
}

Identifier QualifiedIdentifier::first() const
{
  if( (m_index && cd->identifiersSize() == 0) || (!m_index && dd->identifiersSize() == 0) )
    return Identifier();
  else
    return at(0);
}

Identifier QualifiedIdentifier::last() const
{
  uint c = count();
  if(c)
    return at(c-1);
  else
    return Identifier();
}

Identifier QualifiedIdentifier::top() const
{
  return last();
}

QualifiedIdentifier QualifiedIdentifier::mid(int pos, int len) const
{
  QualifiedIdentifier ret;
  if( pos == 0 )
    ret.setExplicitlyGlobal(explicitlyGlobal());

  int cnt = (int)count();

  if( len == -1 )
    len = cnt - pos;

  if( pos+len > cnt )
    len -= cnt - (pos+len);

  for( int a = pos; a < pos+len; a++ )
    ret.push(at(a));

  return ret;
}

const Identifier QualifiedIdentifier::at(int i) const
{
  if(m_index) {
    Q_ASSERT(i >= 0 && i < (int)cd->identifiersSize());
    return cd->identifiers()[i];
  }else{
    Q_ASSERT(i >= 0 && i < (int)dd->identifiersSize());
    return dd->identifiers()[i];
  }
}

void QualifiedIdentifier::makeConstant() const
{
  if(m_index)
    return;
  m_index = qualifiedidentifierRepository()->index( QualifiedIdentifierItemRequest(*dd) );
  delete dd;
  cd = qualifiedidentifierRepository()->itemFromIndex( m_index );
}

void QualifiedIdentifier::prepareWrite()
{

  if(m_index) {
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

uint IndexedTypeIdentifier::hash() const
{
    quint32 bitfields = m_isConstant
      | (m_isReference << 1)
      | (m_isRValue << 2)
      | (m_isVolatile << 3)
      | (m_pointerDepth << 4)
      | (m_pointerConstMask << 9);
    return KDevHash() << m_identifier.getIndex() << bitfields;
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
  for(int s = depth; s < (int)m_pointerDepth; ++s)
    setIsConstPointer(s, false);

  m_pointerDepth = depth;
}

bool IndexedTypeIdentifier::isConstPointer(int depthNumber) const
{
  return m_pointerConstMask & (1 << depthNumber);
}

void IndexedTypeIdentifier::setIsConstPointer(int depthNumber, bool constant)
{
  if(constant)
    m_pointerConstMask |= (1 << depthNumber);
  else
    m_pointerConstMask &= (~(1 << depthNumber));
}

QString IndexedTypeIdentifier::toString(bool ignoreExplicitlyGlobal) const
{
  QString ret;
  if(isConstant())
    ret += "const ";
  if(isVolatile())
    ret += "volatile ";

  ret += m_identifier.identifier().toString(ignoreExplicitlyGlobal);
  for(int a = 0; a < pointerDepth(); ++a) {
    ret += '*';
    if( isConstPointer(a) )
      ret += "const";
  }

  if(isRValue())
    ret += "&&";
  else if(isReference())
    ret += '&';
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

IndexedTypeIdentifier::IndexedTypeIdentifier(const QString& identifier, bool isExpression)
  : m_identifier(QualifiedIdentifier(identifier, isExpression))
  , m_isConstant(false)
  , m_isReference(false)
  , m_isRValue(false)
  , m_isVolatile(false)
  , m_pointerDepth(0)
  , m_pointerConstMask(0)
{ }

IndexedIdentifier::IndexedIdentifier()
  : index(emptyConstantIdentifierPrivateIndex())
{
  if(shouldDoDUChainReferenceCounting(this)) {
    QMutexLocker lock(identifierRepository()->mutex());
    increase(identifierRepository()->dynamicItemFromIndexSimple(index)->m_refCount, index);
  }
}

IndexedIdentifier::IndexedIdentifier(const Identifier& id)
  : index(id.index())
{
  if(shouldDoDUChainReferenceCounting(this)) {
    QMutexLocker lock(identifierRepository()->mutex());
    increase(identifierRepository()->dynamicItemFromIndexSimple(index)->m_refCount, index);
  }
}

IndexedIdentifier::IndexedIdentifier(const IndexedIdentifier& rhs)
  : index(rhs.index)
{
  if(shouldDoDUChainReferenceCounting(this)) {
    QMutexLocker lock(identifierRepository()->mutex());
    increase(identifierRepository()->dynamicItemFromIndexSimple(index)->m_refCount, index);
  }
}

IndexedIdentifier::~IndexedIdentifier()
{
  if(shouldDoDUChainReferenceCounting(this)) {
    QMutexLocker lock(identifierRepository()->mutex());
    decrease(identifierRepository()->dynamicItemFromIndexSimple(index)->m_refCount, index);
  }
}

IndexedIdentifier& IndexedIdentifier::operator=(const Identifier& id)
{
  if(shouldDoDUChainReferenceCounting(this)) {
    QMutexLocker lock(identifierRepository()->mutex());
    decrease(identifierRepository()->dynamicItemFromIndexSimple(index)->m_refCount, index);
  }

  index = id.index();

  if(shouldDoDUChainReferenceCounting(this)) {
    QMutexLocker lock(identifierRepository()->mutex());
    increase(identifierRepository()->dynamicItemFromIndexSimple(index)->m_refCount, index);
  }
  return *this;
}

IndexedIdentifier& IndexedIdentifier::operator=(const IndexedIdentifier& id)
{
  if(shouldDoDUChainReferenceCounting(this)) {
    QMutexLocker lock(identifierRepository()->mutex());
    decrease(identifierRepository()->dynamicItemFromIndexSimple(index)->m_refCount, index);
  }

  index = id.index;

  if(shouldDoDUChainReferenceCounting(this)) {
    QMutexLocker lock(identifierRepository()->mutex());
    increase(identifierRepository()->dynamicItemFromIndexSimple(index)->m_refCount, index);
  }
  return *this;
}

bool IndexedIdentifier::operator==(const IndexedIdentifier& rhs) const
{
  return index == rhs.index;
}

bool IndexedIdentifier::operator!=(const IndexedIdentifier& rhs) const
{
  return index != rhs.index;
}

bool IndexedIdentifier::operator==(const Identifier& id) const
{
  return index == id.index();
}

Identifier IndexedIdentifier::identifier() const
{
  return Identifier(index);
}

IndexedIdentifier::operator Identifier() const
{
  return Identifier(index);
}

bool IndexedQualifiedIdentifier::isValid() const
{
  return index != emptyConstantQualifiedIdentifierPrivateIndex();
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

IndexedQualifiedIdentifier::IndexedQualifiedIdentifier()
  : index(emptyConstantQualifiedIdentifierPrivateIndex())
{
  ifDebug( kDebug() << "(" << ++cnt << ")" << identifier().toString() << index; )

  if(shouldDoDUChainReferenceCounting(this)) {
    ifDebug( kDebug() << "increasing"; )

    //kDebug() << "(" << ++cnt << ")" << this << identifier().toString() << "inc" << index;
    QMutexLocker lock(qualifiedidentifierRepository()->mutex());
    increase(qualifiedidentifierRepository()->dynamicItemFromIndexSimple(index)->m_refCount, index);
  }
}

IndexedQualifiedIdentifier::IndexedQualifiedIdentifier(const QualifiedIdentifier& id)
  : index(id.index())
{
  ifDebug( kDebug() << "(" << ++cnt << ")" << identifier().toString() << index; )

  if(shouldDoDUChainReferenceCounting(this)) {
    ifDebug( kDebug() << "increasing"; )
    QMutexLocker lock(qualifiedidentifierRepository()->mutex());
    increase(qualifiedidentifierRepository()->dynamicItemFromIndexSimple(index)->m_refCount, index);
  }
}

IndexedQualifiedIdentifier::IndexedQualifiedIdentifier(const IndexedQualifiedIdentifier& id)
  : index(id.index)
{
  ifDebug( kDebug() << "(" << ++cnt << ")" << identifier().toString() << index; )

  if(shouldDoDUChainReferenceCounting(this)) {
    ifDebug( kDebug() << "increasing"; )

    QMutexLocker lock(qualifiedidentifierRepository()->mutex());
    increase(qualifiedidentifierRepository()->dynamicItemFromIndexSimple(index)->m_refCount, index);
  }
}

IndexedQualifiedIdentifier& IndexedQualifiedIdentifier::operator=(const QualifiedIdentifier& id)
{
  ifDebug( kDebug() << "(" << ++cnt << ")" << identifier().toString() << index; )

  if(shouldDoDUChainReferenceCounting(this)) {
    QMutexLocker lock(qualifiedidentifierRepository()->mutex());

    ifDebug( kDebug() << "decreasing"; )
    decrease(qualifiedidentifierRepository()->dynamicItemFromIndexSimple(index)->m_refCount, index);

    index = id.index();

    ifDebug( kDebug() << index << "increasing"; )
    increase(qualifiedidentifierRepository()->dynamicItemFromIndexSimple(index)->m_refCount, index);
  } else {
    index = id.index();
  }

  return *this;
}

IndexedQualifiedIdentifier& IndexedQualifiedIdentifier::operator=(const IndexedQualifiedIdentifier& rhs)
{

  ifDebug( kDebug() << "(" << ++cnt << ")" << identifier().toString() << index; )

  if(shouldDoDUChainReferenceCounting(this)) {
    QMutexLocker lock(qualifiedidentifierRepository()->mutex());
    ifDebug( kDebug() << "decreasing"; )

    decrease(qualifiedidentifierRepository()->dynamicItemFromIndexSimple(index)->m_refCount, index);

    index = rhs.index;

    ifDebug( kDebug() << index << "increasing"; )
    increase(qualifiedidentifierRepository()->dynamicItemFromIndexSimple(index)->m_refCount, index);
  } else {
    index = rhs.index;
  }

  return *this;
}

IndexedQualifiedIdentifier::~IndexedQualifiedIdentifier()
{
  ifDebug( kDebug() << "(" << ++cnt << ")" << identifier().toString() << index; )
  if(shouldDoDUChainReferenceCounting(this)) {
    ifDebug( kDebug() << index << "decreasing"; )
    QMutexLocker lock(qualifiedidentifierRepository()->mutex());
    decrease(qualifiedidentifierRepository()->dynamicItemFromIndexSimple(index)->m_refCount, index);
  }
}

bool IndexedQualifiedIdentifier::operator==(const IndexedQualifiedIdentifier& rhs) const
{
  return index == rhs.index;
}

bool IndexedQualifiedIdentifier::operator==(const QualifiedIdentifier& id) const
{
  return index == id.index();
}

QualifiedIdentifier IndexedQualifiedIdentifier::identifier() const
{
  return QualifiedIdentifier(index);
}

IndexedQualifiedIdentifier::operator QualifiedIdentifier() const
{
  return QualifiedIdentifier(index);
}

void initIdentifierRepository() {
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

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on

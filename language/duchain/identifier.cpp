/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>

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

#include <QHash>
#include "stringhelpers.h"
#include "indexedstring.h"
#include "appendedlist_static.h"
#include "repositories/itemrepository.h"

namespace KDevelop
{
template<bool dynamic = false>
class IdentifierPrivate
{
public:
  IdentifierPrivate() : m_unique(0), m_hash(0){
  }

  //Flags the stored hash-value invalid
  void clearHash() { //This is always called on an object private to an Identifier, so there is no threading-problem.
    Q_ASSERT(dynamic);
    m_hash = 0;
  }

  uint hash() const { //Since this only needs reading and the data needs not to be private, this may be called by multiple threads simultaneously, so computeHash() must be thread-safe.
    if( !m_hash && dynamic )
      computeHash();
    return m_hash;
  }

  int m_unique;
  IndexedString m_identifier;

  START_APPENDED_LISTS(IdentifierPrivate)
  
  APPENDED_LIST_FIRST(uint, templateIdentifiers)

  END_APPENDED_LISTS(templateIdentifiers)
  
    void computeHash() const {
      Q_ASSERT(dynamic);
      //this must stay thread-safe(may be called by multiple threads at a time)
      //The thread-safety is given because all threads will have the same result, and it will only be written once at the end.
      uint hash = m_identifier.hash();
      FOREACH_FUNCTION(uint templateIdentifier, templateIdentifiers)
        hash = hash * 13 + TypeIdentifier(templateIdentifier).hash();
      m_hash += m_unique;
      m_hash = hash;
    }
    
  
    mutable uint m_hash;
};

typedef IdentifierPrivate<true> DynamicIdentifierPrivate;
typedef IdentifierPrivate<false> ConstantIdentifierPrivate;

struct IdentifierItemRequest {
  IdentifierItemRequest(const DynamicIdentifierPrivate& identifier) : m_identifier(identifier) {
    identifier.hash(); //Make sure the hash is valid by calling this
  }

  enum {
    AverageSize = sizeof(IdentifierPrivate<false>)+4
  };

  //Should return the hash-value associated with this request(For example the hash of a string)
  unsigned int hash() const {
    return m_identifier.hash();
  }
  
  //Should return the size of an item created with createItem
  size_t itemSize() const {
      return m_identifier.completeSize();
  }
  //Should create an item where the information of the requested item is permanently stored. The pointer
  //@param item equals an allocated range with the size of itemSize().
  void createItem(ConstantIdentifierPrivate* item) const {
    item->m_hash = m_identifier.m_hash;
    item->m_unique = m_identifier.m_unique;
    item->m_identifier = m_identifier.m_identifier;
    item->copyListsFrom(m_identifier);
  }
  
  //Should return whether the here requested item equals the given item
  bool equals(const ConstantIdentifierPrivate* item) const {
    return item->m_hash == m_identifier.m_hash && item->m_unique == m_identifier.m_unique && item->m_identifier == m_identifier.m_identifier && m_identifier.listsEqual(*item);
  }
  
  const DynamicIdentifierPrivate& m_identifier;
};

ItemRepository<ConstantIdentifierPrivate, IdentifierItemRequest> identifierRepository("Identifier Repository");
uint emptyConstantIdentifierPrivateIndex = identifierRepository.index(DynamicIdentifierPrivate());
const ConstantIdentifierPrivate* emptyConstantIdentifierPrivate = identifierRepository.itemFromIndex(emptyConstantIdentifierPrivateIndex);

///Before something is modified in QualifiedIdentifierPrivate, it must be made sure that it is private to the QualifiedIdentifier it is used in(@see QualifiedIdentifier::prepareWrite)
template<bool dynamic>
class QualifiedIdentifierPrivate
{
public:
  QualifiedIdentifierPrivate() : m_explicitlyGlobal(false), m_isExpression(false), m_isConstant(false), m_isReference(false), m_pointerDepth(0), m_pointerConstantMask(0), m_hash(0) {
  }
  bool m_explicitlyGlobal:1;
  bool m_isExpression:1;
  bool m_isConstant:1; //Data for TypeIdentifier, stored here.
  bool m_isReference:1; //Data for TypeIdentifier, stored here.
  uchar m_pointerDepth;
  uint m_pointerConstantMask; //Stores in a mask for each pointer-depth whether it is constant. Supports only max. 32 levels.
  mutable uint m_hash;

  START_APPENDED_LISTS(QualifiedIdentifierPrivate)
  
  APPENDED_LIST_FIRST(uint, identifiers)

  END_APPENDED_LISTS(identifiers)
  
  //Constructs m_identifiers
  void splitIdentifiers( const QString& str, int start )
  {
    Q_ASSERT(dynamic);
    uint currentStart = start;

    while( currentStart < (uint)str.length() ) {
      identifiersList.append(Identifier( str, currentStart, &currentStart ).index());
      while( currentStart < (uint)str.length() && (str[currentStart] == ' ' ) )
        ++currentStart;
      currentStart += 2; //Skip "::"
    }
  }

  inline void clearHash() const {
    m_hash = 0;
  }

  uint hash() const
  {
    if( m_hash == 0 )
    {
      uint mhash = 0;
      FOREACH_FUNCTION( uint identifier, identifiers )
        mhash = 11*mhash + Identifier(identifier).hash();

      mhash += 17 * m_isConstant + 19 * m_isReference + 23 * m_pointerDepth + 31 * m_pointerConstantMask;

      m_hash = mhash;
    }
    return m_hash;
  }
};

typedef QualifiedIdentifierPrivate<true> DynamicQualifiedIdentifierPrivate;
typedef QualifiedIdentifierPrivate<false> ConstantQualifiedIdentifierPrivate;

struct QualifiedIdentifierItemRequest {
  QualifiedIdentifierItemRequest(const DynamicQualifiedIdentifierPrivate& identifier) : m_identifier(identifier) {
    identifier.hash(); //Make sure the hash is valid by calling this
  }

  enum {
    AverageSize = sizeof(QualifiedIdentifierPrivate<false>)+8
  };

  //Should return the hash-value associated with this request(For example the hash of a string)
  unsigned int hash() const {
    return m_identifier.hash();
  }
  
  //Should return the size of an item created with createItem
  size_t itemSize() const {
      return m_identifier.completeSize();
  }
  //Should create an item where the information of the requested item is permanently stored. The pointer
  //@param item equals an allocated range with the size of itemSize().
  void createItem(ConstantQualifiedIdentifierPrivate* item) const {
    item->m_explicitlyGlobal = m_identifier.m_explicitlyGlobal;
    item->m_isExpression = m_identifier.m_isExpression;
    item->m_isConstant = m_identifier.m_isConstant;
    item->m_isReference = m_identifier.m_isReference;
    item->m_pointerDepth = m_identifier.m_pointerDepth;
    item->m_pointerConstantMask = m_identifier.m_pointerConstantMask;
    item->m_hash = m_identifier.m_hash;
    item->copyListsFrom(m_identifier);
  }
  
  //Should return whether the here requested item equals the given item
  bool equals(const ConstantQualifiedIdentifierPrivate* item) const {
    return item->m_explicitlyGlobal == m_identifier.m_explicitlyGlobal &&
    item->m_isExpression == m_identifier.m_isExpression &&
    item->m_isConstant == m_identifier.m_isConstant &&
    item->m_isReference == m_identifier.m_isReference &&
    item->m_pointerDepth == m_identifier.m_pointerDepth &&
    item->m_pointerConstantMask == m_identifier.m_pointerConstantMask &&
    item->m_hash == m_identifier.m_hash &&
    m_identifier.listsEqual(*item);
  }
  
  const DynamicQualifiedIdentifierPrivate& m_identifier;
};

ItemRepository<ConstantQualifiedIdentifierPrivate, QualifiedIdentifierItemRequest> qualifiedIdentifierRepository("Qualified Identifier Repository");

uint emptyConstantQualifiedIdentifierPrivateIndex = qualifiedIdentifierRepository.index(DynamicQualifiedIdentifierPrivate());

const ConstantQualifiedIdentifierPrivate* emptyConstantQualifiedIdentifierPrivate = qualifiedIdentifierRepository.itemFromIndex(emptyConstantQualifiedIdentifierPrivateIndex);

uint QualifiedIdentifier::combineHash(uint leftHash, uint /*leftSize*/, Identifier appendIdentifier) {
  return 11*leftHash + appendIdentifier.hash();
}

Identifier::Identifier(const Identifier& rhs) {
  rhs.makeConstant();
  cd = rhs.cd;
  m_index = rhs.m_index;
}

Identifier::Identifier(uint index) : m_index(index) {
  Q_ASSERT(m_index);
  cd = identifierRepository.itemFromIndex(index);
}

Identifier::~Identifier() {
  if(!m_index)
    delete dd;
}

bool Identifier::nameEquals(const Identifier& rhs) const {
  return identifier() == rhs.identifier();
}

uint Identifier::hash() const {
  if(!m_index)
    return dd->hash();
  else
    return cd->hash();
}

bool Identifier::isEmpty() const {
  if(!m_index)
    return dd->m_identifier.isEmpty() && dd->templateIdentifiersSize() == 0;
  else
    return cd->m_identifier.isEmpty() && cd->templateIdentifiersSize() == 0;
}

Identifier::Identifier(const IndexedString& str)
  : m_index(0), dd(new IdentifierPrivate<true>)
{
  dd->m_identifier = str;
}


Identifier::Identifier(const QString& id, uint start, uint* takenRange)
  : m_index(0), dd(new IdentifierPrivate<true>)
{
  ///Extract template-parameters
  ParamIterator paramIt("<>:", id, start);
  dd->m_identifier = IndexedString(paramIt.prefix().trimmed());
  while( paramIt ) {
    appendTemplateIdentifier( TypeIdentifier(*paramIt) );
    ++paramIt;
  }

  if( takenRange )
    *takenRange = paramIt.position();
}

Identifier::Identifier()
  : m_index(emptyConstantIdentifierPrivateIndex), cd(emptyConstantIdentifierPrivate)
{
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

const QString Identifier::identifier() const
{
  if(!m_index)
    return dd->m_identifier.str();
  else
    return cd->m_identifier.str();
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

TypeIdentifier Identifier::templateIdentifier(int num) const
{
  if(!m_index)
    return TypeIdentifier(dd->templateIdentifiers()[num]);
  else
    return TypeIdentifier(cd->templateIdentifiers()[num]);
}

uint Identifier::templateIdentifiersCount() const
{
  if(!m_index)
    return dd->templateIdentifiersSize();
  else
    return cd->templateIdentifiersSize();
}

void Identifier::appendTemplateIdentifier(const TypeIdentifier& identifier)
{
  prepareWrite();
  dd->templateIdentifiersList.append(identifier.index());
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

void Identifier::setTemplateIdentifiers(const QList<TypeIdentifier>& templateIdentifiers)
{
  prepareWrite();
  dd->templateIdentifiersList.clear();
  foreach(const TypeIdentifier& id, templateIdentifiers)
    dd->templateIdentifiersList.append(id.index());
}

QString Identifier::toString() const
{
  if (!this) {
    return "(null identifier)";
  }

  QString ret = identifier();

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

uint QualifiedIdentifier::index() const {
  makeConstant();
  Q_ASSERT(m_index);
  return m_index;
}

void Identifier::makeConstant() const {
  if(m_index)
    return;
  m_index = identifierRepository.index( IdentifierItemRequest(*dd) );
  delete dd;
  cd = identifierRepository.itemFromIndex( m_index );
}

void Identifier::prepareWrite() {
  
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

QualifiedIdentifier::QualifiedIdentifier(uint index) : m_index(index), cd( qualifiedIdentifierRepository.itemFromIndex(index) ) {
}

QualifiedIdentifier::QualifiedIdentifier(const QString& id)
  : m_index(0), dd(new DynamicQualifiedIdentifierPrivate)
{
  if (id.startsWith("::")) {
    dd->m_explicitlyGlobal = true;
    dd->splitIdentifiers(id, 2);
  } else {
    dd->m_explicitlyGlobal = false;
    dd->splitIdentifiers(id, 0);
  }
}

QualifiedIdentifier::QualifiedIdentifier(const Identifier& id)
  : m_index(0), dd(new DynamicQualifiedIdentifierPrivate)
{
  if (id.dd->m_identifier.str().isEmpty()) {
    dd->m_explicitlyGlobal = true;
  } else {
    dd->m_explicitlyGlobal = false;
    dd->identifiersList.append(id.index());
  }
}

QualifiedIdentifier::QualifiedIdentifier()
  : m_index(emptyConstantQualifiedIdentifierPrivateIndex), cd(emptyConstantQualifiedIdentifierPrivate)
{
}

QualifiedIdentifier::~QualifiedIdentifier() {
  if(!m_index)
    delete dd;
}

QualifiedIdentifier::QualifiedIdentifier(const QualifiedIdentifier& id)
{
  id.makeConstant();
  m_index = id.m_index;
  cd = id.cd;
  if( cd->m_pointerConstantMask || cd->m_isReference || cd->m_pointerDepth || cd->m_isConstant ) {
    //When copying from a type-identifier, do not share the d-pointer, because it contains invalid information.
    prepareWrite();
    dd->m_pointerConstantMask = 0;
    dd->m_isReference = 0;
    dd->m_pointerDepth = 0;
    dd->m_isConstant = 0;
  }
}
QStringList QualifiedIdentifier::toStringList() const
{
  QStringList ret;
  if (explicitlyGlobal())
    ret.append(QString());

  if(m_index) {
    FOREACH_FUNCTION(uint index, cd->identifiers)
      ret << Identifier(index).toString();
  }else{
    FOREACH_FUNCTION(uint index, dd->identifiers)
      ret << Identifier(index).toString();
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
    FOREACH_FUNCTION(uint index, cd->identifiers)
    {
      if( !first )
        ret += "::";
      else
        first = false;
      
      ret += Identifier(index).toString();
    }
  }else{
    FOREACH_FUNCTION(uint index, dd->identifiers)
    {
      if( !first )
        ret += "::";
      else
        first = false;
      
      ret += Identifier(index).toString();
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

QualifiedIdentifier QualifiedIdentifier::operator+(const QualifiedIdentifier& rhs) const {
  return rhs.merge(*this);
}

QualifiedIdentifier& QualifiedIdentifier::operator+=(const QualifiedIdentifier& rhs) {
  push(rhs);
  return *this;
}

QualifiedIdentifier QualifiedIdentifier::operator+(const Identifier& rhs) const {
  QualifiedIdentifier ret(*this);
  ret.push(rhs);
  return ret;
}

QualifiedIdentifier& QualifiedIdentifier::operator+=(const Identifier& rhs) {
  push(rhs);
  return *this;
}

// QualifiedIdentifier QualifiedIdentifier::strip(const QualifiedIdentifier & unwantedBase) const
// {
//   // Don't strip the top identifier
//   if (count() <= unwantedBase.count())
//     return *this;
// 
//   //Make sure this one starts with unwantedBase
//   for( int a = 0; a < unwantedBase.count(); a++ )
//     if( d->m_identifiers[a] != unwantedBase.d->m_identifiers[a] )
//       return *this;
//   
//   
//   QualifiedIdentifier ret;
//   ret.setExplicitlyGlobal(false);
//   ret.prepareWrite();
// 
//   int remove = unwantedBase.d->m_identifiers.count();
// 
//   ret.d->m_identifiers.append(&d->m_identifiers[remove], d->m_identifiers.size() - remove);
//   
//   return ret;
// }

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

bool QualifiedIdentifier::isSame(const QualifiedIdentifier& rhs, bool ignoreExplicitlyGlobal) const
{
  if( cd == rhs.cd )
    return true;

  if (!ignoreExplicitlyGlobal && (explicitlyGlobal() != rhs.explicitlyGlobal()))
    return false;

  if( isExpression() != rhs.isExpression() )
    return false;
  
  if( hash() != rhs.hash() )
    return false;
  
  return sameIdentifiers(rhs);
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

#if 0

QualifiedIdentifier::MatchTypes QualifiedIdentifier::match(const Identifier& rhs) const
{
  if( d->m_identifiers.isEmpty() )
    return NoMatch;

  if( d->m_identifiers[d->m_identifiers.size()-1] == rhs ) {
    if( d->m_identifiers.size() == 1 )
      return ExactMatch;
    else
      return EndsWith;
  }
    
  return NoMatch;
}

QualifiedIdentifier::MatchTypes QualifiedIdentifier::match(const QualifiedIdentifier& rhs) const
{
  const int difference = count() - rhs.count();
  if (difference == 0) {
    if (hash() == rhs.hash())
      return ExactMatch;
    else
      return NoMatch;

  } else if (difference > 0) {
    // eg 6 vs 4... we need at(2)
    if (rhs.explicitlyGlobal())
      return NoMatch;

    for( int a = difference; a < count(); a++ )
      if( rhs.d->m_identifiers[a-difference] != d->m_identifiers[a] )
        return NoMatch;
    
    return EndsWith;
  } else {
    if (explicitlyGlobal())
      return NoMatch;
      
    for( int a = -difference; a < rhs.count(); a++ )
        if( rhs.d->m_identifiers[a] != d->m_identifiers[a+difference] )
          return NoMatch;

    return TargetEndsWith;
  }

  return NoMatch;
}
#endif

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

uint QualifiedIdentifier::hash() const {
  if(m_index)
    return cd->hash();
  else
    return dd->hash();
}

uint qHash(const TypeIdentifier& id)
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

// QString Identifier::mangled() const
// { ///@todo work this over
//   static QRegExp simpleIdentifier("[a-zA-Z0-9_]*");
//   if (simpleIdentifier.exactMatch(d->m_identifier.str()))
//     return QString("%1%2").arg(d->m_identifier.str().length()).arg(d->m_identifier.str());
// 
//   // Get the encoded utf form
//   QString utf = QString::fromLatin1(d->m_identifier.str().toUtf8());
// 
//   return QString("U%1%2").arg(utf.length()).arg(utf);
// }

// QString QualifiedIdentifier::mangled() const
// {
//   if (isEmpty())
//     return QString();
// 
//   if (count() == 1)
//     return first().mangled();
// 
//   QString ret('Q');
// 
//   if (count() > 9)
//     ret += QString(",%1,").arg(count());
//   else
//     ret += count();
// 
//   for (int i = 0; i < count(); ++i)
//     ret += at(i).mangled();
// 
//   return ret;
// }

void QualifiedIdentifier::push(const Identifier& id)
{
  prepareWrite();
  
  dd->identifiersList.append(id.index());
}

void QualifiedIdentifier::push(const QualifiedIdentifier& id)
{
  prepareWrite();
  id.makeConstant();

  dd->identifiersList.append(id.cd->identifiers(), id.cd->identifiersSize());
}

void QualifiedIdentifier::pop()
{
  prepareWrite();
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
  if( (m_index && cd->identifiersSize() == 0) || dd->identifiersSize() == 0 )
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

QualifiedIdentifier QualifiedIdentifier::mid(int pos, int len) const {
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

void QualifiedIdentifier::makeConstant() const {
  if(m_index)
    return;
  m_index = qualifiedIdentifierRepository.index( QualifiedIdentifierItemRequest(*dd) );
  delete dd;
  cd = qualifiedIdentifierRepository.itemFromIndex( m_index );
}

void QualifiedIdentifier::prepareWrite() {
  
  if(m_index) {
    const QualifiedIdentifierPrivate<false>* oldCc = cd;
    dd = new QualifiedIdentifierPrivate<true>;
    dd->m_explicitlyGlobal = oldCc->m_explicitlyGlobal;
    dd->m_isExpression = oldCc->m_isExpression;
    dd->m_isConstant = oldCc->m_isConstant;
    dd->m_isReference = oldCc->m_isReference;
    dd->m_pointerDepth = oldCc->m_pointerDepth;
    dd->m_pointerConstantMask = oldCc->m_pointerConstantMask;
    dd->m_hash = oldCc->m_hash;
    
    dd->copyListsFrom(*oldCc);
    m_index = 0;
  }
  
  dd->clearHash();
}

bool TypeIdentifier::isSame(const TypeIdentifier& rhs, bool ignoreExplicitlyGlobal) const {
  if(m_index)
    return QualifiedIdentifier::isSame(rhs, ignoreExplicitlyGlobal) && cd->m_isConstant == rhs.cd->m_isConstant && cd->m_isReference == rhs.cd->m_isReference && cd->m_pointerConstantMask == rhs.cd->m_pointerConstantMask;
  else
    return QualifiedIdentifier::isSame(rhs, ignoreExplicitlyGlobal) && dd->m_isConstant == rhs.dd->m_isConstant && dd->m_isReference == rhs.dd->m_isReference && dd->m_pointerConstantMask == rhs.dd->m_pointerConstantMask;
}

bool TypeIdentifier::operator==(const TypeIdentifier& rhs) const {
  if(m_index)
    return QualifiedIdentifier::operator==(rhs) && cd->m_isConstant == rhs.cd->m_isConstant && cd->m_isReference == rhs.cd->m_isReference && cd->m_pointerConstantMask == rhs.cd->m_pointerConstantMask && cd->m_pointerDepth == rhs.cd->m_pointerDepth;
  else
    return QualifiedIdentifier::operator==(rhs) && dd->m_isConstant == rhs.dd->m_isConstant && dd->m_isReference == rhs.dd->m_isReference && dd->m_pointerConstantMask == rhs.dd->m_pointerConstantMask && dd->m_pointerDepth == rhs.dd->m_pointerDepth;
}

bool TypeIdentifier::operator!=(const TypeIdentifier& rhs) const {
  return !operator==(rhs);
}

bool TypeIdentifier::isReference() const {
  if(m_index)
    return cd->m_isReference;
  else
    return dd->m_isReference;
}

void TypeIdentifier::setIsReference(bool isRef) {
  prepareWrite();
  dd->m_isReference = isRef;
}

bool TypeIdentifier::isConstant() const {
  if(m_index)
    return cd->m_isConstant;
  else
    return dd->m_isConstant;
}

void TypeIdentifier::setIsConstant(bool isConst) {
  prepareWrite();
  dd->m_isConstant = isConst;
}

///Returns the pointer depth. Example for C++: "char*" has pointer-depth 1, "char***" has pointer-depth 3
int TypeIdentifier::pointerDepth() const {
  if(m_index)
    return cd->m_pointerDepth;
  else
    return dd->m_pointerDepth;
}

/**Sets the pointer-depth to the specified count
  * For efficiency-reasons the maximum currently is 32. */
void TypeIdentifier::setPointerDepth(int depth) {
  prepareWrite();
  ///Clear the mask in removed fields
  for(int s = depth; s < (int)dd->m_pointerDepth; ++s)
    setIsConstPointer(s, false);
    
  dd->m_pointerDepth = depth;
}

bool TypeIdentifier::isConstPointer(int depthNumber) const {
  if(m_index)
    return cd->m_pointerConstantMask & (1 << depthNumber);
  else
    return dd->m_pointerConstantMask & (1 << depthNumber);
}

void TypeIdentifier::setIsConstPointer(int depthNumber, bool constant) {
  prepareWrite();
  if(constant)
    dd->m_pointerConstantMask |= (1 << depthNumber);
  else
    dd->m_pointerConstantMask &= (~(1 << depthNumber));
}

QString TypeIdentifier::toString(bool ignoreExplicitlyGlobal) const {
  QString ret;
  if(isConstant())
    ret += "const ";
  ret += QualifiedIdentifier::toString(ignoreExplicitlyGlobal);
  for(int a = 0; a < pointerDepth(); ++a) {
    ret += '*';
    if( isConstPointer(a) )
      ret += "const";
  }
  
  if(isReference())
    ret += '&';
  return ret;
}

TypeIdentifier::TypeIdentifier(uint index) : QualifiedIdentifier(index) {
}

TypeIdentifier::TypeIdentifier() : QualifiedIdentifier() {
}

TypeIdentifier::TypeIdentifier(const QString& str) : QualifiedIdentifier(str) {
}

TypeIdentifier::TypeIdentifier(const TypeIdentifier& id) : QualifiedIdentifier() {
  id.makeConstant();
  m_index = id.m_index;
  cd = id.cd;
}

TypeIdentifier::TypeIdentifier(const QualifiedIdentifier& id) : QualifiedIdentifier(id) {
}

IndexedIdentifier::IndexedIdentifier() : index(emptyConstantIdentifierPrivateIndex) {
}

IndexedIdentifier::IndexedIdentifier(const Identifier& id) : index(id.index()) {
}

IndexedIdentifier& IndexedIdentifier::operator=(const Identifier& id) {
  index = id.index();
  return *this;
}
bool IndexedIdentifier::operator==(const IndexedIdentifier& rhs) const {
  return index == rhs.index;
}
bool IndexedIdentifier::operator==(const Identifier& id) const {
  return index == id.index();
}

Identifier IndexedIdentifier::identifier() const {
  return Identifier(index);
}

IndexedIdentifier::operator Identifier() const {
  return Identifier(index);
}

IndexedQualifiedIdentifier::IndexedQualifiedIdentifier() : index(emptyConstantQualifiedIdentifierPrivateIndex) {
}

bool IndexedQualifiedIdentifier::isValid() const {
  return index != emptyConstantQualifiedIdentifierPrivateIndex;
}

IndexedQualifiedIdentifier::IndexedQualifiedIdentifier(const QualifiedIdentifier& id) : index(id.index()) {
}

IndexedQualifiedIdentifier& IndexedQualifiedIdentifier::operator=(const QualifiedIdentifier& id) {
  index = id.index();
  return *this;
}
bool IndexedQualifiedIdentifier::operator==(const IndexedQualifiedIdentifier& rhs) const {
  return index == rhs.index;
}
bool IndexedQualifiedIdentifier::operator==(const QualifiedIdentifier& id) const {
  return index == id.index();
}

QualifiedIdentifier IndexedQualifiedIdentifier::identifier() const {
  return QualifiedIdentifier(index);
}

IndexedQualifiedIdentifier::operator QualifiedIdentifier() const {
  return QualifiedIdentifier(index);
}

}

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on

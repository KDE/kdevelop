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
#include "hashedstring.h"

namespace KDevelop
{

class IdentifierPrivate : public KShared
{
public:
  IdentifierPrivate() : m_unique(0), m_hash(0){
  }

  //Flags the stored hash-value invalid
  void clearHash() { //This is always called on an object private to an Identifier, so there is no threading-problem.
    m_hash = 0;
  }

  uint hash() const { //Since this only needs reading and the data needs not to be private, this may be called by multiple threads simultaneously, so computeHash() must be thread-safe.
    if( !m_hash )
      computeHash();
    return m_hash;
  }

  int m_unique;
  HashedString m_identifier;
  QList<TypeIdentifier> m_templateIdentifiers;

private:
    void computeHash() const {
      //this must stay thread-safe(may be called by multiple threads at a time)
      //The thread-safety is given because all threads will have the same result, and it will only be written once at the end.
      uint hash = m_identifier.hash();
      for( QList<TypeIdentifier>::const_iterator it = m_templateIdentifiers.begin(); it != m_templateIdentifiers.end(); ++it )
        hash = hash * 13 + (*it).hash();
      m_hash += m_unique;
      m_hash = hash;
    }
  
    mutable uint m_hash;
};

///Before something is modified in QualifiedIdentifierPrivate, it must be made sure that it is private to the QualifiedIdentifier it is used in(@see QualifiedIdentifier::prepareWrite)
class QualifiedIdentifierPrivate : public KShared
{
public:
  QualifiedIdentifierPrivate() : m_explicitlyGlobal(false), m_isExpression(false), m_isConstant(false), m_isReference(false), m_pointerDepth(0), m_pointerConstantMask(0), m_hash(0) {
  }
  QList<Identifier> m_identifiers;
  bool m_explicitlyGlobal:1;
  bool m_isExpression:1;
  bool m_isConstant:1; //Data for TypeIdentifier, stored here.
  bool m_isReference:1; //Data for TypeIdentifier, stored here.
  uchar m_pointerDepth;
  uint m_pointerConstantMask; //Stores in a mask for each pointer-depth whether it is constant. Supports only max. 32 levels.
  mutable uint m_hash;

  //Constructs m_identifiers
  void splitIdentifiers( const QString& str, int start )
  {
    uint currentStart = start;

    while( currentStart < (uint)str.length() ) {
      m_identifiers << Identifier( str, currentStart, &currentStart );
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
      foreach( const Identifier& identifier, m_identifiers )
        mhash = 11*mhash + identifier.hash();

      mhash += 17 * m_isConstant + 19 * m_isReference + 23 * m_pointerDepth + 31 * m_pointerConstantMask;

      m_hash = mhash;
    }
    return m_hash;
  }
  
};

Identifier::Identifier(const Identifier& rhs) : d(rhs.d) {
}

Identifier::~Identifier() {
}

bool Identifier::nameEquals(const Identifier& rhs) const {
  return d->m_identifier == rhs.d->m_identifier;
}

uint Identifier::hash() const {
  return d->hash();
}

bool Identifier::isEmpty() const {
  return d->m_identifier.str().isEmpty() && d->m_templateIdentifiers.isEmpty();
}

Identifier::Identifier(const QString& id, uint start, uint* takenRange)
  : d(new IdentifierPrivate)
{
  ///Extract template-parameters
  ParamIterator paramIt("<>:", id, start);
  d->m_identifier = paramIt.prefix().trimmed();
  while( paramIt ) {
    appendTemplateIdentifier( TypeIdentifier(*paramIt) );
    ++paramIt;
  }

  if( takenRange )
    *takenRange = paramIt.position();
}

Identifier::Identifier()
  : d(new IdentifierPrivate)
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
  return d->m_unique;
}

int Identifier::uniqueToken() const
{
  return d->m_unique;
}

void Identifier::setUnique(int token)
{
  prepareWrite();
  d->m_unique = token;
}

const QString Identifier::identifier() const
{
  return d->m_identifier.str();
}

void Identifier::setIdentifier(const QString& identifier)
{
  prepareWrite();
  d->m_identifier = identifier;
}

const QList<TypeIdentifier>& Identifier::templateIdentifiers() const
{
  return d->m_templateIdentifiers;
}

void Identifier::appendTemplateIdentifier(const TypeIdentifier& identifier)
{
  prepareWrite();
  d->m_templateIdentifiers.append(identifier);
}

void Identifier::clearTemplateIdentifiers()
{
  prepareWrite();
  d->m_templateIdentifiers.clear();
}

void Identifier::setTemplateIdentifiers(const QList<TypeIdentifier>& templateIdentifiers)
{
  prepareWrite();
  d->m_templateIdentifiers = templateIdentifiers;
}

QString Identifier::toString() const
{
  if (!this) {
    return "(null identifier)";
  }

  QString ret = identifier();

  if (templateIdentifiers().count()) {
    ret.append("< ");
    for (int i = 0; i < templateIdentifiers().count(); ++i) {
      ret.append(templateIdentifiers()[i].toString());
      if (i != templateIdentifiers().count() - 1)
        ret.append(", ");
    }
    ret.append(" >");
  }

  return ret;
}

bool Identifier::operator==(const Identifier& rhs) const
{
  if(d == rhs.d)
    return true;
  
  if (isUnique() || rhs.isUnique())
    if (uniqueToken() == rhs.uniqueToken())
      return true;
    else
      return false;

  if (identifier() != rhs.identifier())
    return false;

  if (templateIdentifiers() != rhs.templateIdentifiers())
    return false;

  return true;
}

bool Identifier::operator!=(const Identifier& rhs) const
{
  return !operator==(rhs);
}

Identifier& Identifier::operator=(const Identifier& rhs)
{
  d = rhs.d;
  return *this;
}

void Identifier::prepareWrite() {
  if( ((int)d->ref) != 1 ) {
    //If there is more than one counted references to the d-pointer, it is shared with other QualifiedIdentifiers, so copy it
    d = KSharedPtr<IdentifierPrivate>(new IdentifierPrivate(*d));
  }
  d->clearHash();
}


static const int idguess = 4;

QualifiedIdentifier::QualifiedIdentifier(const QString& id)
  : d(new QualifiedIdentifierPrivate)
{
  if (id.startsWith("::")) {
    d->m_explicitlyGlobal = true;
    d->splitIdentifiers(id, 2);
  } else {
    d->m_explicitlyGlobal = false;
    d->splitIdentifiers(id, 0);
  }
}

QualifiedIdentifier::QualifiedIdentifier(const Identifier& id)
  : d(new QualifiedIdentifierPrivate)
{
  if (id.d->m_identifier.str().isEmpty()) {
    d->m_explicitlyGlobal = true;
  } else {
    d->m_explicitlyGlobal = false;
    d->m_identifiers << id;
  }
}

QualifiedIdentifier::QualifiedIdentifier()
  : d(new QualifiedIdentifierPrivate)
{
}

QualifiedIdentifier::~QualifiedIdentifier() {
}

QualifiedIdentifier::QualifiedIdentifier(const QualifiedIdentifier& id)
  : d(id.d)
{
  if( d->m_pointerConstantMask || d->m_isReference || d->m_pointerDepth || d->m_isConstant ) {
    //When copying from a type-identifier, do not share the d-pointer, because it contains invalid information.
    prepareWrite();
    d->m_pointerConstantMask = 0;
    d->m_isReference = 0;
    d->m_pointerDepth = 0;
    d->m_isConstant = 0;
  }
}
QStringList QualifiedIdentifier::toStringList() const
{
  QStringList ret;
  if (d->m_explicitlyGlobal)
    ret.append(QString());

  foreach(const Identifier& id, d->m_identifiers)
    ret << id.toString();

  return ret;
}

QString QualifiedIdentifier::toString(bool ignoreExplicitlyGlobal) const
{
  QString ret;
  if( !ignoreExplicitlyGlobal && explicitlyGlobal() )
    ret = "::";

  bool first = true;
  foreach(const Identifier& id, d->m_identifiers)
  {
    if( !first )
      ret += "::";
    else
      first = false;
    
    ret += id.toString();
  }
  
  return ret;
}

QualifiedIdentifier QualifiedIdentifier::merge(const QualifiedIdentifier& base) const
{
  QualifiedIdentifier ret(base);
  ret.prepareWrite();
  
  ret.d->m_identifiers += d->m_identifiers;
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

QualifiedIdentifier QualifiedIdentifier::mergeWhereDifferent(const QualifiedIdentifier& base) const
{
  if (explicitlyGlobal())
    return *this;

  QualifiedIdentifier id;

  int i = 0, j = 0;
  id.setExplicitlyGlobal(base.explicitlyGlobal());

  for (; j < base.count(); ++j) {
    id.push(base.at(j));
    if (i >= count() || at(i) == base.at(j)) {
      i++;
    } else {
      break;
    }
  }

  for (; i < count(); ++i)
    id.push(at(i));

  return id;
}

QualifiedIdentifier QualifiedIdentifier::strip(const QualifiedIdentifier & unwantedBase) const
{
  // Don't strip the top identifier
  if (count() <= unwantedBase.count())
    return *this;

  //Make sure this one starts with unwantedBase
  for( int a = 0; a < unwantedBase.count(); a++ )
    if( d->m_identifiers[a] != unwantedBase.d->m_identifiers[a] )
      return *this;
  
  
  QualifiedIdentifier ret(*this);
  ret.setExplicitlyGlobal(false);
  ret.prepareWrite();

  for( int a = 0; a < unwantedBase.d->m_identifiers.count(); a++ )
    ret.d->m_identifiers.pop_front();

  return ret;
}

bool QualifiedIdentifier::isExpression() const
{
  return d->m_isExpression;
}

void QualifiedIdentifier::setIsExpression(bool is)
{
  prepareWrite();
  d->m_isExpression = is;
}

bool QualifiedIdentifier::explicitlyGlobal() const
{
  // True if started with "::"
  return d->m_explicitlyGlobal;
}

void QualifiedIdentifier::setExplicitlyGlobal(bool eg)
{
  prepareWrite();
  d->m_explicitlyGlobal = eg;
}

bool QualifiedIdentifier::isSame(const QualifiedIdentifier& rhs, bool ignoreExplicitlyGlobal) const
{
  if( d == rhs.d )
    return true;

  if (!ignoreExplicitlyGlobal && (explicitlyGlobal() != rhs.explicitlyGlobal()))
    return false;

  if( isExpression() != rhs.isExpression() )
    return false;
  
  return hash() == rhs.hash();
}

bool QualifiedIdentifier::operator==(const QualifiedIdentifier& rhs) const
{
  if( d == rhs.d )
    return true;
  return hash() == rhs.hash();
}

bool QualifiedIdentifier::operator!=(const QualifiedIdentifier& rhs) const
{
  return !operator==(rhs);
}

QualifiedIdentifier& QualifiedIdentifier::operator=(const QualifiedIdentifier& rhs)
{
  d = rhs.d;
  return *this;
}

QualifiedIdentifier::MatchTypes QualifiedIdentifier::match(const Identifier& rhs) const
{
  if( d->m_identifiers.isEmpty() )
    return NoMatch;

  if( d->m_identifiers.last() == rhs ) {
    if( d->m_identifiers.count() == 1 )
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

bool QualifiedIdentifier::beginsWith(const QualifiedIdentifier& other) const
{
  int i = 0, j = 0;

  for (; i < count() && j < other.count(); ++i, ++j)
    if (at(i) == other.at(j)) {
      continue;
    } else {
      return false;
    }

  return true;
}

uint QualifiedIdentifier::hash() const {
  return d->hash();
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

QString Identifier::mangled() const
{ ///@todo work this over
  static QRegExp simpleIdentifier("[a-zA-Z0-9_]*");
  if (simpleIdentifier.exactMatch(d->m_identifier.str()))
    return QString("%1%2").arg(d->m_identifier.str().length()).arg(d->m_identifier.str());

  // Get the encoded utf form
  QString utf = QString::fromLatin1(d->m_identifier.str().toUtf8());

  return QString("U%1%2").arg(utf.length()).arg(utf);
}

QString QualifiedIdentifier::mangled() const
{
  if (isEmpty())
    return QString();

  if (count() == 1)
    return first().mangled();

  QString ret('Q');

  if (count() > 9)
    ret += QString(",%1,").arg(count());
  else
    ret += count();

  for (int i = 0; i < count(); ++i)
    ret += at(i).mangled();

  return ret;
}

void QualifiedIdentifier::push(const Identifier& id)
{
  prepareWrite();
  
  d->m_identifiers << id;
}

void QualifiedIdentifier::push(const QualifiedIdentifier& id)
{
  prepareWrite();

  d->m_identifiers += id.d->m_identifiers;
}

void QualifiedIdentifier::pop()
{
  prepareWrite();
  d->m_identifiers.pop_back();
}

void QualifiedIdentifier::clear()
{
  prepareWrite();
  d->m_identifiers.clear();
  d->m_explicitlyGlobal = false;
  d->m_isExpression = false;
}


bool QualifiedIdentifier::isEmpty() const
{
  return d->m_identifiers.isEmpty();
}

int QualifiedIdentifier::count() const
{
  return d->m_identifiers.count();
}

Identifier QualifiedIdentifier::first() const
{
  if( d->m_identifiers.isEmpty() )
    return Identifier();
  else
    return at(0);
}

Identifier QualifiedIdentifier::last() const
{
  if( d->m_identifiers.isEmpty() )
    return Identifier();
  else
    return at(count() - 1);
}

Identifier QualifiedIdentifier::top() const
{
  if( d->m_identifiers.isEmpty() )
    return Identifier();
  else
    return at(count() - 1);
}

QualifiedIdentifier QualifiedIdentifier::mid(int pos, int len) const {
  QualifiedIdentifier ret;
  if( pos == 0 )
    ret.setExplicitlyGlobal(explicitlyGlobal());
  
  if( len == -1 )
    len = d->m_identifiers.count() - pos;

  if( pos+len > d->m_identifiers.count() )
    len -= d->m_identifiers.count() - (pos+len);

  for( int a = pos; a < pos+len; a++ )
    ret.push(at(a));
  
  return ret;
}

Identifier QualifiedIdentifier::at(int i) const
{
  Q_ASSERT(i >= 0 && i < d->m_identifiers.size());
  return d->m_identifiers[i];
}

void QualifiedIdentifier::prepareWrite() {
  if( ((int)d->ref) != 1 ) {
    //If there is more than one counted references to the d-pointer, it is shared with other QualifiedIdentifiers, so copy it
    d = KSharedPtr<QualifiedIdentifierPrivate>(new QualifiedIdentifierPrivate(*d));
  }
  d->clearHash();
}

bool TypeIdentifier::isSame(const TypeIdentifier& rhs, bool ignoreExplicitlyGlobal) const {
  return QualifiedIdentifier::isSame(rhs, ignoreExplicitlyGlobal) && d->m_isConstant == rhs.d->m_isConstant && d->m_isReference == rhs.d->m_isReference && d->m_pointerConstantMask == rhs.d->m_pointerConstantMask;
}

bool TypeIdentifier::operator==(const TypeIdentifier& rhs) const {
  return QualifiedIdentifier::operator==(rhs) && d->m_isConstant == rhs.d->m_isConstant && d->m_isReference == rhs.d->m_isReference && d->m_pointerConstantMask == rhs.d->m_pointerConstantMask && d->m_pointerDepth == rhs.d->m_pointerDepth;
}

bool TypeIdentifier::operator!=(const TypeIdentifier& rhs) const {
  return !operator==(rhs);
}

bool TypeIdentifier::isReference() const {
  return d->m_isReference;
}

void TypeIdentifier::setIsReference(bool isRef) {
  prepareWrite();
  d->m_isReference = isRef;
}

bool TypeIdentifier::isConstant() const {
  return d->m_isConstant;
}

void TypeIdentifier::setIsConstant(bool isConst) {
  prepareWrite();
  d->m_isConstant = isConst;
}

///Returns the pointer depth. Example for C++: "char*" has pointer-depth 1, "char***" has pointer-depth 3
int TypeIdentifier::pointerDepth() const {
  return d->m_pointerDepth;
}

/**Sets the pointer-depth to the specified count
  * For efficiency-reasons the maximum currently is 32. */
void TypeIdentifier::setPointerDepth(int depth) {
  prepareWrite();
  ///Clear the mask in removed fields
  for(int s = depth; s < (int)d->m_pointerDepth; ++s)
    setIsConstPointer(s, false);
    
  d->m_pointerDepth = depth;
}

bool TypeIdentifier::isConstPointer(int depthNumber) const {
  return d->m_pointerConstantMask & (1 << depthNumber);
}

void TypeIdentifier::setIsConstPointer(int depthNumber, bool constant) {
  prepareWrite();
  if(constant)
    d->m_pointerConstantMask |= (1 << depthNumber);
  else
    d->m_pointerConstantMask &= (~(1 << depthNumber));
}

QString TypeIdentifier::toString(bool ignoreExplicitlyGlobal) const {
  QString ret;
  if(d->m_isConstant)
    ret += "const ";
  ret += QualifiedIdentifier::toString(ignoreExplicitlyGlobal);
  for(int a = 0; a < d->m_pointerDepth; ++a) {
    ret += '*';
    if( isConstPointer(a) )
      ret += "const";
  }
  if(d->m_isReference)
    ret += '&';
  return ret;
}

TypeIdentifier::TypeIdentifier() : QualifiedIdentifier() {
}

TypeIdentifier::TypeIdentifier(const QString& str) : QualifiedIdentifier(str) {
}

TypeIdentifier::TypeIdentifier(const TypeIdentifier& id) : QualifiedIdentifier() {
  d = id.d;
}

TypeIdentifier::TypeIdentifier(const QualifiedIdentifier& id) : QualifiedIdentifier(id) {
}

}

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on

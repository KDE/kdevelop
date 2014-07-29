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

#ifndef KDEVPLATFORM_IDENTIFIER_H
#define KDEVPLATFORM_IDENTIFIER_H

#include <QtCore/QList>
#include <QtCore/QStringList>
#include <QtCore/QMetaType>
#include <util/kdevvarlengtharray.h>
#include <config-kdevplatform.h>

#include "../languageexport.h"
#include "referencecounting.h"

//We use shared d-pointers, which is even better than a d-pointer, but krazy probably won't get it, so exclude the test.
//krazy:excludeall=dpointer

namespace KDevelop
{

class IndexedTypeIdentifier;
class Identifier;
class QualifiedIdentifier;
template<bool>
class QualifiedIdentifierPrivate;
template<bool>
class IdentifierPrivate;
class IndexedString;

/**
 * A helper-class to store an identifier by index in a type-safe way.
 *
 * The difference to Identifier is that this class only stores the index of an identifier that is in the repository, without any dynamic
 * abilities or access to the contained data.
 *
 * This class does "disk reference counting"
 *
 * @warning Do not use this after QCoreApplication::aboutToQuit() has been emitted, items that are not disk-referenced will be invalid at that point.
 */
class KDEVPLATFORMLANGUAGE_EXPORT IndexedIdentifier : public ReferenceCountManager
{
public:
  IndexedIdentifier();
  IndexedIdentifier(const Identifier& id);
  IndexedIdentifier(const IndexedIdentifier& rhs);
  IndexedIdentifier(IndexedIdentifier&& rhs) Q_DECL_NOEXCEPT;
  IndexedIdentifier& operator=(const Identifier& id);
  IndexedIdentifier& operator=(const IndexedIdentifier& rhs);
  IndexedIdentifier& operator=(IndexedIdentifier&& rhs) Q_DECL_NOEXCEPT;
  ~IndexedIdentifier();
  bool operator==(const IndexedIdentifier& rhs) const;
  bool operator!=(const IndexedIdentifier& rhs) const;
  bool operator==(const Identifier& id) const;

  bool isEmpty() const;

  Identifier identifier() const;
  operator Identifier() const;

  uint getIndex() const
  {
    return index;
  }

private:
  unsigned int index;
};

/**
 * A helper-class to store an identifier by index in a type-safe way.
 *
 * The difference to QualifiedIdentifier is that this class only stores the index of an identifier that is in the repository, without any dynamic
 * abilities or access to the contained data.
 *
 * This class does "disk reference counting"
 *
 * @warning Do not use this after QCoreApplication::aboutToQuit() has been emitted, items that are not disk-referenced will be invalid at that point.
 */
class KDEVPLATFORMLANGUAGE_EXPORT IndexedQualifiedIdentifier : public ReferenceCountManager
{
public:
  IndexedQualifiedIdentifier();
  IndexedQualifiedIdentifier(const QualifiedIdentifier& id);
  IndexedQualifiedIdentifier(const IndexedQualifiedIdentifier& rhs);
  IndexedQualifiedIdentifier(IndexedQualifiedIdentifier&& rhs) Q_DECL_NOEXCEPT;
  IndexedQualifiedIdentifier& operator=(const QualifiedIdentifier& id);
  IndexedQualifiedIdentifier& operator=(const IndexedQualifiedIdentifier& id);
  IndexedQualifiedIdentifier& operator=(IndexedQualifiedIdentifier&& rhs) Q_DECL_NOEXCEPT;
  ~IndexedQualifiedIdentifier();
  bool operator==(const IndexedQualifiedIdentifier& rhs) const;
  bool operator==(const QualifiedIdentifier& id) const;

  bool operator<(const IndexedQualifiedIdentifier& rhs) const
  {
    return index < rhs.index;
  }

  bool isValid() const;
  bool isEmpty() const;

  QualifiedIdentifier identifier() const;
  operator QualifiedIdentifier() const;

  uint getIndex() const
  {
    return index;
  }

private:
  uint index;
};

/**
 * Represents a single unqualified identifier
 */
class KDEVPLATFORMLANGUAGE_EXPORT Identifier
{
  friend class QualifiedIdentifier;

public:
  /**
   * @param start The position in the given string where to start searching for the identifier. (optional)
   * @param takenRange If this is nonzero, it will be filled with the length of the range from the beginning
   *                   of the given string, that was used to construct this identifier. (optional)
   *
   * @warning The identifier is parsed in a C++-similar way, and the result may not be what you expect.
   *          If you want to prevent that parsing, use the constructor that takes IndexedString.
   */
  explicit Identifier(const QString& str, uint start = 0, uint* takenRange = 0);
  /**
   * Preferred constructor, use this if you already have an IndexedString available. This does not decompose the given string.
   */
  explicit Identifier(const IndexedString& str);
  Identifier(const Identifier& rhs);
  Identifier(uint index);
  Identifier();
  Identifier(Identifier&& rhs) Q_DECL_NOEXCEPT;
  ~Identifier();
  Identifier& operator=(const Identifier& rhs);
  Identifier& operator=(Identifier&& rhs) Q_DECL_NOEXCEPT;

  static Identifier unique(int token);

  bool isUnique() const;
  int uniqueToken() const;
  /**
   * If \a token is non-zero, turns this Identifier into the special per-document unique identifier.
   *
   * This is used e.g. for anonymous namespaces.
   *
   * Pass a token which is specific to the document to allow correct equality comparison.
   */
  void setUnique(int token);

  const IndexedString identifier() const;
  void setIdentifier(const QString& identifier);
  /**
   * Should be preferred over the other version
   */
  void setIdentifier(const IndexedString& identifier);

  uint hash() const;

  /**
   * Comparison ignoring the template-identifiers
   */
  bool nameEquals(const Identifier& rhs) const;

  /**
   * @warning This is expensive.
   */
  IndexedTypeIdentifier templateIdentifier(int num) const;
  uint templateIdentifiersCount() const;
  void appendTemplateIdentifier(const IndexedTypeIdentifier& identifier);
  void clearTemplateIdentifiers();
  void setTemplateIdentifiers(const QList<IndexedTypeIdentifier>& templateIdentifiers);

  QString toString() const;

  bool operator==(const Identifier& rhs) const;
  bool operator!=(const Identifier& rhs) const;

  bool isEmpty() const;

  /**
   * @return a unique index within the global identifier repository for this identifier.
   *
   * If the identifier isn't in the repository yet, it is added to the repository.
   */
  uint index() const;

  bool inRepository() const;
private:
  void makeConstant() const;
  void prepareWrite();

  //Only one of the following pointers is valid at a given time
  mutable uint m_index; //Valid if cd is valid
  union {
    mutable IdentifierPrivate<true>* dd; //Dynamic, owned by this identifier
    mutable const IdentifierPrivate<false>* cd; //Constant, owned by the repository
  };
};

/**
 * Represents a qualified identifier
 *
 * QualifiedIdentifier has it's hash-values stored, so using the hash-values is very efficient.
 */
class KDEVPLATFORMLANGUAGE_EXPORT QualifiedIdentifier
{
public:
  explicit QualifiedIdentifier(const QString& id, bool isExpression = false);
  explicit QualifiedIdentifier(const Identifier& id);
  QualifiedIdentifier(const QualifiedIdentifier& id);
  QualifiedIdentifier(uint index);
  QualifiedIdentifier();
  QualifiedIdentifier(QualifiedIdentifier&& rhs) Q_DECL_NOEXCEPT;
  ~QualifiedIdentifier();
  QualifiedIdentifier& operator=(const QualifiedIdentifier& rhs);
  QualifiedIdentifier& operator=(QualifiedIdentifier&& rhs) Q_DECL_NOEXCEPT;

  /**
   * Append @p id to this qualified identifier.
   */
  void push(const IndexedIdentifier& id);
  /**
   * Append @p id to this qualified identifier.
   *
   * NOTE: If you have an indexed identifier available, use the above method instead.
   */
  void push(const Identifier& id);
  /**
   * Append all identifiers of @p id to this qualified identifier.
   */
  void push(const QualifiedIdentifier& id);

  /**
   * Pops one identifier from back:
   */
  void pop();
  void clear();
  bool isEmpty() const;
  int count() const;
  Identifier first() const;
  Identifier last() const;
  Identifier top() const;
  Identifier at(int i) const;
  /**
   * @param pos Position where to start the copy.
   * @param len If this is -1, the whole following part will be returned.
   */
  QualifiedIdentifier mid(int pos, int len = -1) const;

  /**
   * Copy the leftmost \a len number of identifiers.
   *
   * @param len The number of identifiers to copy, or if negative, the number of identifiers to omit from the right
   */
  inline QualifiedIdentifier left(int len) const
  {
    return mid(0, len > 0 ? len : count() + len);
  }

  ///@todo Remove this flag
  bool explicitlyGlobal() const;
  void setExplicitlyGlobal(bool eg);
  bool isQualified() const;

  /**
   * A flag that can be set by setIsExpression
   */
  bool isExpression() const;
  /**
   * Set the expression-flag, that can be retrieved by isExpression().
   * This flag is not respected while creating the hash-value and while operator==() comparison.
   * It is respected while isSame(..) comparison.
   */
  void setIsExpression(bool);

  QString toString(bool ignoreExplicitlyGlobal = false) const;
  QStringList toStringList() const;

  QualifiedIdentifier operator+(const QualifiedIdentifier& rhs) const;
  QualifiedIdentifier& operator+=(const QualifiedIdentifier& rhs);

  /**
   * Nicer interfaces to merge
   */
  QualifiedIdentifier operator+(const Identifier& rhs) const;
  QualifiedIdentifier& operator+=(const Identifier& rhs);

  QualifiedIdentifier operator+(const IndexedIdentifier& rhs) const;
  QualifiedIdentifier& operator+=(const IndexedIdentifier& rhs);

  /**
   * @return a QualifiedIdentifier with this one appended to the other.
   *
   * It is explicitly global if either this or base is.
   */
  QualifiedIdentifier merge(const QualifiedIdentifier& base) const;

  /**
   * The comparison-operators do not respect explicitlyGlobal and isExpression, they only respect the real scope.
   * This is for convenient use in hash-tables etc.
   */
  bool operator==(const QualifiedIdentifier& rhs) const;
  bool operator!=(const QualifiedIdentifier& rhs) const;

  bool beginsWith(const QualifiedIdentifier& other) const;

  uint index() const;

  /**
   * @return true if this qualified identifier is already in the persistent identifier repository
   */
  bool inRepository() const;

  /**
   * The hash does not respect explicitlyGlobal, only the real scope.
   */
  uint hash() const;

  /**
   * Finds all identifiers in the identifier-repository that have the given hash value
   */
  static void findByHash(uint hash, KDevVarLengthArray<QualifiedIdentifier>& target);

protected:
  bool sameIdentifiers(const QualifiedIdentifier& rhs) const;

  void makeConstant() const;
  void prepareWrite();

  mutable uint m_index;
  union {
    mutable QualifiedIdentifierPrivate<true>* dd;
    mutable const QualifiedIdentifierPrivate<false>* cd;
  };
};

/**
 * Extends IndexedQualifiedIdentifier by:
 * - Arbitrary count of pointer-poperators with cv-qualifiers
 * - Reference operator
 * All the properties set here are respected in the hash value.
 */
class KDEVPLATFORMLANGUAGE_EXPORT IndexedTypeIdentifier
{
public:
  /**
   * Variables like pointerDepth, isReference, etc. are not parsed from the string, so this parsing is quite limited.
   */
  explicit IndexedTypeIdentifier(const IndexedQualifiedIdentifier& identifier = IndexedQualifiedIdentifier());
  explicit IndexedTypeIdentifier(const QString& identifer, bool isExpression = false);

  bool isReference() const;
  void setIsReference(bool);

  bool isRValue() const;
  void setIsRValue(bool);

  bool isConstant() const;
  void setIsConstant(bool);

  bool isVolatile() const;
  void setIsVolatile(bool);

  IndexedQualifiedIdentifier identifier() const ;

  void setIdentifier(const IndexedQualifiedIdentifier& id);

  /**
   * @return the pointer depth. Example for C++: "char*" has pointer-depth 1, "char***" has pointer-depth 3
   */
  int pointerDepth() const;
  /**
   * Sets the pointer-depth to the specified count.
   *
   * When the pointer-depth is increased, the "isConstPointer" values for new depths will be initialized with false.
   *
   * For efficiency-reasons the maximum currently is 23.
   */
  void setPointerDepth(int);

  /**
   * Whether the target of pointer 'depthNumber' is constant
   */
  bool isConstPointer(int depthNumber) const;
  void setIsConstPointer(int depthNumber, bool constant);

  QString toString(bool ignoreExplicitlyGlobal = false) const;

  uint hash() const;

  /**
   * The comparison-operators do not respect explicitlyGlobal and isExpression, they only respect the real scope.
   * This is for convenient use in hash-tables etc.
   */
  bool operator==(const IndexedTypeIdentifier& rhs) const;
  bool operator!=(const IndexedTypeIdentifier& rhs) const;

private:
  IndexedQualifiedIdentifier m_identifier;
  // The overall number of bits shared by these bit-fields should not exceed 32,
  // so that we don't waste space. IndexedTypeIdentifer should be as compact as possible.
  bool m_isConstant : 1;
  bool m_isReference : 1;
  bool m_isRValue : 1;
  bool m_isVolatile : 1;
  uint m_pointerDepth : 5;
  uint m_pointerConstMask : 23;
};

KDEVPLATFORMLANGUAGE_EXPORT uint qHash(const IndexedTypeIdentifier& id);
KDEVPLATFORMLANGUAGE_EXPORT uint qHash(const QualifiedIdentifier& id);
KDEVPLATFORMLANGUAGE_EXPORT uint qHash(const Identifier& id);

inline uint qHash(const IndexedIdentifier& id)
{
  return id.getIndex();
}

inline uint qHash(const IndexedQualifiedIdentifier& id)
{
  return id.getIndex();
}

}

Q_DECLARE_TYPEINFO(KDevelop::IndexedQualifiedIdentifier, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(KDevelop::IndexedIdentifier, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(KDevelop::IndexedQualifiedIdentifier);
Q_DECLARE_METATYPE(KDevelop::IndexedIdentifier);

Q_DECLARE_TYPEINFO(KDevelop::QualifiedIdentifier, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(KDevelop::Identifier, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(KDevelop::QualifiedIdentifier)
Q_DECLARE_METATYPE(KDevelop::Identifier)

/**
 * {q,k}Debug() stream operator: Writes the Identifier to the debug output.
 */
KDEVPLATFORMLANGUAGE_EXPORT QDebug operator<<(QDebug s, const KDevelop::Identifier& identifier);

/**
 * {q,k}Debug() stream operator: Writes the QualifiedIdentifier to the debug output.
 */
KDEVPLATFORMLANGUAGE_EXPORT QDebug operator<<(QDebug s, const KDevelop::QualifiedIdentifier& identifier);

#endif // KDEVPLATFORM_IDENTIFIER_H

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

#ifndef IDENTIFIER_H
#define IDENTIFIER_H

#include <QtCore/QList>
#include <QtCore/QStack>
#include <QtCore/QStringList>
#include <QtCore/QVarLengthArray>

#include <ksharedptr.h>
#include <kdebug.h>

#include "language/languageexport.h"

//We use shared d-pointers, which is even better than a d-pointer, but krazy probably won't get it, so exclude the test.
//krazy:excludeall=dpointer

namespace KDevelop
{

class TypeIdentifier;
class Identifier;
class QualifiedIdentifier;
template<bool>
class QualifiedIdentifierPrivate;
template<bool>
class IdentifierPrivate;
class IndexedString;

//A helper-class to store an identifier by index in a type-safe way. Will be extended to do reference-counting at some point.
//The difference to Identifier is that this class only stores the index of an identifier that is in the repository, without any dynamic
//abilities or access to the contained data.
struct IndexedIdentifier {
  IndexedIdentifier();
  IndexedIdentifier(const Identifier& id);
  IndexedIdentifier& operator=(const Identifier& id);
  bool operator==(const IndexedIdentifier& rhs) const;
  bool operator==(const Identifier& id) const;
  
  Identifier identifier() const;
  operator Identifier() const;
  
  uint index;
};

//A helper-class to store an identifier by index in a type-safe way. Will be extended to do reference-counting at some point.
//The difference to QualifiedIdentifier is that this class only stores the index of an identifier that is in the repository, without any dynamic
//abilities or access to the contained data.
struct IndexedQualifiedIdentifier {
  IndexedQualifiedIdentifier();
  IndexedQualifiedIdentifier(const QualifiedIdentifier& id);
  IndexedQualifiedIdentifier& operator=(const QualifiedIdentifier& id);
  bool operator==(const IndexedQualifiedIdentifier& rhs) const;
  bool operator==(const QualifiedIdentifier& id) const;
  
  QualifiedIdentifier identifier() const;
  operator QualifiedIdentifier() const;
  
  uint index;
};

/// Represents a single unqualified identifier
class KDEVPLATFORMLANGUAGE_EXPORT Identifier
{
  friend class QualifiedIdentifier;

public:
  /**
   * @param start The position in the given string where to start searching for the identifier(optional).
   * @param takenRange If this is nonzero, it will be filled with the length of the range from the beginning of the given string, that was used to construct this identifier.(optional)
   * */
  explicit Identifier(const QString& str, uint start = 0, uint* takenRange = 0);
  ///Preferred constructor, ise this if you already have an IndexedString available. This does not decompose the given string.
  explicit Identifier(const IndexedString& str);
  Identifier(const Identifier& rhs);
  Identifier(uint index);
  Identifier();
  ~Identifier();

  static Identifier unique(int token);

  bool isUnique() const;
  int uniqueToken() const;
  /// If \a token is non-zero, turns this Identifier into the special per-document
  /// Unique identifier, used for anonymous namespaces.
  /// Pass a token which is specific to the document to allow correct equality comparison.
  void setUnique(int token);

  const QString identifier() const;
  void setIdentifier(const QString& identifier);
  //Should be preferred over the other version
  void setIdentifier(const IndexedString& identifier);

//   QString mangled() const;

  uint hash() const;

  /**
   * Comparison ignoring the template-identifiers
   * */
  bool nameEquals(const Identifier& rhs) const;

  //Expensive
  TypeIdentifier templateIdentifier(int num) const;
  uint templateIdentifiersCount() const;
  void appendTemplateIdentifier(const TypeIdentifier& identifier);
  void clearTemplateIdentifiers();
  void setTemplateIdentifiers(const QList<TypeIdentifier>& templateIdentifiers);

  QString toString() const;

  bool operator==(const Identifier& rhs) const;
  bool operator!=(const Identifier& rhs) const;
  Identifier& operator=(const Identifier& rhs);

  bool isEmpty() const;

  ///Returns a unique index within the global identifier repository for this identifier.
  ///If the identifier isn't in the repository yet, it is added to the repository.
  uint index() const;
  
  /**
    * kDebug(9505) stream operator.  Writes this identifier to the debug output in a nicely formatted way.
    */
  inline friend kdbgstream& operator<< (kdbgstream& s, const Identifier& identifier) {
    s << identifier.toString();
    return s;
  }

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
 * */
class KDEVPLATFORMLANGUAGE_EXPORT QualifiedIdentifier
{
public:
  explicit QualifiedIdentifier(const QString& id);
  explicit QualifiedIdentifier(const Identifier& id);
  QualifiedIdentifier(const QualifiedIdentifier& id);
  QualifiedIdentifier(uint index);
  QualifiedIdentifier();
  ~QualifiedIdentifier();

  void push(const Identifier& id);
  void push(const QualifiedIdentifier& id);
  //Pops one identifier from back:
  void pop();
  void clear();
  bool isEmpty() const;
  int count() const;
  Identifier first() const;
  Identifier last() const;
  Identifier top() const;
  const Identifier at(int i) const;
  /**
   * @param pos Position where to start the copy.
   * @param len If this is -1, the whole following part will be returned.
   * */
  QualifiedIdentifier mid(int pos, int len = -1) const;

  static QualifiedIdentifier merge(const QStack<QualifiedIdentifier>& idStack);

  bool explicitlyGlobal() const;
  void setExplicitlyGlobal(bool eg);
  bool isQualified() const;

  ///A flag that can be set by setIsExpression
  bool isExpression() const;
  /**
   * Set the expression-flag, that can be retrieved by isExpression().
   * This flag is not respected while creating the hash-value and while operator==() comparison.
   * It is respected while isSame(..) comparison.
   * */
  void setIsExpression(bool);

  QString toString(bool ignoreExplicitlyGlobal = false) const;
  QStringList toStringList() const;

//   QString mangled() const;

  QualifiedIdentifier operator+(const QualifiedIdentifier& rhs) const;
  QualifiedIdentifier& operator+=(const QualifiedIdentifier& rhs);

  //Nicer interfaces to merge
  QualifiedIdentifier operator+(const Identifier& rhs) const;
  QualifiedIdentifier& operator+=(const Identifier& rhs);

  //Returns a QualifiedIdentifier with this one appended to the other. It is explicitly global if either this or base is.
  QualifiedIdentifier merge(const QualifiedIdentifier& base) const;
//   //The returned identifier will have explicitlyGlobal() set to false
//   QualifiedIdentifier strip(const QualifiedIdentifier& unwantedBase) const;

  /**
   * A more complex comparison than operator==(..).
   * It does respect the isExpression() flag, and optionally the explicitlyGlobal flag.
   * */
  bool isSame(const QualifiedIdentifier& rhs, bool ignoreExplicitlyGlobal=true) const;

  /**
   * Computes the hash-value that would be created with a qualified identifier with hash-value @param leftHash of size @param leftSize
   * with @param appendIdentifier appended as an identifier
   * */
  static uint combineHash(uint leftHash, uint leftSize, Identifier appendIdentifier);
  
  /**The comparison-operators do not respect explicitlyGlobal and isExpression, they only respect the real scope.
   * This is for convenient use in hash-tables etc.
   * */
  bool operator==(const QualifiedIdentifier& rhs) const;
  bool operator!=(const QualifiedIdentifier& rhs) const;
  QualifiedIdentifier& operator=(const QualifiedIdentifier& rhs);
#if 0
  enum MatchTypes {
    NoMatch        /**< matches no identifier */,
    EndsWith       /**< The current identifier ends with the one given to the match function */,
    TargetEndsWith /**< The identifier given to the match function ends with the current identifier */,
    ExactMatch     /**< matches if the identifiers match exactly */
  };

  MatchTypes match(const Identifier& other) const;
  MatchTypes match(const QualifiedIdentifier& other) const;
#endif
  bool beginsWith(const QualifiedIdentifier& other) const;

  uint index() const;
  
  /**
    * kDebug(9505) stream operator.  Writes this identifier to the debug output in a nicely formatted way.
    */
  inline friend kdbgstream& operator<< (kdbgstream& s, const QualifiedIdentifier& identifier) {
    s << identifier.toString();
    return s;
  }

  typedef uint HashType;
  
  ///The hash does not respect explicitlyGlobal, only the real scope.
  HashType hash() const;

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
 * Extends QualifiedIdentifier by:
 * - Arbitrary count of pointer-poperators with cv-qualifiers
 * - Reference operator
 * All the properties set here are respected in the hash value.
 * */
class KDEVPLATFORMLANGUAGE_EXPORT TypeIdentifier : public QualifiedIdentifier
{
public:
  ///Variables like pointerDepth, isReference, etc. are not parsed from the string, so this parsing is quite limited.
  TypeIdentifier();
  TypeIdentifier(const QString& str);
  TypeIdentifier(const QualifiedIdentifier& id);
  TypeIdentifier(const TypeIdentifier& id);
  TypeIdentifier(uint index);
  bool isReference() const;
  void setIsReference(bool);
  
  bool isConstant() const;
  void setIsConstant(bool);

  ///Returns the pointer depth. Example for C++: "char*" has pointer-depth 1, "char***" has pointer-depth 3
  int pointerDepth() const;
  /**Sets the pointer-depth to the specified count
   * When the pointer-depth is increased, the "isConstPointer" values for new depths will be initialized with false.
   * For efficiency-reasons the maximum currently is 32. */
  void setPointerDepth(int);

  ///Whether the target of pointer 'depthNumber' is constant
  bool isConstPointer(int depthNumber) const;
  void setIsConstPointer(int depthNumber, bool constant);

  bool isSame(const TypeIdentifier& rhs, bool ignoreExplicitlyGlobal=true) const;

  QString toString(bool ignoreExplicitlyGlobal = false) const;
  
  /**The comparison-operators do not respect explicitlyGlobal and isExpression, they only respect the real scope.
   * This is for convenient use in hash-tables etc.
   * */
  bool operator==(const TypeIdentifier& rhs) const;
  bool operator!=(const TypeIdentifier& rhs) const;
};

KDEVPLATFORMLANGUAGE_EXPORT uint qHash(const TypeIdentifier& id);
KDEVPLATFORMLANGUAGE_EXPORT uint qHash(const QualifiedIdentifier& id);
KDEVPLATFORMLANGUAGE_EXPORT uint qHash(const Identifier& id);

}

#endif // IDENTIFIER_H

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on

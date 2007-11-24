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
#include "../languageexport.h"

//We use shared d-pointers, which is even better than a d-pointer, but krazy probably won't get it, so exclude the test.
//krazy:excludeall=dpointer

namespace KDevelop
{

class QualifiedIdentifier;
class QualifiedIdentifierPrivate;
class IdentifierPrivate;

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
  Identifier(const Identifier& rhs);
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

  QString mangled() const;

  uint hash() const;

  /**
   * Comparison ignoring the template-identifiers
   * */
  bool nameEquals(const Identifier& rhs) const;

  const QList<QualifiedIdentifier>& templateIdentifiers() const;
  void appendTemplateIdentifier(const QualifiedIdentifier& identifier);
  void clearTemplateIdentifiers();
  void setTemplateIdentifiers(const QList<QualifiedIdentifier>& templateIdentifiers);

  QString toString() const;

  bool operator==(const Identifier& rhs) const;
  bool operator!=(const Identifier& rhs) const;
  Identifier& operator=(const Identifier& rhs);

  bool isEmpty() const;

  /**
    * kDebug(9505) stream operator.  Writes this identifier to the debug output in a nicely formatted way.
    */
  inline friend kdbgstream& operator<< (kdbgstream& s, const Identifier& identifier) {
    s << identifier.toString();
    return s;
  }

  /**
    * Non-debug stream operator; does nothing.
    */
  inline friend kndbgstream& operator<< (kndbgstream& s, const Identifier&) { return s; }

private:
  void prepareWrite();
  KSharedPtr<IdentifierPrivate> d;
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
  //QualifiedIdentifier(const QualifiedIdentifier& id);
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
  Identifier at(int i) const;
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

  QString mangled() const;

  QualifiedIdentifier operator+(const QualifiedIdentifier& rhs) const;
  QualifiedIdentifier& operator+=(const QualifiedIdentifier& rhs);

  //Nicer interfaces to merge
  QualifiedIdentifier operator+(const Identifier& rhs) const;
  QualifiedIdentifier& operator+=(const Identifier& rhs);

  //Returns a QualifiedIdentifier with this one appended to the other. It is explicitly global if either this or base is.
  QualifiedIdentifier merge(const QualifiedIdentifier& base) const;
  QualifiedIdentifier mergeWhereDifferent(const QualifiedIdentifier& base) const;
  //The returned identifier will have explicitlyGlobal() set to false
  QualifiedIdentifier strip(const QualifiedIdentifier& unwantedBase) const;

  /**
   * A more complex comparison than operator==(..).
   * It does respect the isExpression() flag, and optionally the explicitlyGlobal flag.
   * */
  bool isSame(const QualifiedIdentifier& rhs, bool ignoreExplicitlyGlobal=true) const;

  /**The comparison-operators do not respect explicitlyGlobal and isExpression, they only respect the real scope.
   * This is for convenient use in hash-tables etc.
   * */
  bool operator==(const QualifiedIdentifier& rhs) const;
  bool operator!=(const QualifiedIdentifier& rhs) const;
  QualifiedIdentifier& operator=(const QualifiedIdentifier& rhs);

  enum MatchTypes {
    NoMatch,
    EndsWith, //The current identifier ends with the one given to the match function
    TargetEndsWith, //The identifier given to the match function ends with the current identifier
    ExactMatch
  };

  MatchTypes match(const Identifier& other) const;
  MatchTypes match(const QualifiedIdentifier& other) const;
  bool beginsWith(const QualifiedIdentifier& other) const;

  /**
    * kDebug(9505) stream operator.  Writes this identifier to the debug output in a nicely formatted way.
    */
  inline friend kdbgstream& operator<< (kdbgstream& s, const QualifiedIdentifier& identifier) {
    s << identifier.toString();
    return s;
  }

  ///The hash does not respect explicitlyGlobal, only the real scope.
  uint hash() const;

  /**
    * Non-debug stream operator; does nothing.
    */
  inline friend kndbgstream& operator<< (kndbgstream& s, const QualifiedIdentifier&) { return s; }

private:
  void prepareWrite();
  KSharedPtr<QualifiedIdentifierPrivate> d;
};

KDEVPLATFORMLANGUAGE_EXPORT uint qHash(const QualifiedIdentifier& id);
KDEVPLATFORMLANGUAGE_EXPORT uint qHash(const Identifier& id);

}

#endif // IDENTIFIER_H

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on

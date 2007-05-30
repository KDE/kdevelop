/* This file is part of KDevelop
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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

#include <kdebug.h>
#include "kdevlanguageexport.h"

namespace KDevelop
{

class QualifiedIdentifier;

/// Represents a single unqualified identifier
class KDEVPLATFORMLANGUAGE_EXPORT Identifier
{
  friend class QualifiedIdentifier;

public:
  explicit Identifier(const QString& id);
  Identifier();

  static Identifier unique(int token);

  bool isUnique() const;
  int uniqueToken() const;
  /// If \a token is non-zero, turns this Identifier into the special per-document
  /// Unique identifier, used for anonymous namespaces.
  /// Pass a token which is specific to the document to allow correct equality comparison.
  void setUnique(int token);

  const QString& identifier() const;
  void setIdentifier(const QString& identifier);

  QString mangled() const;

  const QList<QualifiedIdentifier>& templateIdentifiers() const;
  void appendTemplateIdentifier(const QualifiedIdentifier& identifier);
  void clearTemplateIdentifiers();
  void setTemplateIdentifiers(const QList<QualifiedIdentifier>& templateIdentifiers);

  QString toString() const;

  bool operator==(const Identifier& rhs) const;
  bool operator!=(const Identifier& rhs) const;
  Identifier& operator=(const Identifier& rhs);

  /**
    * kDebug() stream operator.  Writes this identifier to the debug output in a nicely formatted way.
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
  class IdentifierPrivate* const d;
};

/// Represents a qualified identifier
class KDEVPLATFORMLANGUAGE_EXPORT QualifiedIdentifier
{
public:
  explicit QualifiedIdentifier(const QString& id);
  explicit QualifiedIdentifier(const Identifier& id);
  QualifiedIdentifier(const QualifiedIdentifier& id, int reserve = 0);
  QualifiedIdentifier();

  void push(const Identifier& id);
  void push(const QualifiedIdentifier& id);
  void pop();
  void clear();
  bool isEmpty() const;
  int count() const;
  Identifier first() const;
  Identifier last() const;
  Identifier top() const;
  Identifier at(int i) const;

  static QualifiedIdentifier merge(const QStack<QualifiedIdentifier>& idStack);

  bool explicitlyGlobal() const;
  void setExplicitlyGlobal(bool eg);
  bool isQualified() const;

  QString toString(bool ignoreExplicitlyGlobal = false) const;
  QStringList toStringList() const;

  QString mangled() const;

  QualifiedIdentifier merge(const QualifiedIdentifier& base) const;
  QualifiedIdentifier mergeWhereDifferent(const QualifiedIdentifier& base) const;
  QualifiedIdentifier strip(const QualifiedIdentifier& unwantedBase) const;

  bool operator==(const QualifiedIdentifier& rhs) const;
  bool operator!=(const QualifiedIdentifier& rhs) const;
  QualifiedIdentifier& operator=(const QualifiedIdentifier& rhs);

  enum MatchTypes {
    NoMatch,
    Contains,
    ContainedBy,
    ExactMatch
  };

  MatchTypes match(const Identifier& other) const;
  MatchTypes match(const QualifiedIdentifier& other) const;
  bool beginsWith(const QualifiedIdentifier& other) const;

  /**
    * kDebug() stream operator.  Writes this identifier to the debug output in a nicely formatted way.
    */
  inline friend kdbgstream& operator<< (kdbgstream& s, const QualifiedIdentifier& identifier) {
    s << identifier.toString();
    return s;
  }

  /**
    * Non-debug stream operator; does nothing.
    */
  inline friend kndbgstream& operator<< (kndbgstream& s, const QualifiedIdentifier&) { return s; }

private:
  class QualifiedIdentifierPrivate* const d;
};

KDEVPLATFORMLANGUAGE_EXPORT uint qHash(const QualifiedIdentifier& id);

}

#endif // IDENTIFIER_H

// kate: space-indent on; indent-width 2; tab-width: 4; replace-tabs on; auto-insert-doxygen on

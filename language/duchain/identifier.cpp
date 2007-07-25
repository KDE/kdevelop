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

#include "identifier.h"

#include <QHash>
#include "stringhelpers.h"

namespace KDevelop
{

class IdentifierPrivate
{
public:
  int m_unique;
  QString m_identifier;
  QList<QualifiedIdentifier> m_templateIdentifiers;
};

class QualifiedIdentifierPrivate
{
public:
  QString m_qid;
  QVarLengthArray<int, 8> m_idSplits;
  bool m_explicitlyGlobal;
};

Identifier::Identifier(const QString& id)
  : d(new IdentifierPrivate)
{
  d->m_unique = 0;

  ///Extract template-parameters
  ParamIterator paramIt("<>", id);
  d->m_identifier = paramIt.prefix();
  while( paramIt ) {
    appendTemplateIdentifier( QualifiedIdentifier(*paramIt) );
    ++paramIt;
  }
}

Identifier::Identifier()
  : d(new IdentifierPrivate)
{
  d->m_unique = 0;
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
  d->m_unique = token;
}

const QString& Identifier::identifier() const
{
  return d->m_identifier;
}

void Identifier::setIdentifier(const QString& identifier)
{
  d->m_identifier = identifier;
}

const QList<QualifiedIdentifier>& Identifier::templateIdentifiers() const
{
  return d->m_templateIdentifiers;
}

void Identifier::appendTemplateIdentifier(const QualifiedIdentifier& identifier)
{
  d->m_templateIdentifiers.append(identifier);
}

void Identifier::clearTemplateIdentifiers()
{
  d->m_templateIdentifiers.clear();
}

void Identifier::setTemplateIdentifiers(const QList<QualifiedIdentifier>& templateIdentifiers)
{
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
  if( d != rhs.d )
  {
    d->m_identifier = rhs.d->m_identifier;
    d->m_unique = rhs.d->m_unique;
    d->m_templateIdentifiers = rhs.d->m_templateIdentifiers;
  }
  return *this;
}

static const int idguess = 4;

QualifiedIdentifier::QualifiedIdentifier(const QString& id)
  : d(new QualifiedIdentifierPrivate)
{
  if (id.startsWith("::")) {
    d->m_explicitlyGlobal = true;
    d->m_qid = id.mid(2);
  } else {
    d->m_explicitlyGlobal = false;
    d->m_qid = id;
  }

  d->m_idSplits.append(0);
  int split = 0;
  forever {
    split = d->m_qid.indexOf("::", split);
    if (split == -1)
      break;
    d->m_idSplits.append(split + 2);
    split += 2;
  }
}

QualifiedIdentifier::QualifiedIdentifier(const Identifier& id)
  : d(new QualifiedIdentifierPrivate)
{
  if (id.d->m_identifier.isEmpty()) {
    d->m_explicitlyGlobal = true;
  } else {
    d->m_explicitlyGlobal = false;
    d->m_qid = id.toString();
    d->m_idSplits.append(0);
  }
}

QualifiedIdentifier::QualifiedIdentifier()
  : d(new QualifiedIdentifierPrivate)
{
  d-> m_explicitlyGlobal = false;
}

QualifiedIdentifier::QualifiedIdentifier(const QualifiedIdentifier& id, int reserve)
  : d(new QualifiedIdentifierPrivate)
{
  if (reserve)
    d->m_qid.reserve(reserve);

  d->m_qid = id.d->m_qid;
  d->m_idSplits = id.d->m_idSplits;
  d->m_explicitlyGlobal = id.d->m_explicitlyGlobal;
}

QStringList QualifiedIdentifier::toStringList() const
{
  QStringList ret;
  if (d->m_explicitlyGlobal)
    ret.append(QString());

  ret << d->m_qid.split("::");

  return ret;
}

QString QualifiedIdentifier::toString(bool ignoreExplicitlyGlobal) const
{
  if (ignoreExplicitlyGlobal || !explicitlyGlobal())
    return d->m_qid;
  else
    return QString("::") + d->m_qid;
}

QualifiedIdentifier QualifiedIdentifier::merge(const QualifiedIdentifier& base) const
{
  if (explicitlyGlobal())
    return *this;

  QualifiedIdentifier ret(base, base.d->m_qid.length() + 2 + d->m_qid.length());
  if (ret.count())
    ret.d->m_qid.append("::");
  ret.d->m_qid.append(d->m_qid);

  const int offset = ret.d->m_qid.length();
  for (int i = 0; i < d->m_idSplits.count(); ++i)
    ret.d->m_idSplits.append(d->m_idSplits[i] + offset);

  // TODO verify...

  return ret;
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

  if (d->m_qid.startsWith(unwantedBase.d->m_qid)) {
    const int offset = unwantedBase.d->m_qid.length() + 2;

    for (int index = 0; index < d->m_idSplits.count(); ++index) {
      if (d->m_idSplits[index] != offset)
        continue;

      // Match
      QualifiedIdentifier id;
      // Don't convey explicitly global...
      id.d->m_qid = d->m_qid.mid(d->m_idSplits[index]);
      for (; index < d->m_idSplits.count(); ++index)
        id.d->m_idSplits.append(d->m_idSplits[index] - offset);

      return id;
    }
  }

  return *this;
}

bool QualifiedIdentifier::explicitlyGlobal() const
{
  // True if started with "::"
  return d->m_explicitlyGlobal;
}

void QualifiedIdentifier::setExplicitlyGlobal(bool eg)
{
  d->m_explicitlyGlobal = eg;
}

bool QualifiedIdentifier::operator==(const QualifiedIdentifier& rhs) const
{
  // Fast path
  if (d->m_idSplits.count() != rhs.d->m_idSplits.count())
    return false;

  return d->m_qid == rhs.d->m_qid;
}

bool QualifiedIdentifier::operator!=(const QualifiedIdentifier& rhs) const
{
  return !operator==(rhs);
}

QualifiedIdentifier& QualifiedIdentifier::operator=(const QualifiedIdentifier& rhs)
{
  if( d != rhs.d )
  {
    d->m_explicitlyGlobal = rhs.d->m_explicitlyGlobal;
    d->m_qid = rhs.d->m_qid;
    d->m_idSplits = rhs.d->m_idSplits;
  }
  return *this;
}

QualifiedIdentifier::MatchTypes QualifiedIdentifier::match(const Identifier& rhs) const
{
  const int difference = count() - 1;
  if (difference == 0) {
    if (d->m_qid == rhs.d->m_identifier)
      return ExactMatch;
    else
      return NoMatch;

  } else if (difference > 0) {
    if (d->m_qid.endsWith(rhs.d->m_identifier))
      return Contains;
  }

  return NoMatch;
}

QualifiedIdentifier::MatchTypes QualifiedIdentifier::match(const QualifiedIdentifier& rhs) const
{
  const int difference = count() - rhs.count();
  if (difference == 0) {
    if (d->m_qid == rhs.d->m_qid)
      return ExactMatch;
    else
      return NoMatch;

  } else if (difference > 0) {
    // eg 6 vs 4... we need at(2)
    if (rhs.explicitlyGlobal())
      return NoMatch;

    //if (m_idSplits[count() - 1 - rhs.count()] == rhs.m_qid.length())
      if (d->m_qid.endsWith(rhs.d->m_qid))
        return Contains;

  } else {
    if (explicitlyGlobal())
      return NoMatch;

    //if (rhs.m_idSplits[rhs.count() - 1 - count()] == m_qid.length())
      if (rhs.d->m_qid.endsWith(d->m_qid))
        return ContainedBy;
  }

  return NoMatch;

  /*int i = count() - 1;
  int j = rhs.count() - 1;
  for (; i >= 0 && j >= 0; --i, --j)
    if (at(i) != rhs.at(j))
      return NoMatch;

  // put i and j back where they were before the last unneeded decrement
  ++i;
  ++j;

  if (i == l && j == r)
    return ExactMatch;

  if (i > l)
    if (!rhs.explicitlyGlobal())
      return Contains;
    else
      return NoMatch;
  else if (!explicitlyGlobal())
    return ContainedBy;
  else
    return NoMatch;*/
}

bool QualifiedIdentifier::beginsWith(const QualifiedIdentifier& other) const
{
  int i = 0, j = 0;

  /*if (explicitlyGlobal())
    i = 1;

  if (other.explicitlyGlobal())
    j = 1;*/

  bool ret = true;

  for (; i < count() && j < other.count(); ++i, ++j)
    if (at(i) == other.at(j)) {
      continue;
    } else {
      ret = false;
      break;
    }

  return ret;
}

uint qHash(const QualifiedIdentifier& id)
{
  QString identifier = id.toString(true);
  return qHash(identifier);
}

bool QualifiedIdentifier::isQualified() const
{
  return count() > 1 || explicitlyGlobal();
}

QString Identifier::mangled() const
{
  static QRegExp simpleIdentifier("[a-zA-Z0-9_]*");
  if (simpleIdentifier.exactMatch(d->m_identifier))
    return QString("%1%2").arg(d->m_identifier.length()).arg(d->m_identifier);

  // Get the encoded utf form
  QString utf = QString::fromLatin1(d->m_identifier.toUtf8());

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
  if (d->m_qid.isEmpty()) {
    if (id.d->m_identifier.isEmpty()) {
      d->m_explicitlyGlobal = true;
    } else {
      d->m_idSplits.append(0);
      d->m_qid.append(id.toString());
    }
  } else {
    d->m_qid.append("::");
    d->m_idSplits.append(d->m_qid.length());
    d->m_qid.append(id.toString());
  }
}

void QualifiedIdentifier::push(const QualifiedIdentifier& id)
{
  if (d->m_qid.isEmpty()) {
    d->m_qid = id.d->m_qid;
    d->m_idSplits = id.d->m_idSplits;
    d->m_explicitlyGlobal = id.d->m_explicitlyGlobal;

  } else {
    d->m_qid.append("::");
    const int offset = d->m_qid.length();
    d->m_qid.append(id.d->m_qid);
    for (int i = 0; i < id.d->m_idSplits.count(); ++i)
      d->m_idSplits.append(offset + id.d->m_idSplits[i]);
  }
}

void QualifiedIdentifier::pop()
{
  d->m_qid = d->m_qid.left(d->m_idSplits[d->m_idSplits.count() - 1] - 2);
  d->m_idSplits.resize(d->m_idSplits.count() - 1);
}

void QualifiedIdentifier::clear()
{
  d->m_qid.clear();
  d->m_idSplits.clear();
  d->m_explicitlyGlobal = false;
}


bool QualifiedIdentifier::isEmpty() const
{
  return d->m_idSplits.isEmpty();
}

int QualifiedIdentifier::count() const
{
  return d->m_idSplits.count();
}

Identifier QualifiedIdentifier::first() const
{
  return at(0);
}

Identifier QualifiedIdentifier::last() const
{
  return at(count() - 1);
}

Identifier QualifiedIdentifier::top() const
{
  return at(count() - 1);
}

Identifier QualifiedIdentifier::at(int i) const
{
  return Identifier(d->m_qid.mid(d->m_idSplits[i], (i == d->m_idSplits.count() - 1) ? -1 : d->m_idSplits[i + 1] - 2));
}

}

// kate: space-indent on; indent-width 2; tab-width: 4; replace-tabs on; auto-insert-doxygen on

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

Identifier::Identifier(const QString id)
  : m_unique(0)
  , m_identifier(id)
{
}

Identifier::Identifier()
  : m_unique(0)
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
  return m_unique;
}

int Identifier::uniqueToken() const
{
  return m_unique;
}

void Identifier::setUnique(int token)
{
  m_unique = token;
}

const QString& Identifier::identifier() const
{
  return m_identifier;
}

void Identifier::setIdentifier(const QString& identifier)
{
  m_identifier = identifier;
}

const QList<QualifiedIdentifier>& Identifier::templateIdentifiers() const
{
  return m_templateIdentifiers;
}

void Identifier::appendTemplateIdentifier(const QualifiedIdentifier& identifier)
{
  m_templateIdentifiers.append(identifier);
}

void Identifier::clearTemplateIdentifiers()
{
  m_templateIdentifiers.clear();
}

void Identifier::setTemplateIdentifiers(const QList<QualifiedIdentifier>& templateIdentifiers)
{
  m_templateIdentifiers = templateIdentifiers;
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

static const int idguess = 4;

QualifiedIdentifier::QualifiedIdentifier(const QString id)
{
  if (id.startsWith("::")) {
    m_explicitlyGlobal = true;
    m_qid = id.mid(2);
  } else {
    m_explicitlyGlobal = false;
    m_qid = id;
  }

  m_idSplits.append(0);
  int split = 0;
  forever {
    split = m_qid.indexOf("::", split);
    if (split == -1)
      break;
    m_idSplits.append(split + 2);
    split += 2;
  }
}

QualifiedIdentifier::QualifiedIdentifier(const Identifier& id)
{
  if (id.m_identifier.isEmpty()) {
    m_explicitlyGlobal = true;
  } else {
    m_explicitlyGlobal = false;
    m_qid = id.m_identifier;
    m_idSplits.append(0);
  }
}

QualifiedIdentifier::QualifiedIdentifier()
  : m_explicitlyGlobal(false)
{
}

QualifiedIdentifier::QualifiedIdentifier(const QualifiedIdentifier& id, int reserve)
{
  if (reserve)
    m_qid.reserve(reserve);

  m_qid = id.m_qid;
  m_idSplits = id.m_idSplits;
  m_explicitlyGlobal = id.m_explicitlyGlobal;
}

QStringList QualifiedIdentifier::toStringList() const
{
  QStringList ret;
  if (m_explicitlyGlobal)
    ret.append(QString());

  ret << m_qid.split("::");

  return ret;
}

QString QualifiedIdentifier::toString(bool ignoreExplicitlyGlobal) const
{
  if (ignoreExplicitlyGlobal || !explicitlyGlobal())
    return m_qid;
  else
    return QString("::") + m_qid;
}

QualifiedIdentifier QualifiedIdentifier::merge(const QualifiedIdentifier& base) const
{
  if (explicitlyGlobal())
    return *this;

  QualifiedIdentifier ret(base, base.m_qid.length() + 2 + m_qid.length());
  if (ret.count())
    ret.m_qid.append("::");
  ret.m_qid.append(m_qid);

  const int offset = ret.m_qid.length();
  for (int i = 0; i < m_idSplits.count(); ++i)
    ret.m_idSplits.append(m_idSplits[i] + offset);

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

bool QualifiedIdentifier::explicitlyGlobal() const
{
  // True if started with "::"
  return m_explicitlyGlobal;
}

void QualifiedIdentifier::setExplicitlyGlobal(bool eg)
{
  m_explicitlyGlobal = eg;
}

bool QualifiedIdentifier::operator==(const QualifiedIdentifier& rhs) const
{
  // Fast path
  if (m_idSplits.count() != rhs.m_idSplits.count())
    return false;

  return m_qid == rhs.m_qid;
}

bool QualifiedIdentifier::operator!=(const QualifiedIdentifier& rhs) const
{
  return !operator==(rhs);
}

QualifiedIdentifier::MatchTypes QualifiedIdentifier::match(const Identifier& rhs) const
{
  const int difference = count() - 1;
  if (difference == 0) {
    if (m_qid == rhs.m_identifier)
      return ExactMatch;
    else
      return NoMatch;

  } else if (difference > 0) {
    if (m_qid.endsWith(rhs.m_identifier))
      return Contains;
  }

  return NoMatch;
}

QualifiedIdentifier::MatchTypes QualifiedIdentifier::match(const QualifiedIdentifier& rhs) const
{
  const int difference = count() - rhs.count();
  if (difference == 0) {
    if (m_qid == rhs.m_qid)
      return ExactMatch;
    else
      return NoMatch;

  } else if (difference > 0) {
    // eg 6 vs 4... we need at(2)
    if (rhs.explicitlyGlobal())
      return NoMatch;

    //if (m_idSplits[count() - 1 - rhs.count()] == rhs.m_qid.length())
      if (m_qid.endsWith(rhs.m_qid))
        return Contains;

  } else {
    if (explicitlyGlobal())
      return NoMatch;

    //if (rhs.m_idSplits[rhs.count() - 1 - count()] == m_qid.length())
      if (rhs.m_qid.endsWith(m_qid))
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
  if (simpleIdentifier.exactMatch(m_identifier))
    return QString("%1%2").arg(m_identifier.length()).arg(m_identifier);

  // Get the encoded utf form
  QString utf = QString::fromLatin1(m_identifier.toUtf8());

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
  if (m_qid.isEmpty()) {
    if (id.m_identifier.isEmpty()) {
      m_explicitlyGlobal = true;
    } else {
      m_idSplits.append(0);
      m_qid.append(id.m_identifier);
    }
  } else {
    m_qid.append("::");
    m_idSplits.append(m_qid.length());
    m_qid.append(id.m_identifier);
  }
}

void QualifiedIdentifier::push(const QualifiedIdentifier& id)
{
  if (m_qid.isEmpty()) {
    m_qid = id.m_qid;
    m_idSplits = id.m_idSplits;
    m_explicitlyGlobal = id.m_explicitlyGlobal;

  } else {
    m_qid.append("::");
    const int offset = m_qid.length();
    m_qid.append(id.m_qid);
    for (int i = 0; i < id.m_idSplits.count(); ++i)
      m_idSplits.append(offset + id.m_idSplits[i]);
  }
}

void QualifiedIdentifier::pop()
{
  m_qid = m_qid.left(m_idSplits[m_idSplits.count() - 1] - 2);
  m_idSplits.resize(m_idSplits.count() - 1);
}

void QualifiedIdentifier::clear()
{
  m_qid.clear();
  m_idSplits.clear();
  m_explicitlyGlobal = false;
}

Identifier QualifiedIdentifier::at(int i) const
{
  return Identifier(m_qid.mid(m_idSplits[i], (i == m_idSplits.count() - 1) ? -1 : m_idSplits[i + 1] - 2));
}

// kate: indent-width 2;

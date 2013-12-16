/*
 * This file is part of KDevelop
 * Copyright 2012-2013 Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "tst_identifier.h"

#include <language/duchain/identifier.h>
#include <language/duchain/indexedstring.h>

#include <qtest_kde.h>

#include <tests/testcore.h>
#include <tests/autotestshell.h>

QTEST_KDEMAIN(TestIdentifier, NoGUI);

using namespace KDevelop;

void TestIdentifier::initTestCase()
{
  AutoTestShell::init();
  TestCore::initialize(Core::NoUi);
}

void TestIdentifier::cleanupTestCase()
{
  TestCore::shutdown();
}

void TestIdentifier::testIdentifier()
{
  QFETCH(QString, stringId);
  const IndexedString indexedStringId(stringId);

  Identifier id(stringId);
  QCOMPARE(id.isEmpty(), stringId.isEmpty());
  QCOMPARE(id, Identifier(stringId));
  QVERIFY(!(id != Identifier(stringId)));
  QCOMPARE(id, Identifier(stringId));
  QCOMPARE(id, Identifier(indexedStringId));
  QCOMPARE(id.identifier(), indexedStringId);
  QCOMPARE(id.toString(), stringId);
  QVERIFY(id.nameEquals(Identifier(stringId)));
  QVERIFY(!id.isUnique());

  if (stringId.isEmpty()) {
    QVERIFY(id.inRepository());
    QVERIFY(Identifier(id).inRepository());
    QVERIFY(Identifier(indexedStringId).inRepository());
  }

  Identifier copy = id;
  QCOMPARE(copy, id);
  copy = copy;
  QCOMPARE(copy, id);
  copy = Identifier();
  QVERIFY(copy.isEmpty());
  copy = id;
  QCOMPARE(copy, id);

  IndexedIdentifier indexedId(id);
  QVERIFY(indexedId == id);
  QCOMPARE(indexedId, IndexedIdentifier(id));
  QCOMPARE(indexedId.isEmpty(), stringId.isEmpty());
  QCOMPARE(indexedId.identifier(), id);
  IndexedIdentifier indexedCopy = indexedId;
  QCOMPARE(indexedCopy, indexedId);
  indexedCopy = indexedCopy;
  QCOMPARE(indexedCopy, indexedId);
  indexedCopy = IndexedIdentifier();
  QVERIFY(indexedCopy.isEmpty());
  indexedCopy = indexedId;
  QCOMPARE(indexedCopy, indexedId);

  Identifier moved = std::move(id);
  QVERIFY(id.isEmpty());
  QVERIFY(id.inRepository());
  QCOMPARE(moved, copy);

  IndexedIdentifier movedIndexed = std::move(indexedId);
  QVERIFY(indexedId.isEmpty());
  QCOMPARE(movedIndexed, indexedCopy);
}

void TestIdentifier::testIdentifier_data()
{
  QTest::addColumn<QString>("stringId");

  QTest::newRow("empty") << QString();
  QTest::newRow("foo") << QString("foo");
  QTest::newRow("bar") << QString("bar");
  //TODO: test template identifiers
}

void TestIdentifier::testQualifiedIdentifier()
{
  QFETCH(QString, stringId);
  const QStringList list = stringId.split("::", QString::SkipEmptyParts);

  QualifiedIdentifier id(stringId);
  QCOMPARE(id.isEmpty(), stringId.isEmpty());
  QCOMPARE(id, QualifiedIdentifier(stringId));
  QVERIFY(!(id != QualifiedIdentifier(stringId)));
  QCOMPARE(id, QualifiedIdentifier(stringId));
  if (list.size() == 1) {
    QCOMPARE(id, QualifiedIdentifier(Identifier(list.last())));
  } else if (list.isEmpty()) {
    QualifiedIdentifier empty{Identifier()};
    QCOMPARE(id, empty);
    QVERIFY(empty.isEmpty());
    QVERIFY(empty.inRepository());
  }
  QCOMPARE(id.toString(), stringId);
  QCOMPARE(id.toStringList(), list);
  QCOMPARE(id.top(), Identifier(list.isEmpty() ? QString() : list.last()));

  if (stringId.isEmpty()) {
    QVERIFY(id.inRepository());
    QVERIFY(QualifiedIdentifier(id).inRepository());
  }

  QualifiedIdentifier copy = id;
  QCOMPARE(copy, id);
  copy = copy;
  QCOMPARE(copy, id);
  copy = QualifiedIdentifier();
  QVERIFY(copy.isEmpty());
  copy = id;
  QCOMPARE(copy, id);

  IndexedQualifiedIdentifier indexedId(id);
  QVERIFY(indexedId == id);
  QCOMPARE(indexedId, IndexedQualifiedIdentifier(id));
  QCOMPARE(indexedId.isValid(), !stringId.isEmpty());
  QCOMPARE(indexedId.identifier(), id);
  IndexedQualifiedIdentifier indexedCopy = indexedId;
  QCOMPARE(indexedCopy, indexedId);
  indexedCopy = indexedCopy;
  QCOMPARE(indexedCopy, indexedId);
  indexedCopy = IndexedQualifiedIdentifier();
  QVERIFY(!indexedCopy.isValid());
  indexedCopy = indexedId;
  QCOMPARE(indexedCopy, indexedId);

  QualifiedIdentifier moved = std::move(id);
  QVERIFY(id.isEmpty());
  QCOMPARE(moved, copy);

  IndexedQualifiedIdentifier movedIndexed = std::move(indexedId);
  QVERIFY(!indexedId.isValid());
  QCOMPARE(movedIndexed, indexedCopy);
}

void TestIdentifier::testQualifiedIdentifier_data()
{
  QTest::addColumn<QString>("stringId");

  QTest::newRow("empty") << QString();
  QTest::newRow("foo") << "foo";
  QTest::newRow("foo::bar") << "foo::bar";
  //TODO: test template identifiers
}

void TestIdentifier::benchIdentifierCopyConstant()
{
  QBENCHMARK {
    Identifier identifier("Asdf");
    identifier.index();
    Identifier copy(identifier);
  }
}

void TestIdentifier::benchIdentifierCopyDynamic()
{
  QBENCHMARK {
    Identifier identifier("Asdf");
    Identifier copy(identifier);
  }
}

void TestIdentifier::benchQidCopyPush()
{
  QBENCHMARK {
    Identifier id("foo");
    QualifiedIdentifier base(id);
    QualifiedIdentifier copy(base);
    copy.push(id);
  }
}

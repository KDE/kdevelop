/***************************************************************************
 *   KDevelop Automake Support Tests                                       *
 *   Copyright (C) 2005 by Matt Rogers <mattr@kde.org>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "testinterface.h"
#include <QtTest/QtTest>
#include "makefileinterface.h"

QTEST_MAIN(TestInterface)

void TestInterface::testIsVariable()
{
	QFETCH(QString, variable);
	QFETCH(bool, result);

	QCOMPARE( MakefileInterface::isVariable( variable ), result );
}

void TestInterface::testIsVariable_data()
{
	QTest::addColumn<QString>("variable");
	QTest::addColumn<bool>("result");

	QTest::newRow("v1") << "$(top_srcdir1)" << true;
	QTest::newRow("v2") << "$(!DFDFDDFFD)" << false;
	QTest::newRow("v3") << "@MY_1VARIABLE@" << true;
	QTest::newRow("v4") << "@%#$ASDFSDF#@" << false;
	QTest::newRow("v5") << "bin_PROGRAMS" << false;
}

void TestInterface::testCanonicalize()
{
	QFETCH(QString, target);
	QFETCH(QString, result);

	QCOMPARE( MakefileInterface::canonicalize( target ), result );
}

void TestInterface::testCanonicalize_data()
{
	QTest::addColumn<QString>("target");
	QTest::addColumn<QString>("result");
	QTest::newRow("v1") << "libfoo.la" << "libfoo_la";
	QTest::newRow("v2") << "libbar++.la" << "libbar___la";
	QTest::newRow("v3") << "lib123!#$%^.la" << "lib123______la";
}

#include "testinterface.moc"

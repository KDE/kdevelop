/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "generationexpressionsolvertest.h"
#include <generationexpressionsolver.h>

QTEST_MAIN( GenerationExpressionSolverTest )

using namespace KDevelop;

void GenerationExpressionSolverTest::testRun_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("result");

    QTest::newRow("nothing") << "nothing" << "nothing";
    QTest::newRow("true") << "$<1:hola>" << "hola";
    QTest::newRow("false") << "$<0:hola>" << QString();
    QTest::newRow("streq_true") << "$<STREQUAL:a,a>" << "1";
    QTest::newRow("streq_false") << "$<STREQUAL:a,b>" << "0";
    QTest::newRow("and0") << "$<AND:0,1>" << "0";
    QTest::newRow("and1") << "$<AND:1,1>" << "1";
    QTest::newRow("or0") << "$<OR:1,0>" << "1";
    QTest::newRow("or1") << "$<OR:0,0>" << "0";
    QTest::newRow("not") << "$<NOT:0>" << "1";
    QTest::newRow("bool1") << "$<BOOL:True>" << "1";
    QTest::newRow("bool0") << "$<BOOL:False>" << "0";
    QTest::newRow("target_property1") << "$<TARGET_PROPERTY:TEST_PROPERTY>" << "hola";
    QTest::newRow("target_property2") << "$<TARGET_PROPERTY:tgt,TEST_PROPERTY>" << "hola";
    QTest::newRow("build_interface") << "$<BUILD_INTERFACE:hola>" << "hola";
    QTest::newRow("install_interface") << "$<INSTALL_INTERFACE:hola>" << "hola";
    QTest::newRow("install_interface2") << "$<INSTALL_INTERFACE:$<WOP>/falala>" << "bonjour/falala";
    QTest::newRow("variable") << "$<1:$<WOP>>" << "bonjour";
    QTest::newRow("variable2") << "$<1:$<WOP>/falala>" << "bonjour/falala";
}

void GenerationExpressionSolverTest::testRun()
{
    QFETCH(QString, input);
    QFETCH(QString, result);

    CMakeProperties props;
    props[TargetProperty]["tgt"]["TEST_PROPERTY"] = QStringList("hola");
    GenerationExpressionSolver solver(props, QHash<QString,QString>());
    solver.defineVariable("WOP", "bonjour");
    solver.setTargetName("tgt");
    QString ret = solver.run(input);
    QCOMPARE(ret, result);
}

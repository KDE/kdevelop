/* KDevelop CMake Support
 *
 * Copyright 2008 Aleix Pol Gonzalez <aleixpol@gmail.com>
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

#include "cmakeduchaintest.h"
#include "cmakeprojectvisitor.h"

#include <identifier.h>
#include <declaration.h>
#include <duchainlock.h>
#include <duchain.h>
#include <simplerange.h>

using namespace KDevelop;

QTEST_MAIN( CMakeDUChainTest )

Q_DECLARE_METATYPE(QList<SimpleRange>)

CMakeDUChainTest::CMakeDUChainTest()
{
	m_fakeContext = new TopDUContext(HashedString("test"), SimpleRange(0,0,0,0));
}

CMakeDUChainTest::~CMakeDUChainTest()
{}

void CMakeDUChainTest::testDUChainWalk_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QList<SimpleRange> >("ranges");
    
    QTest::newRow("simple") << "project(simpletest)\n" << QList<SimpleRange>();
    
    QList<SimpleRange> sr;
    sr.append(SimpleRange(1, 4, 1, 7));
    QTest::newRow("simple 2") <<
            "project(simpletest)\n"
            "set(var a b c)\n" << sr;
    
    QTest::newRow("simple 3") <<
            "project(simpletest)\n"
            "find_package(KDE4)\n" << QList<SimpleRange>();
    

    sr.append(SimpleRange(2, 4, 2, 8));
    QTest::newRow("simple 2 with use") <<
            "project(simpletest)\n"
            "set(var a b c)\n"
	    "set(var2 ${var})\n"<< sr;
}

void CMakeDUChainTest::testDUChainWalk()
{
    QFETCH(QString, input);
    QFETCH(QList<SimpleRange>, ranges);
    
    QFile file("cmake_duchain_test");
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    
    QTextStream out(&file);
    out << input;
    file.close();
    CMakeFileContent code=CMakeListsParser::readCMakeFile(file.fileName());
    file.remove();
    QVERIFY(code.count() != 0);
    
    MacroMap mm;
    VariableMap vm;
    
    CMakeProjectVisitor v(file.fileName(), m_fakeContext);
    v.setVariableMap(&vm);
    v.setMacroMap(&mm);
//     v.setModulePath();
    v.walk(code, 0);
    
    DUChainWriteLocker lock(DUChain::lock());
    TopDUContext* ctx=v.context();
    QVERIFY(ctx);
    
    QVector<Declaration*> declarations=ctx->localDeclarations();
    /*for(int i=0; i<declarations.count(); i++)
    {
        qDebug() << "ddd" << declarations[i]->identifier().toString();
        if(!ranges.contains(declarations[i]->range()))
            qDebug() << "doesn't exist " << declarations[i]->range().start.column
                << declarations[i]->range().end.column;
        QVERIFY(ranges.contains(declarations[i]->range()));
    }*/
    
    foreach(const SimpleRange& sr, ranges)
    {
        bool found=false;
        for(int i=0; !found && i<declarations.count(); i++)
        {
            if(declarations[i]->range()==sr)
                found=true;
            else
                qDebug() << "diff " << declarations[i]->range().start.column << declarations[i]->range().end.column;
        }
        if(!found)
            qDebug() << "doesn't exist " << sr.start.column << sr.end.column;
        QVERIFY(found);
    }
    
    DUChain::self()->clear();
}

#include "cmakeduchaintest.moc"


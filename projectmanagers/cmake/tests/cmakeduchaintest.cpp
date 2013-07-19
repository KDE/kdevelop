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

#include <language/duchain/identifier.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/editor/simplerange.h>
#include <language/duchain/dumpchain.h>
#include <language/duchain/use.h>
#include <language/duchain/indexedstring.h>
#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <language/duchain/parsingenvironment.h>

using namespace KDevelop;

QTEST_MAIN( CMakeDUChainTest )

Q_DECLARE_METATYPE(QList<RangeInRevision>)
Q_DECLARE_METATYPE(QSet<RangeInRevision>)

void CMakeDUChainTest::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
}

void CMakeDUChainTest::cleanupTestCase()
{
    TestCore::shutdown();
}

void CMakeDUChainTest::testDUChainWalk_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QList<RangeInRevision> >("ranges");

    QTest::newRow("simple") << "project(simpletest)\n" << QList<RangeInRevision>();

    QList<RangeInRevision> sr;
    sr.append(RangeInRevision(1, 4, 1, 7));
    QTest::newRow("simple 2") <<
            "project(simpletest)\n"
            "set(var a b c)\n" << sr;

    QTest::newRow("simple 3") <<
            "project(simpletest)\n"
            "find_package(KDE4)\n" << QList<RangeInRevision>();


    sr.append(RangeInRevision(2, 4, 2, 8));
    QTest::newRow("simple 2 with use") <<
            "project(simpletest)\n"
            "set(var a b c)\n"
	    "set(var2 ${var})\n"<< sr;

    sr.clear();
    sr.append(RangeInRevision(1, 15, 1, 18));
    QTest::newRow("simple 2 with use") <<
            "project(simpletest)\n"
            "add_executable(var a b c)\n" << sr;
}

void CMakeDUChainTest::testDUChainWalk()
{
    QFETCH(QString, input);
    QFETCH(QList<RangeInRevision>, ranges);

    KDevelop::ReferencedTopDUContext m_fakeContext;
    {
        DUChainWriteLocker lock(DUChain::lock());
        m_fakeContext = new TopDUContext(IndexedString("test"), RangeInRevision(0,0,0,0));
        DUChain::self()->addDocumentChain(m_fakeContext);
    }
    
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
    CacheValues cv;
    vm.insert("CMAKE_CURRENT_SOURCE_DIR", QStringList("."));

    CMakeProjectVisitor v(file.fileName(), m_fakeContext);
    v.setVariableMap(&vm);
    v.setMacroMap(&mm);
    v.setCacheValues(&cv);
//     v.setModulePath();
    v.walk(code, 0);

    ReferencedTopDUContext ctx=v.context();
    QVERIFY(ctx);

    DUChainWriteLocker lock(DUChain::lock());
    QVector<Declaration*> declarations=ctx->localDeclarations();
    foreach(Declaration* d, declarations) {
        QCOMPARE(d->topContext()->parsingEnvironmentFile()->language(), IndexedString("cmake"));
    }
    /*for(int i=0; i<declarations.count(); i++)
    {
        qDebug() << "ddd" << declarations[i]->identifier().toString();
        if(!ranges.contains(declarations[i]->range()))
            qDebug() << "doesn't exist " << declarations[i]->range().start.column
                << declarations[i]->range().end.column;
        QVERIFY(ranges.contains(declarations[i]->range()));
    }*/

    foreach(const RangeInRevision& sr, ranges)
    {
        bool found=false;
        for(int i=0; !found && i<declarations.count(); i++)
        {
            if(declarations[i]->range()==sr)
                found=true;
//             else
//                 qDebug() << "diff " << declarations[i]->range().start.column << declarations[i]->range().end.column
//                     << declarations[i]->range().end.line;
        }
        if(!found)
            qDebug() << "doesn't exist " << sr.start.column << sr.end.column;
        QVERIFY(found);
    }
    
    DUChain::self()->removeDocumentChain(m_fakeContext);
}

void CMakeDUChainTest::testUses_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QStringList>("decls");
    QTest::addColumn<QSet<RangeInRevision> >("uses");

    QStringList input= QStringList() <<
            "project(simpletest)\n"
            "set(var a b c)\n"
            "set(var2 ${var})\n" <<
            
            "set(CMAKE_MODULE_PATH .)\n"
            "include(included)\n"
            "set(usinginc aa${avalue})\n" <<
            
            "macro(bla kk)\n"
                "message(STATUS ${kk})\n"
            "endmacro(bla)\n"
            "bla(hola)\n" <<
            
            "set(var 1)\n"
            "if(var)\n"
                "message(STATUS \"life rocks\")\n"
            "endif(var)\n"
            "message(STATUS \"------- done\")\n";
            
    QTest::newRow("empty") << "message(STATUS ueee)\n" << QStringList() << QSet<RangeInRevision>();
    QTest::newRow("defanduse") << input[0] << (QStringList() << "var" << "var2") << (QSet<RangeInRevision>() << RangeInRevision(2,11, 2,11+3) );
    QTest::newRow("include") << input[1] << (QStringList() << "CMAKE_MODULE_PATH" << "usinginc")
        << (QSet<RangeInRevision>() << RangeInRevision(2,17, 2,17+6));
    
    QTest::newRow("macro") << input[2] << (QStringList() << "bla")
        << (QSet<RangeInRevision>() << RangeInRevision(2,9, 2,9+3) << RangeInRevision(3,0,  3,3)/* << RangeInRevision(10,3, 10,3+3)*/);
        
    QTest::newRow("conditional") << input[3] << QStringList("var")
        << (QSet<RangeInRevision>() << RangeInRevision(1,3, 1,3+3) << RangeInRevision(3,6, 3,6+3));
        
    QTest::newRow("included_macro") <<
        "set(CMAKE_MODULE_PATH .)\n"
        "include(included)\n"
        "mymacro(33)\nmessage(STATUS 33)\n"
        << QStringList("CMAKE_MODULE_PATH")
        << (QSet<RangeInRevision>() << RangeInRevision(2,0, 2,0+7));
}

void CMakeDUChainTest::testUses()
{
    QFETCH(QString, input);
    QFETCH(QStringList, decls);
    QFETCH(QSet<RangeInRevision>, uses);

    QTemporaryFile filetemp("cmake_duchain_test");
    filetemp.open();
    
    QFile file(filetemp.fileName());
    
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream out(&file);
    out << input;
    file.close();
    CMakeFileContent code=CMakeListsParser::readCMakeFile(file.fileName());
    int endl=1, endc=1;
    
    for(int i=0; i<input.count(); i++, endc++)
    {
        if(input[i]=='\n')
        {
            endl++;
            endc=0;
        }
    }
    ReferencedTopDUContext m_fakeContext=new TopDUContext(IndexedString(file.fileName()), RangeInRevision(0,0, endl, endc));
    DUChain::self()->addDocumentChain(m_fakeContext);
    
    QString inputIncluded=
        "set(avalue 33)\n"
        "macro(mymacro kk)\n"
            "message(STATUS hello ${kk})\n"
        "endmacro(mymacro)\n";
    
    QFile includedFile("included.cmake");
    QVERIFY(includedFile.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream out2(&includedFile);
    out2 << inputIncluded;
    includedFile.close();
    
    QVERIFY(!code.isEmpty());

    MacroMap mm;
    VariableMap vm;
    CacheValues cv;
    vm.insert("CMAKE_CURRENT_SOURCE_DIR", QStringList("."));

    CMakeProjectVisitor v(file.fileName(), m_fakeContext);
    v.setVariableMap(&vm);
    v.setMacroMap(&mm);
    v.setCacheValues(&cv);
    v.walk(code, 0);

    DUChainWriteLocker lock(DUChain::lock());
    TopDUContext* ctx=v.context();
    QVERIFY(ctx);
    QVERIFY(ctx->indexed().data());
//     KDevelop::dumpDUContext(ctx);
    QCOMPARE(ctx->range().start.line, 0);
    QVector<Declaration*> declarations=ctx->localDeclarations();
   
    qDebug() << "nyeeee" << ctx << declarations;
    
//     if(decls.count() != declarations.count())
//     {
//         for(int i=0; i<declarations.count(); i++) {
//             qDebug() << "decl" << declarations[i]->toString();
//         }
//     }
    
    QCOMPARE(decls.count(), declarations.count());
    for(int i=0; i<decls.count(); i++) {
        QVERIFY(declarations[i]->inSymbolTable());
    }
    
    for(int i=0; i<decls.count(); i++)
    {
//         qDebug() << "yeeeeeeee" << decls[i] << ctx->findDeclarations(Identifier(decls[i]));
        QCOMPARE(decls[i], declarations[i]->identifier().toString());
        QCOMPARE(ctx->findDeclarations(Identifier(decls[i])).count(), 1);
        QCOMPARE(ctx->findLocalDeclarations(Identifier(decls[i])).count(), 1);
    }
    
    QCOMPARE(ctx->usesCount(), uses.count());

    QSet<RangeInRevision> found;
    for (int i=0; i<ctx->usesCount(); i++) {
        found.insert(ctx->uses()[i].m_range);
    }
    QCOMPARE(found, uses);
    
    DUChain::self()->removeDocumentChain(m_fakeContext);

    includedFile.remove();
    file.remove();
}

#include "cmakeduchaintest.moc"


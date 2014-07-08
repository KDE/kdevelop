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

#include "cmakeloadprojecttest.h"
#include "cmake-test-paths.h"

#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/topducontext.h>

#include "cmake-test-paths.h"

#include "cmListFileLexer.h"
#include "cmakelistsparser.h"
#include "cmakeprojectvisitor.h"
#include "cmakeast.h"
#include <cmakeparserutils.h>
#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <cmakeprojectdata.h>

inline QDebug &operator<<(QDebug debug, const Target &target)
{ debug << target.name; return debug.maybeSpace(); }

QTEST_MAIN( CMakeLoadProjectTest )

using namespace KDevelop;

void CMakeLoadProjectTest::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
}

void CMakeLoadProjectTest::cleanupTestCase()
{
    TestCore::shutdown();
}

void CMakeLoadProjectTest::testTinyCMakeProject()
{
    CMakeProjectData v = parseProject( QString(CMAKE_TESTS_PROJECTS_DIR)+"/tiny_project" );
    QCOMPARE(v.targets.count(), 1);
    QCOMPARE(v.targets.at( 0 ).name, QString("foo") );
    QCOMPARE(v.targets.at( 0 ).files, QStringList() << "foo.cpp" );
    QCOMPARE(v.vm.value("CMAKE_INCLUDE_CURRENT_DIR"), QStringList("OFF"));
}

#if QT_VERSION <= 0x050000
void CMakeLoadProjectTest::testBug335803()
{
    CMakeProjectData v = parseProject(CMAKE_TESTS_PROJECTS_DIR "/bug335803");
    QCOMPARE(v.projectName, QString("bug335803"));
    QStringList names;
    foreach(const Target& t, v.targets) {
        names << t.name;
    }
    QCOMPARE(v.targets.count(), 8);
    names.sort();
    QCOMPARE(names[0], QLatin1String("echo-a"));
    QCOMPARE(names[1], QLatin1String("echo-b"));
    QCOMPARE(names[2], QLatin1String("echo-custom_name"));
    QCOMPARE(names[3], QLatin1String("echo-d"));
    QCOMPARE(names[4], QLatin1String("echo2-a"));
    QCOMPARE(names[5], QLatin1String("echo2-b"));
    QCOMPARE(names[6], QLatin1String("echo2-custom_name"));
    QCOMPARE(names[7], QLatin1String("echo2-d")); // This one was missing before bug #335803 got fixed
}

void CMakeLoadProjectTest::testSmallQt4Project()
{
    CMakeProjectData v = parseProject(CMAKE_TESTS_PROJECTS_DIR "/qt4app");
    QCOMPARE(v.targets.count(), 1);
    QCOMPARE(v.projectName, QString("qt4app"));
    QCOMPARE(v.targets.at( 0 ).name, QString("qt4app") );
    QCOMPARE(v.targets.at( 0 ).files, QStringList() << "qt4app.cpp" << "main.cpp" );
}

int findTarget(const QVector<Target>& targets, const QString& name)
{
    for(int i=0, count=targets.count(); i<count; ++i) {
        if(targets[i].name==name)
            return i;
    }
    return -1;
}

void CMakeLoadProjectTest::testSmallKDE4Project()
{
    CMakeProjectData v = parseProject(CMAKE_TESTS_PROJECTS_DIR "/kde4app");
    QCOMPARE(v.targets.count(), 3);
    QCOMPARE(v.projectName, QString("kde4app"));

    int idx = findTarget(v.targets, "kde4app");
    QVERIFY(idx>=0);
    QCOMPARE(v.targets.at( idx ).name, QString("kde4app") );
    QCOMPARE(v.targets.at( idx ).files, QStringList() << "kde4app.cpp" << "main.cpp" << "kde4appview.cpp"
                                                      << CMAKE_TESTS_PROJECTS_DIR "/kde4app/ui_kde4appview_base.h"
                                                      << CMAKE_TESTS_PROJECTS_DIR "/kde4app/ui_prefs_base.h"
                                                      << CMAKE_TESTS_PROJECTS_DIR "/kde4app/settings.cpp"
                                                      << CMAKE_TESTS_PROJECTS_DIR "/kde4app/settings.h" );
    int testIdx = findTarget(v.targets, "testkde4app");
    QVERIFY(testIdx>=0);
    QCOMPARE(v.targets.at( testIdx ).name, QString("testkde4app") );
    QCOMPARE(v.targets.at( testIdx ).files, QStringList() << "kde4app.cpp");

    int uninstallIdx = findTarget(v.targets, "uninstall");
    QVERIFY(uninstallIdx>=0);
    QCOMPARE(v.targets.at( uninstallIdx ).name, QString("uninstall") );
    QCOMPARE(v.vm.value("CMAKE_INCLUDE_CURRENT_DIR"), QStringList("ON"));
}
#endif

void CMakeLoadProjectTest::testSmallProjectWithTests()
{
    CMakeProjectData v = parseProject(CMAKE_TESTS_PROJECTS_DIR "/unit_tests");
    QCOMPARE(v.testSuites.count(), 5);
    QCOMPARE(v.projectName, QString("unittests"));
    
    QCOMPARE(v.testSuites.at(0).name, QString("success"));
    QCOMPARE(v.testSuites.at(0).arguments.count(), 0);
    
    QCOMPARE(v.testSuites.at(3).name, QString("test_four"));
    QCOMPARE(v.testSuites.at(3).arguments.count(), 1);
    QCOMPARE(v.testSuites.at(3).arguments.at(0), QString("4"));
}

#if QT_VERSION <= 0x050000
void CMakeLoadProjectTest::testKDE4ProjectWithTests()
{
    CMakeProjectData v = parseProject(CMAKE_TESTS_PROJECTS_DIR "/unit_tests_kde");
    QCOMPARE(v.testSuites.count(), 1); //cmake-test-unittestskde
    QCOMPARE(v.projectName, QString("unittestskde"));
    
    QCOMPARE(v.testSuites.at(0).name, QString("cmake-test-unittestskde"));
    QCOMPARE(v.testSuites.at(0).arguments.count(), 0);
}
#endif

CMakeProjectData CMakeLoadProjectTest::parseProject( const QString& sourcedir )
{
    QString projectfile = sourcedir+"/CMakeLists.txt";
    CMakeFileContent code=CMakeListsParser::readCMakeFile(projectfile);

    QPair<VariableMap,QStringList> initials = CMakeParserUtils::initialVariables();
    CMakeProjectData data;
    data.vm = initials.first;
    data.vm.insert("CMAKE_SOURCE_DIR", QStringList(sourcedir));
    data.vm.insert("CMAKE_PREFIX_PATH", QString::fromLatin1(TEST_PREFIX_PATH).split(';', QString::SkipEmptyParts));
    
    KDevelop::ReferencedTopDUContext buildstrapContext=new TopDUContext(IndexedString("buildstrap"), RangeInRevision(0,0, 0,0));
    DUChain::self()->addDocumentChain(buildstrapContext);
    ReferencedTopDUContext ref=buildstrapContext;
    QStringList modulesPath = data.vm["CMAKE_MODULE_PATH"];
    foreach(const QString& script, initials.second)
    {
        ref = CMakeParserUtils::includeScript(CMakeProjectVisitor::findFile(script, modulesPath, QStringList()), ref, &data, sourcedir, QMap<QString,QString>());
    }
    
    data.vm.insert("CMAKE_CURRENT_BINARY_DIR", QStringList(sourcedir));
    data.vm.insert("CMAKE_CURRENT_LIST_FILE", QStringList(projectfile));
    data.vm.insert("CMAKE_CURRENT_SOURCE_DIR", QStringList(sourcedir));
    data.vm.insert("KDE4_BUILD_TESTS", QStringList("True"));

    CMakeProjectVisitor v(projectfile, ref);
    v.setVariableMap(&data.vm);
    v.setMacroMap(&data.mm);
    v.setCacheValues(&data.cache);
    v.setModulePath(modulesPath);
    v.setProperties(data.properties);
    QMap<QString, QString> env;
    env["CMAKE_PREFIX_PATH"] = QString::fromLatin1(TEST_ENV_PREFIX_PATH);
    env["CMAKE_INCLUDE_PATH"] = QString::fromLatin1(TEST_ENV_INCLUDE_PATH);
    env["CMAKE_LIBRARY_PATH"] = QString::fromLatin1(TEST_ENV_LIBRARY_PATH);
    v.setEnvironmentProfile( env );
    v.walk(code, 0);
    
    data.projectName=v.projectName();
    data.subdirectories=v.subdirectories();
    data.targets=v.targets();
    data.properties=v.properties();
    data.testSuites=v.testSuites();
    
    //printSubdirectories(data->subdirectories);
    
    data.vm.remove("CMAKE_CURRENT_LIST_FILE");
    data.vm.remove("CMAKE_CURRENT_LIST_DIR");
    data.vm.remove("CMAKE_CURRENT_SOURCE_DIR");
    data.vm.remove("CMAKE_CURRENT_BINARY_DIR");

    return data;
}



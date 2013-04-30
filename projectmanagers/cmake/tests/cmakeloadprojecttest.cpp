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


#include "cmListFileLexer.h"
#include "cmakelistsparser.h"
#include "cmakeprojectvisitor.h"
#include "cmakeast.h"
#include <cmakeparserutils.h>
#include <tests/autotestshell.h>
#include <cmakeprojectdata.h>

QTEST_MAIN( CMakeLoadProjectTest )

using namespace KDevelop;

CMakeLoadProjectTest::CMakeLoadProjectTest()
{
    AutoTestShell::init();
    KDevelop::Core::initialize(0, KDevelop::Core::NoUi);
}

CMakeLoadProjectTest::~CMakeLoadProjectTest()
{
    KDevelop::Core::self()->shutdown();
}

void CMakeLoadProjectTest::testTinyCMakeProject()
{
    CMakeProjectData v = parseProject( QString(CMAKE_TESTS_PROJECTS_DIR)+"/tiny_project" );
    QCOMPARE(v.targets.count(), 1);
    QCOMPARE(v.targets.at( 0 ).name, QString("foo") );
    QCOMPARE(v.targets.at( 0 ).files, QStringList() << "foo.cpp" );
    QCOMPARE(v.vm.value("CMAKE_INCLUDE_CURRENT_DIR"), QStringList("OFF"));
}

void CMakeLoadProjectTest::testSmallQt4Project()
{
    CMakeProjectData v = parseProject(CMAKE_TESTS_PROJECTS_DIR "/qt4app");
    QCOMPARE(v.targets.count(), 1);
    QCOMPARE(v.projectName, QString("qt4app"));
    QCOMPARE(v.targets.at( 0 ).name, QString("qt4app") );
    QCOMPARE(v.targets.at( 0 ).files, QStringList() << "qt4app.cpp" << "main.cpp" );
}


void CMakeLoadProjectTest::testSmallKDE4Project()
{
    CMakeProjectData v = parseProject(CMAKE_TESTS_PROJECTS_DIR "/kde4app");
    QCOMPARE(v.targets.count(), 3);
    QCOMPARE(v.projectName, QString("kde4app"));
    QCOMPARE(v.targets.at( 0 ).name, QString("kde4app") );
    QCOMPARE(v.targets.at( 0).files, QStringList() << "kde4app.cpp" << "main.cpp" << "kde4appview.cpp" 
                                                      << CMAKE_TESTS_PROJECTS_DIR "/kde4app/ui_kde4appview_base.h" 
                                                      << CMAKE_TESTS_PROJECTS_DIR "/kde4app/ui_prefs_base.h" 
                                                      << CMAKE_TESTS_PROJECTS_DIR "/kde4app/settings.cpp" 
                                                      << CMAKE_TESTS_PROJECTS_DIR "/kde4app/settings.h" );
    QCOMPARE(v.targets.at( 1 ).name, QString("testkde4app") );
    
    QCOMPARE(v.targets.at( 1 ).files, QStringList() << "kde4app.cpp");
    QCOMPARE(v.targets.at( 2 ).name, QString("uninstall") );
    QCOMPARE(v.vm.value("CMAKE_INCLUDE_CURRENT_DIR"), QStringList("ON"));
}

void CMakeLoadProjectTest::testSmallProjectWithTests()
{
    CMakeProjectData v = parseProject(CMAKE_TESTS_PROJECTS_DIR "/unit_tests");
    QCOMPARE(v.testSuites.count(), 5);
    QCOMPARE(v.projectName, QString("unittests"));
    
    QCOMPARE(v.testSuites.at(0).files, QStringList() << "success.cpp");
    QCOMPARE(v.testSuites.at(0).name, QString("success"));
    QCOMPARE(v.testSuites.at(0).arguments.count(), 0);
    
    QCOMPARE(v.testSuites.at(3).files, QStringList() << "math_test.cpp");
    QCOMPARE(v.testSuites.at(3).name, QString("test_four"));
    QCOMPARE(v.testSuites.at(3).arguments.count(), 1);
    QCOMPARE(v.testSuites.at(3).arguments.at(0), QString("4"));
}

void CMakeLoadProjectTest::testKDE4ProjectWithTests()
{
    CMakeProjectData v = parseProject(CMAKE_TESTS_PROJECTS_DIR "/unit_tests_kde");
    QCOMPARE(v.testSuites.count(), 2); //cmake-test-unittestskde,unittestskde-nonstd-location
    QCOMPARE(v.projectName, QString("unittestskde"));
    
    QCOMPARE(v.testSuites.at(0).files, QStringList() << "test.cpp");
    QCOMPARE(v.testSuites.at(0).name, QString("cmake-test-unittestskde"));
    QCOMPARE(v.testSuites.at(0).arguments.count(), 0);
    QVERIFY(KUrl(v.testSuites.at(0).executable).isRelative());
}

CMakeProjectData CMakeLoadProjectTest::parseProject( const QString& sourcedir )
{
    QString projectfile = sourcedir+"/CMakeLists.txt";
    CMakeFileContent code=CMakeListsParser::readCMakeFile(projectfile);

    QPair<VariableMap,QStringList> initials = CMakeParserUtils::initialVariables();
    CMakeProjectData data;
    data.vm = initials.first;
    data.vm.insert("CMAKE_SOURCE_DIR", QStringList(sourcedir));
    
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
    v.walk(code, 0);
    
    data.projectName=v.projectName();
    data.subdirectories=v.subdirectories();
    data.definitions=v.definitions();
    data.includeDirectories=v.includeDirectories();
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

#include "cmakeloadprojecttest.moc"


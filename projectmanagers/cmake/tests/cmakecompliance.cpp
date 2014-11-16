/* KDevelop CMake Support
 *
 * Copyright 2007 Aleix Pol Gonzalez <aleixpol@gmail.com>
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

#include "cmakecompliance.h"
#include "cmakeast.h"
#include "cmakeprojectvisitor.h"
#include "astfactory.h"
#include "cmake-test-paths.h"
#include "../debug.h"

#include <KProcess>
#include <QFile>

#include <cmakeparserutils.h>
#include <language/duchain/duchain.h>
#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <cmakeprojectdata.h>

using namespace KDevelop;

QTEST_MAIN( CMakeCompliance )

QString CMakeCompliance::output;

//Copied from CMakeManager
QString executeProcess(const QString& execName, const QStringList& args=QStringList())
{
    KProcess p;
    p.setOutputChannelMode(KProcess::MergedChannels);
    p.setProgram(execName, args);
    p.start();

    if(!p.waitForFinished())
    {
        qCDebug(CMAKE) << "failed to execute:" << execName;
    }

    QByteArray b = p.readAllStandardOutput();
    QString t;
    t.prepend(b.trimmed());

    return t;
}

void CMakeCompliance::testEnumerate()
{
    QFETCH( QString, exe);

    QStringList commands=executeProcess(exe, QStringList("--help-command-list")).split("\n");
    commands.erase(commands.begin());
    commands.sort();
    foreach(const QString& cmd, commands)
    {
        if(cmd.toLower().startsWith("end") || cmd.toLower()=="else" || cmd.toLower()=="elseif")
            continue;
        CMakeAst* element = AstFactory::self()->createAst(cmd);
        if(!element)
            qDebug() << cmd << "is not supported";
        delete element;
    }
}

void CMakeCompliance::testEnumerate_data()
{
    QTest::addColumn<QString>( "exe" );
    QStringList cmakes;
    KStandardDirs::findAllExe(cmakes, "cmake");

    foreach(const QString& path, cmakes)
    {
        QTest::newRow( qPrintable(path) ) << (path);
    }
}

CMakeProjectVisitor CMakeCompliance::parseFile( const QString& sourcefile )
{
    CMakeProjectVisitor::setMessageCallback(CMakeCompliance::addOutput);
    QString projectfile = sourcefile;

    CMakeFileContent code=CMakeListsParser::readCMakeFile(projectfile);

    static QPair<VariableMap,QStringList> initials = CMakeParserUtils::initialVariables();
    CMakeProjectData data;
    data.vm = initials.first;
    QString sourcedir=sourcefile.left(sourcefile.lastIndexOf('/'));
    data.vm.insert("CMAKE_SOURCE_DIR", QStringList(sourcedir));

    KDevelop::ReferencedTopDUContext buildstrapContext=new TopDUContext(IndexedString("buildstrap"), RangeInRevision(0,0, 0,0));
    DUChain::self()->addDocumentChain(buildstrapContext);
    ReferencedTopDUContext ref=buildstrapContext;
    QStringList modulesPath = data.vm["CMAKE_MODULE_PATH"];

    foreach(const QString& script, initials.second)
    {
        ref = CMakeParserUtils::includeScript(CMakeProjectVisitor::findFile(script, modulesPath, QStringList()),
                                              ref, &data, sourcedir, QMap<QString,QString>());
    }

    data.vm.insert("CMAKE_CURRENT_BINARY_DIR", QStringList(sourcedir));
    data.vm.insert("CMAKE_CURRENT_LIST_FILE", QStringList(projectfile));
    data.vm.insert("CMAKE_CURRENT_SOURCE_DIR", QStringList(sourcedir));

    CMakeProjectVisitor v(projectfile, ref);
    v.setVariableMap(&data.vm);
    v.setMacroMap(&data.mm);
    v.setCacheValues(&data.cache);
    v.setModulePath(modulesPath);
    output.clear();
    v.walk(code, 0);

    ReferencedTopDUContext ctx=v.context();
    return v;
}

void CMakeCompliance::testCMakeTests()
{
    QFETCH(QString, exe);
    QFETCH(QString, file);

    CMakeProjectVisitor v = parseFile(file);

    QString ret=executeProcess(exe, QStringList("-P") << file);

    QStringList outputList = output.split('\n'), cmakeList=ret.split('\n');
    for(int i=0; i<outputList.size(); i++) {
        QCOMPARE(outputList[i], cmakeList[i]);
    }
}

void CMakeCompliance::testCMakeTests_data()
{
    QTest::addColumn<QString>("exe");
    QTest::addColumn<QString>("file");

    QStringList files=QStringList()
//         << "/CMakeTests/IfTest.cmake.in"
        << "/CMakeTests/ListTest.cmake.in"
    ;

    QStringList cmakes;
    KStandardDirs::findAllExe(cmakes, "cmake");
    foreach(const QString& exe, cmakes) {
        foreach(const QString& file, files)
            QTest::newRow( qPrintable(QString(exe+file)) ) << exe << QString(CMAKE_TESTS_PROJECTS_DIR + file);
    }
}

void CMakeCompliance::addOutput(const QString& msg)
{
    output += "-- "+msg+'\n';
}

CMakeCompliance::CMakeCompliance() {
    AutoTestShell::init();
    KDevelop::TestCore::initialize( Core::NoUi );
}

CMakeCompliance::~CMakeCompliance() {
    KDevelop::TestCore::shutdown();
}


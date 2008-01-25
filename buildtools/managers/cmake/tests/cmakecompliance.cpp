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

#include <KDebug>
#include <KProcess>
#include <QFile>

QTEST_MAIN( CMakeCompliance );

//Copied from CMakeManager
QString executeProcess(const QString& execName, const QStringList& args=QStringList())
{
    KProcess p;
    p.setOutputChannelMode(KProcess::MergedChannels);
    p.setProgram(execName, args);
    p.start();

    if(!p.waitForFinished())
    {
        kDebug(9032) << "failed to execute:" << execName;
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
    QStringList paths=CMakeProjectVisitor::envVarDirectories("PATH");
    
    foreach(const QString& path, paths)
    {
        if(QFile::exists(path+"/cmake"))
            QTest::newRow( qPrintable(path+"/cmake") ) << (path+"/cmake");
    }
}

#include "cmakecompliance.moc"

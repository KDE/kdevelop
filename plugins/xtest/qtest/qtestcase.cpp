/* KDevelop xUnit plugin
 *
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
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

#include "qtestcase.h"
#include "qtestsuite.h"
#include "qtestcommand.h"
#include "qtestoutputparser.h"
#include "qtestoutputmorpher.h"
#include "config/qtestsettings.h"

#include <QCoreApplication>
#include <QDir>
#include <QThread>
#include <QFile>
#include <QTextStream>
#include <QTimer>

#include <KDebug>
#include <KProcess>

using QTest::ISettings;
using QTest::QTestCase;
using QTest::QTestSuite;
using QTest::QTestCommand;
using QTest::QTestOutputParser;
using QTest::QTestOutputMorpher;

using Veritas::Test;

int QTestCase::s_count = 0;

QTestCase::QTestCase(const QString& name, const QFileInfo& exe, QTestSuite* parent)
        : Test(name, parent),
          m_exe(exe),
          m_output(0),
          m_proc(0),
          m_parser(0),
          m_timer(new QTimer(this))
{}

void QTestCase::removeFile(const QString& filePath)
{
    if (not filePath.isNull()) {
        QFile::remove(filePath);
    }
}

QTestCase::~QTestCase()
{
    removeFile(m_outputPath);
    removeFile(m_stdOutFilePath);
    removeFile(m_stdErrFilePath);
    removeFile(m_textOutFilePath);

    if (m_output) delete m_output;
    if (m_parser) delete m_parser;
}

bool QTestCase::shouldRun() const
{
    return true;
}

QFileInfo QTestCase::textOutFilePath() const
{
    return QFileInfo(m_textOutFilePath);
}

QFileInfo QTestCase::stdErrFilePath() const
{
    return QFileInfo(m_stdErrFilePath);
}

QFileInfo QTestCase::executable()
{
    QFileInfo exe(m_exe);
    Test* suite = parent();
    if (suite != 0 && qobject_cast<QTestSuite*>(suite) != 0) {
        QDir path = QDir(qobject_cast<QTestSuite*>(suite)->path().filePath());
        exe.setFile(path, m_exe.filePath());
    }
    return exe;
}

void QTestCase::setExecutable(const QFileInfo& exe)
{
    m_exe = exe;
}

QTestCommand* QTestCase::child(int i) const
{
    return static_cast<QTestCommand*>(Test::child(i));
}

void QTestCase::setSettings(ISettings* s)
{
    m_settings = s;
}

void QTestCase::setProcess(KProcess* proc)
{
    if (m_proc) delete m_proc;
    m_proc = proc;
}

void QTestCase::setOutputParser(QTestOutputParser* p)
{
    m_parser = p;
}

// execute the test and parse result back in.
int QTestCase::run()
{
    // preconditions
    assertProcessSet();
    assertOutputParserSet();

    initTempOutputFile();
    initProcArguments();
    setUpProcSignals();
    initOutputParser();
    executeProc();

    return 1;
}

// helper for run()
void QTestCase::setUpProcSignals()
{
    m_proc->disconnect();
//     connect(m_proc, SIGNAL(finished(int, QProcess::ExitStatus)),
//             this, SIGNAL(executionFinished()));
    connect(m_proc, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(stopTimer()));
}

void QTestCase::stopTimer()
{
    m_timer->stop();
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(morphXmlToText()));
    m_timer->start(100);
}

void QTestCase::morphXmlToText()
{
    QTestOutputMorpher m;
    QFile in(m_outputPath);
    in.open(QIODevice::ReadOnly);
    m.setSource(&in);
    QFile out(m_textOutFilePath);
    out.open(QIODevice::WriteOnly);
    out.write("~~~~~~~~~~~~~~~~~~~~~~~~~~ QTest ~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    m.setTarget(&out);
    m.xmlToText();
    emit executionFinished();
}

// helper for run()
void QTestCase::initOutputParser()
{
    m_parser->reset();
    m_parser->setCase(this);
    m_timer->disconnect();
    m_parser->disconnect();
    connect(m_timer, SIGNAL(timeout()),
            m_parser, SLOT(go()));
}

// helper for run()
void QTestCase::initTempOutputFile()
{
    QFile::remove(m_outputPath);
    QFile::remove(m_stdOutFilePath);
    QFile::remove(m_stdErrFilePath);
    QFile::remove(m_textOutFilePath);

    QString pathPrefix;
    QTextStream str(&pathPrefix);
    s_count += 1;
    str << QDir::tempPath() << QDir::separator() << "kdev-qtest-"
        << QCoreApplication::applicationPid() << "-" << s_count;
    m_outputPath = pathPrefix + ".tmp";
    m_stdOutFilePath = pathPrefix + "-out.tmp";
    m_stdErrFilePath = pathPrefix + "-err.tmp";
    m_textOutFilePath = pathPrefix + "-txt.tmp";
    m_output = new QFile(m_outputPath);

    QFile err(m_stdErrFilePath);
    err.open(QIODevice::ReadWrite);
    err.write("~~~~~~~~~~~~~~~~~~~~~~~~~~ StdandardErrror ~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    err.close();
}

// precondition for run()
void QTestCase::assertProcessSet()
{
    Q_ASSERT_X(m_proc, "QTestCase::run()",
               "Wrong usage. Client class should instantiate a "
               "KProcess and pass it with setProcess().");
}

// precondition for run()
void QTestCase::assertOutputParserSet()
{
    Q_ASSERT_X(m_parser, "QTestCase::run()",
               "Wrong usage. Client class should instantiate a "
               "QTestOutputParser and pass it with setParser().");
}

// helper for run()
void QTestCase::initProcArguments()
{
    m_proc->clearProgram();
    QStringList argv;
    argv << "-xml"
         << "-o" << m_outputPath;
    if (m_settings && m_settings->printAsserts()) {
        argv << "-v2";
    }
    if (m_settings && m_settings->printSignals()) {
        argv << "-vs";
    }
    m_proc->setProgram("./" + executable().fileName(), argv);
    m_proc->setOutputChannelMode(KProcess::SeparateChannels);
}

// helper for run()
void QTestCase::executeProc()
{
    QString dir = QDir::currentPath();
    QDir::setCurrent(executable().dir().absolutePath());
    kDebug() << "Executing " << m_proc->program() << " [ " << executable().filePath() << " ]";
    //m_proc->setStandardOutputFile(m_stdOutFilePath);
    m_proc->setStandardErrorFile(m_stdErrFilePath, QIODevice::Append);
    m_proc->start();
    QDir::setCurrent(dir);
    m_parser->setDevice(m_output);
    m_timer->start(100); // triggers QTestOutputParser evry 0.05 sec
}

#include "qtestcase.moc"

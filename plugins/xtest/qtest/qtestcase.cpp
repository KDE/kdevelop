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
#include <veritas/testresult.h>

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <KLocale>
#include <QTextStream>
#include <QTimer>
#include <KDebug>
#include <KProcess>

using QTest::QTestCase;
using QTest::QTestCommand;
using QTest::ISettings;
using Veritas::Test;
using Veritas::TestResult;

int QTestCase::s_count = 0;

QTestCase::QTestCase(const QString& name, const QFileInfo& exe, QTestSuite* parent)
        : Test(name, parent),
          m_exe(exe),
          m_output(0),
          m_proc(0),
          m_parser(0),
          m_timer(new QTimer(this)),
          m_parserTimeout(new QTimer(this))
{
    m_parserTimeout->setSingleShot(true);
    m_parserTimeout->setInterval(150);
    connect(m_parserTimeout, SIGNAL(timeout()), SLOT(closeOutputFile()));
}

void QTestCase::removeFile(const QString& filePath)
{
    if (not filePath.isNull()) {
        QFile::remove(filePath);
    }
}

void QTestCase::removeTempFiles()
{
    removeFile(m_outputPath);
    removeFile(m_stdOutFilePath);
    removeFile(m_stdErrFilePath);
    removeFile(m_textOutFilePath);
}

QTestCase::~QTestCase()
{
    removeTempFiles();
    if (m_output) delete m_output;
    if (m_parser) delete m_parser;
}

bool QTestCase::shouldRun() const
{
    return true;
}

KUrl QTestCase::outFile() const
{
    return KUrl(m_textOutFilePath);
}

KUrl QTestCase::errorFile() const
{
    return KUrl(m_stdErrFilePath);
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
    proc->setParent(this);
    m_proc = proc;
    connect(m_proc, SIGNAL(finished(int, QProcess::ExitStatus)),
            SLOT(morphXmlToText()));
    connect(m_proc, SIGNAL(error(QProcess::ProcessError)),
            SLOT(processError(QProcess::ProcessError)));
}

void QTestCase::processError(QProcess::ProcessError error)
{
    QString message;
    switch(error) {
        case QProcess::FailedToStart:
            message = i18n("Failed to start test executable.");
            break;
        case QProcess::Crashed:
            message = i18n("Test executable crashed.");
            break;
        case QProcess::Timedout:
            message = i18n("Test executable timed out.");
            break;
        case QProcess::WriteError:
            message = i18n("Failed to write to test executable.");
            break;
        case QProcess::ReadError:
            message = i18n("Failed to read from test executable.");
            break;
        case QProcess::UnknownError:
            message = i18n("Unkown error occured.");
            break;
    }
    signalStarted();
    TestResult* result = new TestResult;
    result->setMessage(message);
    result->setState(Veritas::RunFatal);
    result->setFile(KUrl(m_exe.filePath()));
    result->setLine(0);
    setResult(result);
    signalFinished();
    closeOutputFile();
}

void QTestCase::setOutputParser(QTestOutputParser* p)
{
    Q_ASSERT(!m_parser); Q_ASSERT(p);
    m_parser = p;
    m_parser->setParent(this);
    m_parser->reset();
    m_parser->setCase(this);
    connect(m_timer, SIGNAL(timeout()), m_parser, SLOT(go()));
    connect(m_parser, SIGNAL(done()), SLOT(closeOutputFile()));
}

// execute the test and parse result back in.
int QTestCase::run()
{
    assertProcessSet();
    assertOutputParserSet();

    initTempOutputFile();
    initProcArguments();
    m_finished = false;
    executeProc();

    return 1;
}

void QTestCase::closeOutputFile()
{
    if (m_finished) return;
    m_finished = true;
    emit executionFinished();
    if (m_timer) {
        m_timer->stop();
    }
    if (m_output) {
        m_output->close();
    }
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
    in.close();
    out.close();
    m_parserTimeout->start(); // triggers closeOutputFile()
}

bool QTestCase::fto_outputFileClosed()
{
    if (!m_output) return true;
    return !m_output->isOpen();
}

// helper for run()
void QTestCase::initTempOutputFile()
{
    removeTempFiles();

    QString pathPrefix;
    QTextStream str(&pathPrefix);
    s_count += 1;
    str << QDir::tempPath() << QDir::separator() << "kdev-qtest-"
        << QCoreApplication::applicationPid() << "-" << s_count;
    m_outputPath = pathPrefix + ".tmp";
    m_stdOutFilePath = pathPrefix + "-out.tmp";
    m_stdErrFilePath = pathPrefix + "-err.tmp";
    m_textOutFilePath = pathPrefix + "-txt.tmp";
    if (m_output) delete m_output;
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

namespace
{
void setLDLibDir(KProcess* qTestExe, ISettings* settings)
{
#ifdef Q_OS_LINUX
    if (!settings) return;
    KUrl cmakeLibDir = settings->cmakeProjectLibraryPath();
    if (cmakeLibDir.isEmpty()) return;
    QStringList env = QProcess::systemEnvironment();
    QString currentLDLib;
    foreach(const QString& str, env) {
        if (str.startsWith("LD_LIBRARY_PATH")) {
            QStringList spl = str.split('=');
            if (spl.size() == 2) currentLDLib = spl[1];
        }
    }
    QString newLDLib = cmakeLibDir.path();
    if (!currentLDLib.isEmpty()) newLDLib += ":" + currentLDLib;
    qTestExe->setEnv("LD_LIBRARY_PATH", newLDLib);
    kDebug() << newLDLib;
#endif
}

void setQTestOptions(ISettings* settings, QStringList& argv)
{
    if (!settings) return;
    if (settings->printAsserts()) argv << "-v2";
    if (settings->printSignals()) argv << "-vs";
}

}

// helper for run()
void QTestCase::initProcArguments()
{
    m_proc->clearProgram();
    QStringList argv;
    argv << "-xml"
         << "-o" << m_outputPath;
    setQTestOptions(m_settings, argv);
    setLDLibDir(m_proc, m_settings);
    m_proc->setProgram("./" + executable().fileName(), argv);
    m_proc->setOutputChannelMode(KProcess::SeparateChannels);
}

// helper for run()
void QTestCase::executeProc()
{
    QString dir = QDir::currentPath();
    QDir::setCurrent(executable().dir().absolutePath());
    kDebug() << "Executing " << m_proc->program() << " [ " << executable().filePath() << " ]";
    ///m_proc->setStandardOutputFile(m_stdOutFilePath);
    m_proc->setStandardErrorFile(m_stdErrFilePath, QIODevice::Append);
    //m_parser->setDevice(m_proc);
    //connect(m_proc, SIGNAL(readyRead()), m_parser, SLOT(go()));
    m_proc->start();
    QDir::setCurrent(dir);

    m_parser->setDevice(m_output);
    m_timer->start(50); // triggers QTestOutputParser evry 0.05 sec
}

#include "qtestcase.moc"

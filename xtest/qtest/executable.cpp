/*
* KDevelop xUnit integration
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

#include "executable.h"
#include <KProcess>
#include <KDebug>
#include <QFile>
#include <QDateTime>
#include <QTimer>
#include <QFileInfo>
#include <QDir>
#include <KLocale>
#include <QCoreApplication>

#include "qtestoutputparser.h"
#include "qtestmodelitems.h"
#include "qtestsettings.h"
#include "qtestoutputmorpher.h"
#include <veritas/testresult.h>
#include <veritas/test.h>

using QTest::Executable;
using QTest::OutputParser;
using QTest::OutputMorpher;
using QTest::Case;
using QTest::ISettings;
using Veritas::TestResult;
using Veritas::Test;

namespace
{

void setQTestOptions(ISettings* settings, QStringList& argv)
{
    if (!settings) return;
    if (settings->printAsserts()) argv << "-v2";
    if (settings->printSignals()) argv << "-vs";
}

}

int Executable::s_count = 0;

Executable::Executable()
    :     m_timer(new QTimer(this)),
          m_parserTimeout(new QTimer(this)),
          m_proc(0),
          m_output(0),
          m_parser(0),
          m_settings(0)
{
    m_parserTimeout->setSingleShot(true);
    m_parserTimeout->setInterval(150);
    connect(m_parserTimeout, SIGNAL(timeout()), SLOT(closeOutputFile()));
    m_proc = new KProcess(this);
    connect(m_proc, SIGNAL(finished(int, QProcess::ExitStatus)),
            SLOT(morphXmlToText()));
    connect(m_proc, SIGNAL(error(QProcess::ProcessError)),
            SLOT(processError(QProcess::ProcessError)));

}

Executable::~Executable()
{
    if (m_output) {
        m_output->close();
        delete m_output;
    }
    removeTempFiles();
}

void Executable::setLocation(const KUrl& url)
{
    m_location = url;
}

#define ASSERT_SANE_LOCATION \
    Q_ASSERT(!m_location.isEmpty()); \
    Q_ASSERT(m_location.isLocalFile());

QStringList Executable::fetchFunctions()
{
    ASSERT_SANE_LOCATION

    if (!QFile::exists(m_location.path())) {
        kError() << "Test executable " << m_location.path()
                 << " does not exist. Failed to retrieve test functions.";
        return QStringList();
    }

    KProcess proc;
    proc.setProgram(m_location.pathOrUrl(), QStringList() << "-functions");
    proc.setOutputChannelMode(KProcess::SeparateChannels);
    proc.start();
    proc.waitForFinished(-1);
    // this output is typically minimal, a handful of lines
    QByteArray output = proc.readAllStandardOutput();
    return QString(output).split('\n');
}

QString Executable::name() const
{
    ASSERT_SANE_LOCATION
    return m_location.fileName().split('.')[0];
}

KUrl Executable::location() const
{
    ASSERT_SANE_LOCATION
    return m_location;
}

bool Executable::wasModified() const
{
    return lastModified() != m_timestamp;
}
    
void Executable::updateTimestamp()
{
    m_timestamp = lastModified();
}

QDateTime Executable::lastModified() const
{
    return QFileInfo(m_location.fileName()).lastModified();
}

void Executable::setSettings(ISettings* s)
{
    Q_ASSERT(m_settings == 0); Q_ASSERT(s!=0);
    m_settings = s;
    QStringList argv;
}

void Executable::kill()
{
    if (!m_proc) return;
    m_proc->kill();
    closeOutputFile();
}

void Executable::processError(QProcess::ProcessError error)
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
            message = i18n("Unknown error occurred.");
            break;
    }
    m_currentCase->signalStarted();
    TestResult* result = new TestResult;
    result->setMessage(message);
    result->setState(Veritas::RunFatal);
    result->setFile(m_location);
    result->setLine(0);
    m_currentCase->setResult(result);
    m_currentCase->signalFinished();
    closeOutputFile();
}

void Executable::setOutputParser(OutputParser* p)
{
    Q_ASSERT(!m_parser); Q_ASSERT(p);
    m_parser = p;
    m_parser->setParent(this);
    m_parser->reset();
    m_parser->setCase(m_currentCase);
    connect(m_timer, SIGNAL(timeout()), m_parser, SLOT(go()));
    connect(m_parser, SIGNAL(done()), SLOT(closeOutputFile()));
}

// execute the test and parse result back in.
int Executable::run()
{
    assertProcessSet();
    assertOutputParserSet();
    
    initTempOutputFile();
    initProcArguments();
    m_finished = false;
    executeProc();

    return 1;
}

void Executable::setCase(Case* caze)
{
    m_currentCase = caze;
    setParent(caze);
}

void Executable::closeOutputFile()
{
    if (m_finished) return;
    m_finished = true;
    m_currentCase->emitExecutionFinished();
    if (m_timer) {
        m_timer->stop();
    }
    if (m_output) {
        m_output->close();
    }
}

void Executable::morphXmlToText()
{
    OutputMorpher m;
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

bool Executable::fto_outputFileClosed()
{
    if (!m_output) return true;
    return !m_output->isOpen();
}

// helper for run()
void Executable::initTempOutputFile()
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
    if (m_output) {
        m_output->close();
        delete m_output;
    }
    m_output = new QFile(m_outputPath);

    QFile err(m_stdErrFilePath);
    err.open(QIODevice::ReadWrite);
    err.write("~~~~~~~~~~~~~~~~~~~~~~~~~~ StdandardErrror ~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    err.close();
}

// precondition for run()
void Executable::assertProcessSet()
{
    Q_ASSERT_X(m_proc, "Executable::run()",
               "Wrong usage. Client class should instantiate a "
               "KProcess and pass it with setProcess().");
}

// precondition for run()
void Executable::assertOutputParserSet()
{
    Q_ASSERT_X(m_parser, "Executable::run()",
               "Wrong usage. Client class should instantiate a "
               "OutputParser and pass it with setParser().");
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
    if (!currentLDLib.isEmpty()) newLDLib += ':' + currentLDLib;
    qTestExe->setEnv("LD_LIBRARY_PATH", newLDLib);
    kDebug() << newLDLib;
#endif
}

}

// helper for run()
void Executable::initProcArguments()
{
    m_proc->clearProgram();
    QStringList argv;
    argv << "-xml"
         << "-o" << m_outputPath;
    setQTestOptions(m_settings, argv);
    setLDLibDir(m_proc, m_settings);
    m_proc->setProgram("./" + m_location.fileName(), argv);
    m_proc->setOutputChannelMode(KProcess::SeparateChannels);
}

// helper for run()
void Executable::executeProc()
{
    QString dir = QDir::currentPath();
    QDir::setCurrent(m_location.directory());
    kDebug() << "Executing " << m_proc->program() << " [ " << m_location.path() << " ]";
    ///m_proc->setStandardOutputFile(m_stdOutFilePath);
    m_proc->setStandardErrorFile(m_stdErrFilePath, QIODevice::Append);
    //m_parser->setDevice(m_proc);
    //connect(m_proc, SIGNAL(readyRead()), m_parser, SLOT(go()));
    m_proc->start();
    QDir::setCurrent(dir);

    m_parser->setDevice(m_output);
    m_timer->start(50); // triggers OutputParser evry 0.05 sec
}

void Executable::removeTempFiles()
{
    removeFile(m_outputPath);
    removeFile(m_stdOutFilePath);
    removeFile(m_stdErrFilePath);
    removeFile(m_textOutFilePath);
}

void Executable::removeFile(const QString& filePath)
{
    if (!filePath.isNull()) {
        QFile::remove(filePath);
    }
}

KUrl Executable::outFile() const
{
    return KUrl(m_textOutFilePath);
}

KUrl Executable::errorFile() const
{
    return KUrl(m_stdErrFilePath);
}


#include "executable.moc"

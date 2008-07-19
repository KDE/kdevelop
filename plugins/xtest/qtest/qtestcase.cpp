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
#include "config/qtestsettings.h"

#include <QDir>
#include <QThread>
#include <KDebug>
#include <KProcess>
#include <KTemporaryFile>

using QTest::ISettings;
using QTest::QTestCase;
using QTest::QTestSuite;
using QTest::QTestCommand;
using QTest::QTestOutputParser;

using Veritas::Test;

QTestCase::QTestCase(const QString& name, const QFileInfo& exe, QTestSuite* parent)
        : Test(name, parent), m_exe(exe), m_output(0), m_proc(0), m_parser(0)
{
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
}

QTestCase::~QTestCase()
{
    if (m_output) delete m_output;
    if (m_proc)   delete m_proc;
    if (m_parser) delete m_parser;
}

bool QTestCase::shouldRun() const
{
    return true;
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

// dependency injection
void QTestCase::setProcess(KProcess* proc)
{
    if (m_proc) delete m_proc;
    m_proc = proc;
}

// dependency injection
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

    if (not createTempOutputFile()) return -1;
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
    connect(m_proc, SIGNAL(readyRead()), m_parser, SLOT(go()));
    connect(m_proc, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SIGNAL(executionFinished()));
}

// void QTestCase::maybeParse()
// {
//     if (!m_timer->isActive()) {
//         m_timer->start(100);
//     }
// }


// helper for run()
void QTestCase::initOutputParser()
{
    m_parser->setDevice(m_proc);
    m_parser->reset();
    m_parser->setCase(this);
    m_timer->disconnect();
    connect(m_timer, SIGNAL(timeout()), m_parser, SLOT(go()));
}

// helper for run()
bool QTestCase::createTempOutputFile()
{
    bool isOk = true;
    if (m_output) delete m_output;
    m_output = new KTemporaryFile;
    m_output->open();
    if (!m_output->isOpen()) {
        kWarning() << "Failed to create temporary file for QTest output.";
        isOk = false;
    }
    m_output->close();
    return isOk;
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
    argv << "-xml";
    //<< QString("-o ") + m_output->fileName();
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
    m_proc->start();
    QDir::setCurrent(dir);
}

void QTestCase::setSettings(ISettings* s)
{
    m_settings = s;
}

#include "qtestcase.moc"

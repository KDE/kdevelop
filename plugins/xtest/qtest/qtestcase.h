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

#ifndef QXQTEST_QTESTCASE_H
#define QXQTEST_QTESTCASE_H

#include <QString>
#include <QFileInfo>

#include <veritas/test.h>

#include "qtestcommand.h"

class KTemporaryFile;
class KProcess;
class QTimer;

namespace QTest
{
class QTestSuite;
class QTestOutputParser;
class ISettings;

/*!
QTestLib test class item in the test tree. This will invoke the
QTestLib executable.
@unittest QTest::ut::QTestCaseTest
*/
class QTestCase : public Veritas::Test
{
Q_OBJECT
public:
    QTestCase(const QString&, const QFileInfo&, QTestSuite* parent=0);
    virtual ~QTestCase();

    QTestCommand* child(int i) const;
    QFileInfo executable();

    int run();
    bool shouldRun() const;

    /*! Client classes should instantiate a KProcess.
        QTestCase takes ownership.
        Sole purpose is to increase testability. */
    void setProcess(KProcess*);
    void setOutputParser(QTestOutputParser*);
    void setSettings(ISettings*);

    void initProcArguments();
    void setUpProcSignals();
    void setExecutable(const QFileInfo&);

    QFileInfo textOutFilePath() const;
    QFileInfo stdErrFilePath() const;

private:
    // preconditions for run()
    inline void assertProcessSet();
    inline void assertOutputParserSet();

    // helpers for run()
    void initTempOutputFile();
    void executeProc();
    void initOutputParser();
    void removeFile(const QString& filePath);

private slots:
    void stopTimer();
    void morphXmlToText();

private:
    QFileInfo m_exe;           // qtest-exe location
    ISettings* m_settings;     // Settings wrapper.

    QFile* m_output;           // temp file with qtest xml output.
    QString m_stdOutFilePath;  // unused.
    QString m_outputPath;      // path to temp file with xml output
    QString m_textOutFilePath; // path to temp file with ascii output
    QString m_stdErrFilePath;  // path to temp file with standard error

    KProcess* m_proc;          // this will execute the qtest-exe.
    QTestOutputParser* m_parser;
    QTimer* m_timer;
    static int s_count;        // used to get unique temp files.
};

} // end namespace QTest

#endif // QXQTEST_QTESTCASE_H

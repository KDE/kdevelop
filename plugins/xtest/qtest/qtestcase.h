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
#include <KUrl>
#include <QProcess>

#include <veritas/test.h>
#include "qxqtestexport.h"
#include "qtestcommand.h"

class KProcess;
class QTimer;

namespace QTest
{
class Suite;
class OutputParser;
class ISettings;

/*!
QTestLib test class item in the test tree. This will invoke the
QTestLib executable.
@unittest QTest::ut::CaseTest
*/
class QXQTEST_EXPORT Case : public Veritas::Test
{
Q_OBJECT
public:
    Case(const QString&, const QFileInfo&, Suite* parent=0);
    virtual ~Case();

    Command* child(int i) const;
    QFileInfo executable();

    virtual int run();
    virtual void kill();
    virtual bool shouldRun() const;

    /*! Client classes should instantiate a KProcess.
        Case takes ownership.
        Sole purpose is to increase testability. */
    void setProcess(KProcess*);
    void setOutputParser(OutputParser*);
    void setSettings(ISettings*);

    void initProcArguments();
    void setExecutable(const QFileInfo&);

    KUrl outFile() const;
    KUrl errorFile() const;

    /*! For testers only method that returns whether the output file was closed */
    bool fto_outputFileClosed();

private:
    void removeTempFiles();

    // preconditions for run()
    inline void assertProcessSet();
    inline void assertOutputParserSet();

    // helpers for run()
    void initTempOutputFile();
    void executeProc();
    void removeFile(const QString& filePath);

private slots:
    void morphXmlToText();
    void closeOutputFile();
    void processError(QProcess::ProcessError);

private:
    QFileInfo m_exe;           // qtest-exe location
    ISettings* m_settings;     // Settings wrapper.

    QFile* m_output;           // temp file with qtest xml output.
    QString m_stdOutFilePath;  // unused.
    QString m_outputPath;      // path to temp file with xml output
    QString m_textOutFilePath; // path to temp file with ascii output
    QString m_stdErrFilePath;  // path to temp file with standard error

    KProcess* m_proc;          // this will execute the qtest-exe.
    OutputParser* m_parser;
    QTimer* m_timer;
    static int s_count;        // used to get unique temp files.
    bool m_finished;
    QTimer* m_parserTimeout;
};

} // end namespace QTest

#endif // QXQTEST_QTESTCASE_H

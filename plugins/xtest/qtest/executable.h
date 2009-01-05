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

#ifndef VERITAS_QTEST_EXECUTABLE_H
#define VERITAS_QTEST_EXECUTABLE_H

#include <KUrl>
#include <QDateTime>
#include "qxqtestexport.h"
#include <QProcess>

class QFile;
class QTimer;
class KProcess;

namespace QTest
{

class OutputParser;
class ISettings;
class Case;

/*! Wraps a QTest executable. Currently only used to retrieve
test commands (aka functions) for a given testcase. */
class QXQTEST_EXPORT Executable : public QObject
{
Q_OBJECT
public:
    Executable();
    virtual ~Executable();

    /*! Initialize the filesystem location of this QTest
        @note mandatory to set this. */
    virtual void setLocation(const KUrl& url);
    virtual KUrl location() const;

    /*! Fetch the test functions aka testcommand names.
        Executes this with the -functions flag.
        Return the resulting lines as a list.
        @note blocking! */
    virtual QStringList fetchFunctions();

    /*! Deduce a test name from exe name */
    virtual QString name() const;

    /*! Returns true if the stored timestamp is older than this
     *  executable's last modification time. */
    virtual bool wasModified() const;
    virtual void updateTimestamp();
    
    void setOutputParser(OutputParser*);
    void setSettings(ISettings*);

    void kill();
    int run();

    bool fto_outputFileClosed();
    
    KUrl outFile() const;
    KUrl errorFile() const;

    void setCase(Case*);
    
private:
    void removeTempFiles();

    // preconditions for run()
    inline void assertProcessSet();
    inline void assertOutputParserSet();

    // helpers for run()
    void initTempOutputFile();
    void executeProc();
    void removeFile(const QString& filePath);
    void initProcArguments();
    
private slots:
    void morphXmlToText();
    void closeOutputFile();
    void processError(QProcess::ProcessError);

private:
    QDateTime lastModified() const;
    
    KUrl m_location;
    QDateTime m_timestamp;
    
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

    Case* m_currentCase;
};

}

#endif // VERITAS_QTEST_QTESTEXE_H

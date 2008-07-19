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
#include <QTimer>
#include <QFileInfo>

#include <kdevplatform/veritas/test.h>

#include "qtestcommand.h"

class KTemporaryFile;
class KProcess;

namespace QTest
{
class QTestSuite;
class QTestOutputParser;
class ISettings;

class QTestCase : public Veritas::Test
{
Q_OBJECT
public:
    QTestCase(const QString&, const QFileInfo&, QTestSuite*);
    virtual ~QTestCase();

    QTestCommand* child(int i) const;
    QFileInfo executable();

    int run();
    bool shouldRun() const;

    /*! Client classes should instantiate a KProcess.
        QTestCase takes ownership.
        Sole purpose is to increase testability through DI */
    void setProcess(KProcess*);
    void setOutputParser(QTestOutputParser*);
    void setSettings(ISettings*);
    void initProcArguments();
    void setUpProcSignals();
    void setExecutable(const QFileInfo&);

private:
    // preconditions for run()
    inline void assertProcessSet();
    inline void assertOutputParserSet();

    // helpers for run()
    bool createTempOutputFile();
    void executeProc();
    void initOutputParser();

// private slots:
//     void maybeParse();
//     void surelyParse();

private:
    QFileInfo m_exe;
    ISettings* m_settings;
    KTemporaryFile* m_output;
    KProcess* m_proc;
    QTestOutputParser* m_parser;
    QTimer* m_timer;
};

} // end namespace QTest

#endif // QXQTEST_QTESTCASE_H

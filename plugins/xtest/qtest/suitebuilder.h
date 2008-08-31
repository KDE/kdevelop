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

#ifndef VERITAS_QTEST_SUITEBUILDER_H
#define VERITAS_QTEST_SUITEBUILDER_H

#include <QMap>
#include <QFileInfoList>
#include <QObject>
#include "qxqtestexport.h"

namespace Veritas { class Test; }

class KUrl;
namespace QTest
{
class CaseBuilder;
class QTestSuite;
class ISettings;

/*!
Constructs QTestCases and deduces it's child QTestCommands for a set
of .shell executables. Also orders these testcases into suites based
on directory information. This is expensive so should not be run
in the GUI thread.

@unittest QTest::Test::SuiteBuilderTest
*/
class QXQTEST_EXPORT SuiteBuilder : public QObject
{
Q_OBJECT
public:
    SuiteBuilder();
    virtual ~SuiteBuilder();
    /*! QTest .shell executables */
    void setTestExecutables(const QList<KUrl>&);
    void setSettings(ISettings*);

    /*! Fetch the result, only to be called after the job completed */
    Veritas::Test* root() const;

    /*! Go */
    virtual void start();

signals:
    void progress(int min, int max, int value);

protected:
    /*! Lightweight factory method, which allows unit tests to inject
    Custom CaseBuilders */
    virtual CaseBuilder* createCaseBuilder(const KUrl& testShellLocation) const;

private:
    bool hasRun() const;
    void initRoot();
    void constructSuites();
    void constructCases();
    QString suiteNameForExe(const KUrl& testExe);
    QString suiteNameOneUp(const KUrl& suiteUrl);
    void addSuiteName(const KUrl& exeUrl);

private:
    QList<KUrl> m_testShellExes;
    Veritas::Test* m_root;
    bool m_hasRun;
    QMap<QString, QTestSuite*> m_suites;
    QMap<KUrl, QString> m_suiteNames;
    bool m_testExesSet;
    ISettings* m_settings;
};

}

#endif // VERITAS_QTEST_SUITEBUILDER_H

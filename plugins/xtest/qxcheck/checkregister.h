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

#ifndef QXCHECK_CHECKREGISTER
#define QXCHECK_CHECKREGISTER

#include <QXmlStreamReader>
#include <QList>
#include <QString>
#include <QFileInfo>

class QIODevice;
namespace QxRunner { class RunnerItem; }

namespace QxCheck
{

class TestBase;
class TestSuite;
class TestCase;

class CheckRegister : public QXmlStreamReader
{
public:
    CheckRegister();
    virtual ~CheckRegister();

    void addFromXml(QIODevice*);
    TestBase* rootItem();
    unsigned testSuiteCount();
    TestSuite* takeSuite(unsigned);
    void setRootDir(const QString& root);
    void setExecutable(const QFileInfo& exe) {
        m_exe = exe;
    }

private: // helpers
    bool isStartElement_(const QString& elem);
    bool isEndElement_(const QString& elem);
    void processSuite();
    void processCase(TestSuite* suite);
    void processCmd(TestCase* caze);
    QString   fetchName();
    QFileInfo fetchDir();
    QFileInfo fetchExe();

private: // state
    TestBase* m_rootItem;
    QList<TestSuite*> m_suites;
    QString m_root;
    QFileInfo m_exe;

private: // some xml constants
    static const QString c_suite;
    static const QString c_case;
    static const QString c_cmd;
    static const QString c_name;
};

} // end namespace QxQTest

#endif // QXCHECK_CHECKREGISTER

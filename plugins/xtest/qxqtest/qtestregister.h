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

#ifndef QXQTEST_QTESTREGISTER_H
#define QXQTEST_QTESTREGISTER_H

#include <QXmlStreamReader>
#include <QList>
#include <QString>
#include <QFileInfo>

class QIODevice;

namespace QxQTest
{

class QTestSuite;
class QTestCase;

class QTestRegister : public QXmlStreamReader
{
public:
    QTestRegister();
    virtual ~QTestRegister();

    void addFromXml(QIODevice*);
    unsigned testSuiteCount();
    QTestSuite* takeSuite(unsigned);

private: // helpers
    bool isStartElement_(const QString& elem);
    bool isEndElement_(const QString& elem);
    void processSuite();
    void processCase(QTestSuite* suite);
    void processCmd(QTestCase* caze);
    QString   fetchName();
    QFileInfo fetchDir();
    QFileInfo fetchExe();

private: // state
    QList<QTestSuite*> m_suites;
    QString m_root;

    // some xml constants
    static const QString c_suite;
    static const QString c_case;
    static const QString c_cmd;
    static const QString c_root;
    static const QString c_dir;
    static const QString c_name;
    static const QString c_exe;
};

} // end namespace QxQTest

#endif // QXQTEST_QTESTREGISTER_H

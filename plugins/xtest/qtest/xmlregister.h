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

#ifndef QXQTEST_XMLREGISTER_H
#define QXQTEST_XMLREGISTER_H

#include <QXmlStreamReader>
#include <QList>
#include <QString>
#include <QFileInfo>

#include "iregister.h"
#include "qxqtestexport.h"

class QIODevice;

namespace Veritas { class Test; }

namespace QTest
{

class ISettings;
class QTestSuite;
class QTestCase;
class IRegister;

/*! Reads a QTest suite's structure from XML. After parsing the root of the
generated test tree is available through the 'root()' member function. */
class QXQTEST_EXPORT XmlRegister : public IRegister, public QXmlStreamReader
{
public:
    XmlRegister();
    virtual ~XmlRegister();

    virtual void reload();
    /*! Caller should de-allocate this */
    virtual Veritas::Test* root() const;

    void setRootDir(const QString&);
    void setSource(QIODevice*);
    void setSettings(ISettings*); // TODO this is wrong

private: // helpers
    bool isStartElement_(const QString& elem);
    bool isEndElement_(const QString& elem);
    void processSuite();
    QTestCase* instantiateCase(QTestSuite* parent);
    void processCase(QTestCase* caze);
    void processCmd(QTestCase* caze);
    QString   fetchName();
    QFileInfo fetchDir();
    QFileInfo fetchExe();

private: // state
    Veritas::Test* m_rootItem;
    QString m_root;
    ISettings* m_settings;

private: // some xml constants
    static const QString c_suite;
    static const QString c_case;
    static const QString c_cmd;
    static const QString c_root;
    static const QString c_dir;
    static const QString c_name;
    static const QString c_exe;
};

} // end namespace QTest

#endif // QXQTEST_XMLREGISTER_H

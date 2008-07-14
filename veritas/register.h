/*
 * This file is part of KDevelop
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

#ifndef VERITAS_TESTREGISTER_H
#define VERITAS_TESTREGISTER_H

#include <QtCore/QString>
#include <QtCore/QFileInfo>
#include <QtXml/QXmlStreamReader>

#include "veritas/test.h"
#include "veritas/testcase.h"
#include "veritas/veritasexport.h"

class KProcess;
class QIODevice;

namespace Veritas
{

/*!
 * This class removes bad duplication between the Check
 * & CppUnit plugins. It might be completely irrelevant for
 * other frameworks.
 */
template<typename R, typename S>
class VERITAS_EXPORT Register : public QXmlStreamReader
{
public:
    typedef R TestRoot;
    typedef S TestSuite;

    Register();
    virtual ~Register();

    void addFromXml(QIODevice*);
    void addFromExe(const QFileInfo& exe);
    TestRoot* rootItem();
    void setRootDir(const QString& root);
    void setExecutable(const QFileInfo& exe);

private: // helpers
    void execute(KProcess*);
    bool isStartElement_(const QString& elem);
    bool isEndElement_(const QString& elem);
    void processSuite();
    void processCase(TestSuite* suite);
    void processCmd(Veritas::TestCase* caze);
    QString   fetchName();
    QFileInfo fetchDir();
    QFileInfo fetchExe();

private: // state
    TestRoot* m_rootItem;
    QString m_root;
    QFileInfo m_exe;

private: // some xml constants
    const QString c_suite;
    const QString c_case;
    const QString c_cmd;
    const QString c_name;
};

}

#include "register.hpp" // template implementation

#endif // VERITAS_REGISTER_H

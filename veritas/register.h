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

#include <QList>
#include <QString>
#include <QFileInfo>
#include <KProcess>
#include <KLocalizedString>
#include <KDebug>
#include <QIODevice>
#include <QXmlStreamReader>

#include <test.h>
#include <testcase.h>
#include <veritasexport.h>

namespace Veritas
{

/*!
 * This class removes bad duplication between the Check
 * & CppUnit plugins. It might be completly irrelevant for
 * other frameworks.
 */
template<typename S>
class VERITAS_EXPORT Register : public QXmlStreamReader
{
public:
    typedef S TestSuite;

    Register();
    virtual ~Register();

    void addFromXml(QIODevice*);
    void addFromExe(const QFileInfo& exe);
    Veritas::Test* rootItem();
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
    Veritas::Test* m_rootItem;
    QString m_root;
    QFileInfo m_exe;

private: // some xml constants
    const QString c_suite;
    const QString c_case;
    const QString c_cmd;
    const QString c_name;
};

#include "register.hpp"

}

#endif // VERITAS_REGISTER_H

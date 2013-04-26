/***************************************************************************
 *   Copyright 2008 Harald Fernengel <harry@kdevelop.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_AUTOTESTSHELL_H
#define KDEVPLATFORM_AUTOTESTSHELL_H

#include <QtCore/QStringList>

#include <shell/shellextension.h>
#include <shell/core.h>

/* This is a dummy shell for unit tests. It basically does nothing :)

   You can initialize it in initTestCase() to get a minimal shell to run
   your autotests.

   Example of a minimal KDevPlatform unit test:

   void Mytest::initTestCase()
   {
       AutoTestShell::init();
       TestCore::initialize();
   }

 */

namespace KDevelop
{

class AutoTestShell : public KDevelop::ShellExtension
{
public:
    QString xmlFile() { return QString(); }
    virtual QString binaryPath() { return QString(); };
    QString defaultProfile() { return "kdevtest"; }
    KDevelop::AreaParams defaultArea() {
        KDevelop::AreaParams params;
        params.name = "test";
        params.title = "Test";
        return params;
    }
    QString projectFileExtension() { return QString(); }
    QString projectFileDescription() { return QString(); }
    QStringList defaultPlugins() { return QStringList(); }

    static void init() { s_instance = new AutoTestShell; }
};

}

#endif


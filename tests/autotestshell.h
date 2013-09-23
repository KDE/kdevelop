/***************************************************************************
 *   Copyright 2008 Harald Fernengel <harry@kdevelop.org>                  *
 *   Copyright 2013 Milian Wolff <mail@milianw.de>                         *
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

#include "kdevplatformtestsexport.h"
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

class KDEVPLATFORMTESTS_EXPORT AutoTestShell : public KDevelop::ShellExtension
{
public:
    AutoTestShell(const QStringList& plugins)
        : m_plugins(plugins)
    {}
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
    QStringList defaultPlugins() { return m_plugins; }

    /**
     * Initialize the AutoTestShell and set the global instance.
     *
     * @p plugins A list of default global plugins which should be loaded.
     *            By default, all global plugins are loaded.
     */
    static void init(const QStringList& plugins = QStringList())
    {
        s_instance = new AutoTestShell(plugins);
    }

private:
    QStringList m_plugins;
};

}

#endif


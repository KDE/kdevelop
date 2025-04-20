/*
    SPDX-FileCopyrightText: 2008 Harald Fernengel <harry@kdevelop.org>
    SPDX-FileCopyrightText: 2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_AUTOTESTSHELL_H
#define KDEVPLATFORM_AUTOTESTSHELL_H

#include <QStringList>

#include "testsexport.h"
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

namespace KDevelop {
class KDEVPLATFORMTESTS_EXPORT AutoTestShell
    : public KDevelop::ShellExtension
{
public:
    ~AutoTestShell();

    QString xmlFile() override { return QString(); }
    QString executableFilePath() override { return QString(); };
    QString defaultProfile() { return QStringLiteral("kdevtest"); }
    KDevelop::AreaParams defaultArea() override
    {
        KDevelop::AreaParams params;
        params.name = QStringLiteral("test");
        params.title = QStringLiteral("Test");
        return params;
    }
    QString projectFileExtension() override { return QString(); }
    QString projectFileDescription() override { return QString(); }
    QStringList defaultPlugins() override { return m_plugins; }

    /**
     * Initialize the AutoTestShell and set the global instance.
     *
     * @p plugins A list of default global plugins which should be loaded.
     *            By default, all global plugins are loaded.
     */
    static void init(const QStringList& plugins = QStringList());

    /**
     * Install a @em .notifyrc file for a unit test.
     *
     * Call this function from the @c initTestCase() slot of a unit test class that may trigger
     * sending of a KDevelop notification - in order to prevent @em kf.notifications warnings.
     *
     * @warning This function may be called only from a test function that is
     *          invoked by the Qt Test framework and only after AutoTestShell::init().
     */
    static void initializeNotifications();

private:
    AutoTestShell() = default;
    QStringList m_plugins;
};
}

#endif

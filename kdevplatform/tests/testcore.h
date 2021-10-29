/*
    SPDX-FileCopyrightText: 2008 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_KDEVPLATFORMTESTCORE_H
#define KDEVPLATFORM_KDEVPLATFORMTESTCORE_H

#include "testsexport.h"
#include <shell/core.h>

namespace KDevelop {
class SessionController;
class RunController;
class PluginController;
class DocumentController;
class PartController;
class ProjectController;
class UiController;

/**
 * \class TestCore testcore.h
 *
 * This is an ICore implementation which should be used inside unit tests.
 * By default a temporary session named after currently running application
 * will be instantiated. Furthermore the mainwindow - if created - will be
 * hidden as no user interaction should take place in a unit test.
 *
 * The simplest code which should suffice for most use cases is:
 *
 * \code
 * MyUnitTest::initTestCase() {
 *   AutoTestShell::init()
 *   TestCore::initialize();
 * }
 * MyUnitTest::cleanupTestCase() {
 *   TestCore::shutdown();
 * }
 * \endcode
 *
 * This class also allows one to replace certain or all parts of the
 * shell library with custom implementation to make it possible
 * to write plugin tests more easily.
 *
 * The usage is then as follows:
 * \code
 * TestCore* core = new TestCore();
 * //replace plugin controller or similar
 * core->setPluginController( new MyCustomPluginController( core ) );
 * //initialize
 * core->initialize(Core::Default);
 * ... //test code
 * // if you want to cleanup manually:
 * core->cleanup();
 * delete core;
 * \endcode
 *
 * @note It is important to call initialize and cleanup, else the controllers
 * won't work properly.
 */
class KDEVPLATFORMTESTS_EXPORT TestCore
    : public Core
{
    Q_OBJECT

public:
    TestCore();
    /**
     * Initialize the test core with the given setup @p mode.
     * Optionally launch with a user given session.
     *
     * @p mode Use @c Core::NoUi when your unit test does not require any UI
     *         NOTE: Even in @c Default mode the mainwindow will be hidden,
     *               as unit tests should not depend on user interaction.
     * @p session By default a temporary session will be created called "test-%appname".
     *            If @p session is not empty, a non-temporary session with the given name
     *            will be opened.
     * @return the initialized test core
     */
    static TestCore* initialize(Core::Setup mode = Core::Default, const QString& session = {});

    /**
     * Calls @c cleanup() on the current TestCore instance,
     * then deletes the core.
     *
     * @NOTE: It is important to call this in e.g. @c cleanupTestSuite()
     *        otherwise the core will be leaked, no proper cleanup will take place
     *        and e.g. temporary sessions not properly deleted.
     */
    static void shutdown();

    /**
     * Fakes a shutdown without actually doing the shutdown.
     */
    void setShuttingDown(bool shuttingDown);

    void setSessionController(SessionController*);
    void setPluginController(PluginController*);
    void setRunController(RunController*);
    void setDocumentController(DocumentController*);
    void setPartController(PartController*);
    void setProjectController(ProjectController*);
    void setLanguageController(LanguageController*);
    void setUiController(UiController*);

private:
    void initializeNonStatic(Core::Setup mode, const QString& session);
};
}

#endif

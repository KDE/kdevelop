/***************************************************************************
 *   Copyright 2008 Andreas Pakulat <apaku@gmx.de>                         *
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

#ifndef KDEVPLATFORMTESTCORE_H
#define KDEVPLATFORMTESTCORE_H

#include "kdevplatformtestsexport.h"
#include <shell/core.h>

namespace KDevelop
{

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
 * will be instantiated.
 *
 * The simplest code which should suffice for most use cases is:
 *
 * \code
 * TestCore::initialize();
 * \endcode
 *
 * This class also allows to replace certain or all parts of the
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
class KDEVPLATFORMTESTS_EXPORT TestCore : public Core
{
    Q_OBJECT
public:
    TestCore();
    /**
     * Initialize the test core with the given setup @p mode.
     * Optionally launch with a user given session.
     *
     * @p mode Use Core::NoUi when your unit test does not require any UI
     * @p session By default a temporary session will be created called "test-%appname".
     *            If @p session is not empty, a non-temporary session with the given name
     *            will be opened.
     */
    static void initialize( Core::Setup mode = Core::Default, const QString& session = "" );

    void setSessionController( SessionController* );
    void setPluginController( PluginController* );
    void setRunController( RunController* );
    void setDocumentController( DocumentController* );
    void setPartController( PartController* );
    void setProjectController( ProjectController* );
    void setLanguageController( LanguageController* );
    void setUiController( UiController* );

private:
    void initializeNonStatic( Core::Setup mode, const QString& session );
};

}

#endif

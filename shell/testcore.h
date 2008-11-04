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

#include "testshellexport.h"
#include "core.h"

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
 * This class allows to replace certain or all parts of the
 * shell library with custom implementation to make it possible
 * to write plugin tests more easily.
 *
 * usage is as follows:
 * \code
 * TestCore* core = new TestCore();
 * //replace plugin controller
 * core->setPluginController( new MyCustomPluginController( core ) );
 * core->initialize();
 * ... //test code
 * core->cleanup();
 * delete core;
 * \endcode
 *
 * @note Its important to call initialize and cleanup, else the controller's
 * won't work properly.
 */
class KDEVPLATFORMTESTSHELL_EXPORT TestCore : public Core
{
    Q_OBJECT
public:
    TestCore();
    void initialize( Core::Setup );
    void setSessionController( SessionController* );
    void setPluginController( PluginController* );
    void setRunController( RunController* );
    void setDocumentController( DocumentController* );
    void setPartController( PartController* );
    void setProjectController( ProjectController* );
    void setLanguageController( LanguageController* );
    void setUiController( UiController* );
};

}

#endif

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

#include "scriptingmanager.h"
#include "scripttools.h"

#include "icore.h"

#include <kross/core/manager.h>

namespace KDevelop
{

Q_GLOBAL_STATIC(ScriptingManager, scriptingInstance)
Q_GLOBAL_STATIC(ScriptTools, scriptingTools)

ScriptingManager *ScriptingManager::self()
{
    return scriptingInstance();
}

ScriptingManager::ScriptingManager()
{
    Kross::Manager::self().addQObject(scriptingTools(), "KDevTools");
}

void ScriptingManager::registerCore(ICore *core)
{
    Kross::Manager::self().addQObject(core, "KDevCore");
}

}


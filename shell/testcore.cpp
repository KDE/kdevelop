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
#include "testcore.h"

#include "uicontroller.h"
#include "sessioncontroller.h"
#include "languagecontroller.h"
#include "runcontroller.h"
#include "documentcontroller.h"
#include "plugincontroller.h"
#include "partcontroller.h"
#include "projectcontroller.h"
#include "core_p.h"

namespace KDevelop
{

TestCore::TestCore()
 : Core( new CorePrivate(this) )
{
    KDevelop::Core::m_self = this;
}

void TestCore::initialize( Core::Setup mode )
{
    d->initialize( mode );
}

void TestCore::setSessionController( SessionController* ctrl )
{
    d->sessionController = ctrl;
}

void TestCore::setPluginController( PluginController* ctrl )
{
    d->pluginController = ctrl;
}

void TestCore::setRunController( RunController* ctrl )
{
    d->runController = ctrl;
}

void TestCore::setDocumentController( DocumentController* ctrl )
{
    d->documentController = ctrl;
}

void TestCore::setPartController( PartController* ctrl )
{
    d->partController = ctrl;
}

void TestCore::setProjectController( ProjectController* ctrl )
{
    d->projectController = ctrl;
}

void TestCore::setLanguageController( LanguageController* ctrl )
{
    d->languageController = ctrl;
}

void TestCore::setUiController( UiController* ctrl )
{
    d->uiController = ctrl;
}

}

#include "testcore.moc"

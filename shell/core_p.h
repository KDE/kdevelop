
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

#ifndef KDEVPLATFORM_PLATFORM_CORE_P_H
#define KDEVPLATFORM_PLATFORM_CORE_P_H

#include <kcomponentdata.h>
#include <core.h>
#include <QWeakPointer>

namespace KDevelop
{

class RunController;
class PartController;
class LanguageController;
class DocumentController;
class ProjectController;
class PluginController;
class UiController;
class SessionController;
class SourceFormatterController;
class ProgressManager;
class SelectionController;
class DocumentationController;
class DebugController;
class WorkingSetController;
class TestController;

class KDEVPLATFORMSHELL_EXPORT CorePrivate {
public:
    CorePrivate(Core *core);
    ~CorePrivate();
    bool initialize( Core::Setup mode, QString session );
    QWeakPointer<PluginController> pluginController;
    QWeakPointer<UiController> uiController;
    QWeakPointer<ProjectController> projectController;
    QWeakPointer<LanguageController> languageController;
    QWeakPointer<PartController> partController;
    QWeakPointer<DocumentController> documentController;
    QWeakPointer<RunController> runController;
    QWeakPointer<SessionController> sessionController;
    QWeakPointer<SourceFormatterController> sourceFormatterController;
    QWeakPointer<ProgressManager> progressController;
    QWeakPointer<SelectionController> selectionController;
    QWeakPointer<DocumentationController> documentationController;
    QWeakPointer<DebugController> debugController;
    QWeakPointer<WorkingSetController> workingSetController;
    QWeakPointer<TestController> testController;

    KComponentData m_componentData;
    Core *m_core;
    bool m_cleanedUp;
    bool m_shuttingDown;
    Core::Setup m_mode;
};

}

#endif

/***************************************************************************
 *   Copyright (C) 2007 by Alexander Dymo  <adymo@kdevelop.org>            *
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
#ifndef CORE_H
#define CORE_H

#include <kdevexport.h>
#include "icore.h"

namespace Sublime {
class Area;
}

namespace KDevelop {

class MainWindow;
class Config;
class PluginController;
class ProjectController;
class PartController;
class UiController;
class DocumentController;

class KDEVPLATFORM_EXPORT Core: public ICore {
public:
    static void initialize();
    static Core *self();

    virtual ~Core();

    /** @copydoc ICore::uiController() */
    virtual IUiController *uiController();

    /** @copydoc ICore::pluginController() */
    virtual IPluginController *pluginController();

    /** @copydoc ICore::projectController() */
    virtual IProjectController *projectController();

    /** @copydoc ICore::languageController() */
    virtual ILanguageController *languageController();

    virtual KParts::PartManager *partManager();

    virtual IDocumentController *documentController();

    /// @internal
    PartController *partController();
    /// @internal
    UiController *uiControllerInternal();
    /// @internal
    virtual DocumentController *documentControllerInternal();

    PluginController* pluginControllerInternal();

    void cleanup();

private:
    Core(QObject *parent = 0);
    static Core *m_self;

    struct CorePrivate *d;
};

}

#endif

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on

/* This file is part of KDevelop
Copyright 2007 Alexander Dymo <adymo@kdevelop.org>
Copyright 2007 Kris Wong <kris.p.wong@gmail.com>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/

#ifndef ICORE_H
#define ICORE_H

#include <QtCore/QObject>
#include "interfacesexport.h"


namespace KParts
{
class PartManager;
}

namespace KDevelop
{

class IUiController;
class IPluginController;
class IProjectController;
class ILanguageController;
class IDocumentController;
class IRunController;

/**
 * ICore is the container class for all the various objects in use by
 * KDevelop. If access is needed to a particular controller, then this class
 * should be used.
 *
 * ICore can provide the user with instances of the following things:
 * - the mainwindow
 * - the plugin controller
 * - the project controller
 * - the language controller
 * - the KPart manager
 * - the document controller
 *
 * When an object is provided to ICore so it can be used later, ICore
 * will take ownership of the object and upon application shutdown will take
 * responsibility for deleting the objects stored by ICore.
 */
class KDEVPLATFORMINTERFACES_EXPORT ICore: public QObject
{
    Q_OBJECT

public:
    virtual ~ICore();

    /** @return the static ICore instance */
    static ICore *self();

    /** @return ui controller */
    Q_SCRIPTABLE virtual KDevelop::IUiController *uiController() = 0;

    /** @return plugin controller */
    Q_SCRIPTABLE virtual KDevelop::IPluginController *pluginController() = 0;

    /** @return project controller */
    Q_SCRIPTABLE virtual KDevelop::IProjectController *projectController() = 0;

    /** @return language controller */
    Q_SCRIPTABLE virtual KDevelop::ILanguageController *languageController() = 0;

    /** @return part manager */
    Q_SCRIPTABLE virtual KParts::PartManager *partManager() = 0;

    /** @return document controller */
    Q_SCRIPTABLE virtual KDevelop::IDocumentController *documentController() = 0;

    /** @return run controller */
    Q_SCRIPTABLE virtual KDevelop::IRunController *runController() = 0;

protected:
    ICore(QObject *parent = 0);
    static ICore *m_self;
};

}

#endif

/* This file is part of KDevelop
Copyright (C) 2007 Alexander Dymo <adymo@kdevelop.org>

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

#ifndef KDEVCORE_H
#define KDEVCORE_H

#include <QObject>
#include "kdevexport.h"

class KMainWindow;

namespace KDevelop
{

class IUiController;
class IPluginController;
class IProjectController;
class ILanguageController;

/**
 * ICore is the container class for all the various objects in use by
 * KDevelop. If access is needed to a particular controller, then this class
 * should be used.
 *
 * ICore can provide the user with instances of the following things:
 * <li>the mainwindow
 * <li>the plugin controller
 *
 * When an object is provided to ICore so it can be used later, ICore
 * will take ownership of the object and upon application shutdown will take
 * responsibility for deleting the objects stored by ICore.
 */
class KDEVPLATFORM_EXPORT ICore: public QObject {
public:
    virtual ~ICore();

    /** @return ui controller.*/
    virtual IUiController *uiController() = 0;

    /** @return plugin controller.*/
    virtual IPluginController *pluginController() = 0;

    virtual IProjectController *projectController() = 0;

    virtual ILanguageController *languageController() = 0;

protected:
    ICore(QObject *parent = 0) { Q_UNUSED(parent) };

};

}

#endif

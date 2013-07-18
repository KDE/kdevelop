/*  This file is part of KDevelop
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

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

#ifndef KDEVPLATFORM_ITEMPLATEPROVIDER_H
#define KDEVPLATFORM_ITEMPLATEPROVIDER_H

#include <QtCore/QObject>
#include "interfacesexport.h"

class QIcon;
class QAbstractItemModel;
class QStringList;

namespace KDevelop
{

/**
 * @brief A provider of templates
 *
 * A template provider loads any kind of source code templates and presents them in a model
 * via the tempatesModel() function. This model will usually, but not necessarily, be connected
 * to a tree view, so a tree structure is recommended.
 *
 * If the templates have a similar structure as those used by the AppWizard plugin,
 * the TemplatesModel class may be used for convenience.
 *
 * The provider can also support downloading and uploading additional templates with
 * Get Hot New Stuff. If this is the case, return the name of the configuration file
 * (ending in .knsrc) from the knsConfigurationFile() function.
 *
 * If templates can be loaded from local files, the supportedMimeTypes() should return
 * all file types the provider can load. If loading is not supported, return an empty list.
 *
 * @sa TemplatesModel
 **/
class KDEVPLATFORMINTERFACES_EXPORT ITemplateProvider
{
public:
    /**
     * Destructor
     **/
    virtual ~ITemplateProvider();

    /**
     * @return The name of this provider.
     **/
    virtual QString name() const = 0;
    /**
     * @return An icon associated with this provider.
     **/
    virtual QIcon icon() const = 0;

    /**
     * @return A model containing all available templates.
     *
     * The called does not take ownership of the model.
     **/
    virtual QAbstractItemModel* templatesModel() = 0;

    /**
     * @return The configuration file for Get Hot New Stuff.
     *
     * If GHNS is not used by this provider, return an empty string.
     **/
    virtual QString knsConfigurationFile() const = 0;

    /**
     * @return Types of files this provider can load.
     *
     * If loading is not supported, return an empty list.
     **/
    virtual QStringList supportedMimeTypes() const = 0;

    /**
     * Load a template from the file @p fileName.
     *
     * This function will only be called if @c supportedMimeTypes() returns
     * a non-empty list.
     *
     * @param fileName the name of the file to load.
     **/
    virtual void loadTemplate(const QString& fileName) = 0;

    /**
     * Reloads all template data.
     *
     * This is usually called after loading or updating new templates.
     **/
    virtual void reload() = 0;
};

}

Q_DECLARE_INTERFACE( KDevelop::ITemplateProvider, "org.kdevelop.ITemplateProvider")

#endif // KDEVPLATFORM_ITEMPLATEPROVIDER_H

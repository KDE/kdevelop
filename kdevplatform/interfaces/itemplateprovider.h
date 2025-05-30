/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_ITEMPLATEPROVIDER_H
#define KDEVPLATFORM_ITEMPLATEPROVIDER_H

#include "interfacesexport.h"

#include <QObject>

#include <memory>

class QIcon;
class QString;

namespace KDevelop
{
class TemplatesModel;

/**
 * @brief A provider of templates
 *
 * A template provider loads any kind of source code templates and presents them in a model
 * via the tempatesModel() function. This model will usually, but not necessarily, be connected
 * to a tree view, so a tree structure is recommended.
 *
 * The provider can also support downloading and uploading additional templates with
 * Get Hot New Stuff. If this is the case, return the name of the configuration file
 * (ending in .knsrc) from the knsConfigurationFile() function.
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
     **/
    [[nodiscard]] virtual std::unique_ptr<TemplatesModel> createTemplatesModel() const = 0;

    /**
     * @return The configuration file for Get Hot New Stuff.
     *
     * If GHNS is not used by this provider, return an empty string.
     **/
    virtual QString knsConfigurationFile() const = 0;
};

}

Q_DECLARE_INTERFACE( KDevelop::ITemplateProvider, "org.kdevelop.ITemplateProvider")

#endif // KDEVPLATFORM_ITEMPLATEPROVIDER_H

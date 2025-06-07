/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_ITEMPLATEPROVIDER_H
#define KDEVPLATFORM_ITEMPLATEPROVIDER_H

#include "interfacesexport.h"

#include <QObject>

class QIcon;
class QString;

namespace KDevelop
{
/**
 * @brief A provider of templates
 *
 * A template provider specifies from where to load source code templates into
 * a TemplatesModel. This model will usually, but not necessarily, be connected
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
     * @return the type prefix to initialize a TemplatesModel with
     **/
    [[nodiscard]] virtual QString modelTypePrefix() const = 0;

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

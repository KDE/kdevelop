/* This file is part of KDevelop
 *
 * Copyright (C) 2012-2013 Miquel Sabaté <mikisabate@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#ifndef GHPROVIDERPLUGIN_H
#define GHPROVIDERPLUGIN_H


#include <interfaces/iplugin.h>
#include <interfaces/iprojectprovider.h>


namespace gh
{

/**
 * @class ProviderPlugin
 * This is the class that embeds the code of the Github provider into a plugin.
 */
class ProviderPlugin : public KDevelop::IPlugin, KDevelop::IProjectProvider
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IProjectProvider)

public:
    /// Constructor.
    ProviderPlugin(QObject *parent, const QList<QVariant> &args);

    virtual bool hasError() const;
    virtual QString errorDescription() const;

    /// Destructor.
    virtual ~ProviderPlugin();

    /// @returns the name of this plugin.
    virtual QString name() const;

    /// @returns the main widget for this plugin.
    virtual KDevelop::IProjectProviderWidget * providerWidget(QWidget *parent);
};

} // End of namespace gh


#endif // GHPROVIDERPLUGIN_H

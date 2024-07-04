/*
    SPDX-FileCopyrightText: 2012-2013 Miquel Sabaté <mikisabate@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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
    explicit ProviderPlugin(QObject* parent, const KPluginMetaData& metaData, const QList<QVariant>& args);

    /// Destructor.
    ~ProviderPlugin() override;

    /// @returns the name of this plugin.
    QString name() const override;

    /// @returns the main widget for this plugin.
    KDevelop::IProjectProviderWidget * providerWidget(QWidget *parent) override;
};

} // End of namespace gh


#endif // GHPROVIDERPLUGIN_H

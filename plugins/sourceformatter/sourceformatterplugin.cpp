/*
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "sourceformatterplugin.h"

#include "config/projectconfigpage.h"

#include <interfaces/icore.h>
#include <interfaces/isourceformattercontroller.h>
#include <project/projectconfigpage.h>

#include <KPluginFactory>

K_PLUGIN_FACTORY_WITH_JSON(SourceFormatterPluginFactory, "kdevsourceformatter.json", registerPlugin<SourceFormatterPlugin>();)

SourceFormatterPlugin::SourceFormatterPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList& args)
    : KDevelop::IPlugin(QStringLiteral("kdevsourceformatter"), parent, metaData)
{
    Q_UNUSED(args);

}

SourceFormatterPlugin::~SourceFormatterPlugin() = default;

int SourceFormatterPlugin::perProjectConfigPages() const
{
    const auto hasFormatters = KDevelop::ICore::self()->sourceFormatterController()->hasFormatters();
    return hasFormatters ? 1 : 0;
}

KDevelop::ConfigPage* SourceFormatterPlugin::perProjectConfigPage(int number, const KDevelop::ProjectConfigOptions& options, QWidget* parent)
{
    if (number != 0) {
        return nullptr;
    }

    return new SourceFormatter::ProjectConfigPage(this, options.project, parent);
}

// needed for QObject class created from K_PLUGIN_FACTORY_WITH_JSON
#include "sourceformatterplugin.moc"
#include "moc_sourceformatterplugin.cpp"

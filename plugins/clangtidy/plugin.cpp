/*
    SPDX-FileCopyrightText: 2016 Carlos Nihelton <carlosnsoliveira@gmail.com>
    SPDX-FileCopyrightText: 2018, 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "plugin.h"

// plugin
#include <clangtidyconfig.h>
#include <clangtidyprojectconfig.h>
#include "config/clangtidypreferences.h"
#include "config/clangtidyprojectconfigpage.h"
#include "analyzer.h"
#include "checksetselectionmanager.h"
// KDevPlatform
#include <interfaces/contextmenuextension.h>
#include <project/projectconfigpage.h>
#include <util/path.h>
// KF
#include <KLocalizedString>
#include <KPluginFactory>

using namespace KDevelop;

K_PLUGIN_FACTORY_WITH_JSON(ClangTidyFactory, "kdevclangtidy.json",
                           registerPlugin<ClangTidy::Plugin>();)

namespace ClangTidy
{

Plugin::Plugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList& /*unused*/)
    : IPlugin(QStringLiteral("kdevclangtidy"), parent, metaData)
    , m_checkSetSelectionManager(new CheckSetSelectionManager)
{
    setXMLFile(QStringLiteral("kdevclangtidy.rc"));

    // create after ui.rc file is set with action ids
    m_analyzer = new Analyzer(this, m_checkSetSelectionManager, this);

    auto clangTidyPath = KDevelop::Path(ClangTidySettings::clangtidyPath()).toLocalFile();

    // TODO: not only collect on plugin loading, but also on every change in the settings
    // TODO: should also check version on every job start to see if there was an update
    // behind our back while kdevelop running
    // TODO: collect in async job
    m_checkSet.setClangTidyPath(clangTidyPath);
}

Plugin::~Plugin() = default;

void Plugin::unload()
{
    delete m_checkSetSelectionManager;
    m_checkSetSelectionManager = nullptr;
    delete m_analyzer;
    m_analyzer = nullptr;
}

ContextMenuExtension Plugin::contextMenuExtension(Context* context, QWidget* parent)
{
    ContextMenuExtension extension = KDevelop::IPlugin::contextMenuExtension(context, parent);

    m_analyzer->fillContextMenuExtension(extension, context, parent);

    return extension;
}

KDevelop::ConfigPage* Plugin::perProjectConfigPage(int number, const ProjectConfigOptions& options, QWidget* parent)
{
    if (number != 0) {
        return nullptr;
    }

    // ensure checkset is up-to-date TODO: async
    auto clangTidyPath = KDevelop::Path(ClangTidySettings::clangtidyPath()).toLocalFile();
    m_checkSet.setClangTidyPath(clangTidyPath);

    return new ProjectConfigPage(this, options.project, m_checkSetSelectionManager, &m_checkSet, parent);
}

KDevelop::ConfigPage* Plugin::configPage(int number, QWidget* parent)
{
    if (number != 0) {
        return nullptr;
    }

    // ensure checkset is up-to-date TODO: async
    auto clangTidyPath = KDevelop::Path(ClangTidySettings::clangtidyPath()).toLocalFile();
    m_checkSet.setClangTidyPath(clangTidyPath);

    return new ClangTidyPreferences(m_checkSetSelectionManager, &m_checkSet, this, parent);
}

} // namespace ClangTidy

#include "plugin.moc"
#include "moc_plugin.cpp"

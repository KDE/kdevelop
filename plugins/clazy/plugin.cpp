/*
    SPDX-FileCopyrightText: 2018 Anton Anikin <anton@anikin.xyz>
    SPDX-FileCopyrightText: 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "plugin.h"

// plugin
#include "checksdb.h"
#include "globalsettings.h"
#include "analyzer.h"
#include "checksetselectionmanager.h"
#include "debug.h"
#include "config/globalconfigpage.h"
#include "config/projectconfigpage.h"
// KDevPlatform
#include <interfaces/contextmenuextension.h>
#include <project/projectconfigpage.h>
// KF
#include <KPluginFactory>
// Qt
#include <QApplication>


K_PLUGIN_FACTORY_WITH_JSON(ClazyFactory, "kdevclazy.json",
                           registerPlugin<Clazy::Plugin>();)

namespace Clazy
{

Plugin::Plugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList&)
    : IPlugin(QStringLiteral("kdevclazy"), parent, metaData)
    , m_db(nullptr)
    , m_checkSetSelectionManager(new CheckSetSelectionManager)
{
    setXMLFile(QStringLiteral("kdevclazy.rc"));

    // create after ui.rc file is set with action ids
    m_analyzer = new Analyzer(this, m_checkSetSelectionManager, this);
}

Plugin::~Plugin() = default;

void Plugin::unload()
{
    delete m_checkSetSelectionManager;
    m_checkSetSelectionManager = nullptr;
    delete m_analyzer;
    m_analyzer = nullptr;
}

KDevelop::ContextMenuExtension Plugin::contextMenuExtension(KDevelop::Context* context, QWidget* parent)
{
    KDevelop::ContextMenuExtension extension = KDevelop::IPlugin::contextMenuExtension(context, parent);

    m_analyzer->fillContextMenuExtension(extension, context, parent);

    return extension;
}

KDevelop::ConfigPage* Plugin::perProjectConfigPage(int number, const KDevelop::ProjectConfigOptions& options, QWidget* parent)
{
    if (m_db.isNull()) {
        reloadDB();
    }

    return number ? nullptr : new ProjectConfigPage(this, options.project, m_checkSetSelectionManager, parent);
}

KDevelop::ConfigPage* Plugin::configPage(int number, QWidget* parent)
{
    if (m_db.isNull()) {
        reloadDB();
    }

    return number ? nullptr : new GlobalConfigPage(m_checkSetSelectionManager, m_db, this, parent);
}

QSharedPointer<const ChecksDB> Plugin::checksDB() const
{
    return m_db;
}

QSharedPointer<const ChecksDB> Plugin::loadedChecksDB()
{
    if (m_db.isNull()) {
        reloadDB();
    }

    return m_db;
}

void Plugin::reloadDB()
{
    m_db.reset(new ChecksDB(GlobalSettings::docsPath()));
    connect(GlobalSettings::self(), &GlobalSettings::docsPathChanged, this, &Plugin::reloadDB);
}

}

#include "plugin.moc"
#include "moc_plugin.cpp"

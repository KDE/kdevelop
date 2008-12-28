/*
 * This file is part of KDevelop
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "configmodule.h"

#include <KPluginFactory>
#include <KPluginInfo>
#include <KPluginLoader>
#include <KDebug>
#include <QVBoxLayout>

#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iproject.h>
#include <veritas/itestframework.h>

#include "configwidget.h"
#include <interfaces/iprojectcontroller.h>

using KDevelop::ICore;
using KDevelop::IPlugin;
using KDevelop::IPluginController;
using KDevelop::ProjectConfigSkeleton;
using KDevelop::IProject;
using Veritas::ConfigModule;
using Veritas::ITestFramework;

K_PLUGIN_FACTORY(VeritasConfigFactory, registerPlugin<Veritas::ConfigModule>(); )
K_EXPORT_PLUGIN(VeritasConfigFactory("kcm_kdev_veritassettings"))

namespace
{
QList<ITestFramework*> fetchTestFrameworks()
{
    QList<ITestFramework*> frameworks;
    ICore* core = ICore::self();
    IPluginController* pc = core->pluginController();
    QList<IPlugin*> testPlugins = pc->allPluginsForExtension("org.kdevelop.ITestFramework");

    foreach(IPlugin* plugin, testPlugins) {
        ITestFramework* tf = qobject_cast<ITestFramework*>(plugin);
        Q_ASSERT(tf);
        frameworks << tf;
    }
    return frameworks;
}

/*! Retrieves the project for the current configmodule.*/
IProject* retrieveProject()
{
    KUrl projectUrl = VeritasConfig::self()->projectFileUrl();
    foreach(IProject* proj, ICore::self()->projectController()->projects()) {
        if (proj->projectFileUrl() == projectUrl) {
            return proj;
        }
    }
    Q_ASSERT(0 && "Errr, project kcmodule without associated project."); 
    return 0;
}

bool isCMakeProject()
{
    IProject* proj = retrieveProject();
    KConfigGroup cfg = proj->projectConfiguration()->group( "Project" );
    return cfg.readEntry("Manager") == "KDevCMakeManager";
}

} // end anonymous namespace

ConfigModule::ConfigModule(QWidget* parent, const QVariantList& args)
    : ProjectKCModule<VeritasConfig>(VeritasConfigFactory::componentData(), parent, args)
{
    QVBoxLayout* l = new QVBoxLayout(this);
    m_widget = new ConfigWidget;
    if (isCMakeProject()) m_widget->setReadOnly();
    l->addWidget(m_widget);

    QList<ITestFramework*> frameworks = fetchTestFrameworks();
    initWidgetFrameworkSelectionBox(frameworks);
    initFrameworkSpecificConfigs(frameworks, args);

    connect(m_widget, SIGNAL(frameworkSelected(QString)), SLOT(setDetailsWidgetFor(QString)));
    connect(m_widget, SIGNAL(frameworkSelected(QString)), SLOT(changed()));
    connect(m_widget, SIGNAL(changed()), SLOT(changed()));
    addConfig(VeritasConfig::self(), this);

    load();
}

ConfigModule::~ConfigModule()
{
}

void ConfigModule::save()
{
    VeritasConfig::setExecutables(m_widget->executables());
    VeritasConfig::setFramework(m_widget->currentFramework());
    VeritasConfig::self()->writeConfig();
    ProjectKCModule<VeritasConfig>::save();
}

void ConfigModule::load()
{
    KUrl::List currentTestExes = m_widget->executables();
    foreach(const KUrl& testExe, VeritasConfig::executables()) {
        if (!currentTestExes.contains(testExe)) {
            m_widget->addTestExecutableField(testExe);
        }
    }
    if (m_widget->numberOfTestExecutableFields() == 0) {
        m_widget->addTestExecutableField();
    }
    ProjectKCModule<VeritasConfig>::load();
    m_widget->setCurrentFramework(VeritasConfig::framework());
    setDetailsWidgetFor(VeritasConfig::framework());
}

void ConfigModule::initWidgetFrameworkSelectionBox(const QList<ITestFramework*>& frameworks)
{
    foreach(ITestFramework* tf, frameworks) {
        m_widget->appendFramework(tf->name());
    }
}

void ConfigModule::initFrameworkSpecificConfigs(const QList<ITestFramework*>& frameworks, const QVariantList& args)
{
    foreach(ITestFramework* tf, frameworks) {
        ProjectConfigSkeleton* pcs = tf->configSkeleton(args);
        if (!pcs) continue;
        QWidget* w = tf->createConfigWidget();
        Q_ASSERT(w); // when a framework has an additional configSkeleton
                     // it should also return a valid widget
        addConfig(pcs, w);
        m_specificWidgetFor[tf->name()] = w;
    }
}

void ConfigModule::setDetailsWidgetFor(const QString& frameworkName)
{
    if (m_specificWidgetFor.contains(frameworkName)) {
        m_widget->setDetailsWidget(m_specificWidgetFor[frameworkName]);
    } else {
        m_widget->setDetailsWidget(0);
        kDebug() << "No framework specific config widget registered for" << frameworkName;
    }
}

#include "configmodule.moc"

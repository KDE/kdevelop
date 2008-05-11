/* KDevelop xUnit plugin
 *
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

#include "xtestplugin.h"

#include <kpluginfactory.h>
#include <kpluginloader.h>

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>

#include "sample.h"

K_PLUGIN_FACTORY(KDevXtestFactory, registerPlugin<KDevXtestPlugin>();)
K_EXPORT_PLUGIN(KDevXtestFactory("kdevxtest"))

class KDevXtestPluginFactory: public KDevelop::IToolViewFactory
{
public:
    KDevXtestPluginFactory(KDevXtestPlugin *plugin): m_plugin(plugin) {}

    virtual QWidget* create(QWidget *parent = 0) {
        return sample::testRunnerWidget();
    }

    virtual Qt::DockWidgetArea defaultPosition() {
        return Qt::LeftDockWidgetArea;
    }

    virtual QString id() const {
        return "org.kdevelop.XtestView";
    }

private:
    KDevXtestPlugin *m_plugin;
};

KDevXtestPlugin::KDevXtestPlugin(QObject* parent, const QVariantList &)
        : KDevelop::IPlugin(KDevXtestFactory::componentData(), parent)
{
    m_xtestFactory = new KDevXtestPluginFactory(this);
    core()->uiController()->addToolView("xTest", m_xtestFactory);
    setXMLFile( "kdevxtest.rc" );
}

KDevXtestPlugin::~KDevXtestPlugin()
{
}

Qt::DockWidgetArea KDevXtestPlugin::dockWidgetAreaHint() const 
{
    return Qt::LeftDockWidgetArea;
}

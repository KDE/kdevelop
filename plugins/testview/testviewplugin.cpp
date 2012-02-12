/*  This file is part of KDevelop
    Copyright 2012 Miha ?an?ula <miha@noughmad.eu>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; see the file COPYING.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "testviewplugin.h"
#include "testview.h"

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/itestsuite.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/itestcontroller.h>
#include <interfaces/ilaunchconfiguration.h>
#include <interfaces/launchconfigurationtype.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/ilauncher.h>

#include <KPluginFactory>
#include <KAboutData>
#include <KLocalizedString>
#include <KAction>
#include <KActionCollection>
#include <KActionCollection>

K_PLUGIN_FACTORY(TestViewFactory, registerPlugin<TestViewPlugin>(); )
K_EXPORT_PLUGIN(TestViewFactory(KAboutData("kdevtestview","kdevtestview", ki18n("Unit Test View"), "0.1", ki18n("Lets you see and run unit tests"), KAboutData::License_GPL)))

using namespace KDevelop;

class TestToolViewFactory: public KDevelop::IToolViewFactory
{
    public:
        TestToolViewFactory( TestViewPlugin *plugin ): mplugin( plugin )
        {}
        virtual QWidget* create( QWidget *parent = 0 )
        {
            return new TestView( mplugin, parent );
        }
        virtual Qt::DockWidgetArea defaultPosition()
        {
            return Qt::LeftDockWidgetArea;
        }
        virtual QString id() const
        {
            return "org.kdevelop.TestView";
        }
        virtual QList< QAction* > contextMenuActions(QWidget* viewWidget) const
        {
            return viewWidget->actions();
        }
    private:
        TestViewPlugin *mplugin;
};

TestViewPlugin::TestViewPlugin(QObject* parent, const QVariantList& args): IPlugin(TestViewFactory::componentData(), parent)
{
    Q_UNUSED(args)
    
    KAction* runAll = new KAction( KIcon("system-run"), i18n("Run all"), this );
    connect(runAll, SIGNAL(triggered(bool)), SLOT(runAllTests()));
    actionCollection()->addAction("run_all_tests", runAll);
    
    setXMLFile("kdevctestmanager.rc");
    
    m_viewFactory = new TestToolViewFactory(this);
    core()->uiController()->addToolView(i18n("Unit Tests"), m_viewFactory);
}

TestViewPlugin::~TestViewPlugin()
{

}

void TestViewPlugin::unload()
{
    core()->uiController()->removeToolView(m_viewFactory);
}

void TestViewPlugin::runAllTests()
{
    foreach (ITestSuite* suite, core()->pluginController()->pluginForExtension("org.kdevelop.ITestController")->extension<ITestController>()->testSuites())
    {
        ILaunchConfiguration* config = suite->launchAllCases();
        ILauncher* launcher = 0;
        foreach (ILauncher* l, config->type()->launchers())
        {
            if (l->supportedModes().contains("test"));
            {
                launcher = l;
                break;
            }
        }
        KJob* job = launcher->start("test", config);
        core()->runController()->registerJob(job);
    }
}




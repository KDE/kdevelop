/* This file is part of KDevelop
 *  Copyright 2013 Christoph Thielecke <crissi99@gmx.de>

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

#include <unistd.h>

#include <QRegExp>
#include <QFile>
#include <QTreeView>
#include <QXmlInputSource>
#include <QXmlSimpleReader>
#include <QDomElement>
#include <QApplication>

#include <kiconloader.h>
#include <klocale.h>
#include <kaction.h>
#include <kprocess.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include <kicon.h>
#include <kactioncollection.h>
#include <kcmultidialog.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kaboutdata.h>

#include <execute/iexecuteplugin.h>

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/launchconfigurationtype.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/idebugcontroller.h>
#include <util/executecompositejob.h>

#include "plugin.h"
#include "marks.h"
#include "cppcheckmodel.h"
#include "job.h"
#include "widget.h"

#include <KXMLGUIFactory>


using namespace KDevelop;

K_PLUGIN_FACTORY(CppcheckFactory, registerPlugin<cppcheck::Plugin>();)
K_EXPORT_PLUGIN(CppcheckFactory(KAboutData("kdevcppcheck", "kdevcppcheck", ki18n("Cppcheck"), "0.1", ki18n("Support for running Cppcheck"), KAboutData::License_GPL)))

namespace cppcheck
{

WidgetFactory::WidgetFactory(cppcheck::Plugin* plugin)
        : m_plugin(plugin)
{
}

QWidget* WidgetFactory::create(QWidget *parent)
{
        return new cppcheck::Widget(m_plugin, parent);
}

Qt::DockWidgetArea WidgetFactory::defaultPosition()
{
    return Qt::BottomDockWidgetArea;
}

QString WidgetFactory::id() const
{
    return "org.kdevelop.CppcheckView";
}


Plugin::Plugin(QObject *parent, const QVariantList&)
    : IPlugin(CppcheckFactory::componentData(), parent)
    , m_factory(new cppcheck::WidgetFactory(this))
    , m_marks(new cppcheck::Marks(this))
{

    kDebug() << "setting cppcheck rc file";
    setXMLFile("kdevcppcheck.rc");

    core()->uiController()->addToolView(i18n("Cppcheck"), m_factory);
    KAction* act_checkfile;
    act_checkfile = actionCollection()->addAction("cppcheck_file", this, SLOT(runCppcheckFile()));
    act_checkfile->setStatusTip(i18n("Launches Cppcheck for current file"));
    act_checkfile->setText(i18n("Cppcheck"));

    KAction* act_check_all_files;
    act_check_all_files = actionCollection()->addAction("cppcheck_all", this, SLOT(runCppcheckAll()));
    act_check_all_files->setStatusTip(i18n("Launches Cppcheck for all files"));
    act_check_all_files->setText(i18n("Cppcheck (all Files)"));

    IExecutePlugin* iface = KDevelop::ICore::self()->pluginController()->pluginForExtension("org.kdevelop.IExecutePlugin")->extension<IExecutePlugin>();
    Q_ASSERT(iface);
}

void Plugin::unload()
{
    core()->uiController()->removeToolView(m_factory);
}

void Plugin::incomingModel(cppcheck::Model *model)
{
    emit newModel(model);
}

Plugin::~Plugin()
{
}

void Plugin::runCppcheck(bool allFiles)
{
    QList<KJob*> l;
    l << new cppcheck::Job(this, allFiles, KDevelop::ICore::self()->runController());
    KDevelop::ExecuteCompositeJob *ej = new KDevelop::ExecuteCompositeJob(KDevelop::ICore::self()->runController(), l);
    connect (l.first(), SIGNAL(jobFinished()), this, SLOT(result()));
    ej->start();
}

void Plugin::runCppcheckFile()
{
    bool allFiles = false;
    runCppcheck(allFiles);
}

void Plugin::runCppcheckAll()
{
   bool allFiles = true;
   runCppcheck(allFiles);
}
 

void Plugin::loadOutput()
{
}

void Plugin::result()
{
    
    QWidget *toolview_widget = KDevelop::ICore::self()->uiController()->findToolView(i18n("Cppcheck"), m_factory);
    KDevelop::ICore::self()->uiController()->raiseToolView(toolview_widget);
}
}
#include "plugin.moc"

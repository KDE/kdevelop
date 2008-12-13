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

#include "coverageplugin.h"
#include "reportwidget.h"

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>

#include <QAbstractItemView>
#include <QDir>
#include <KAction>
#include <KAboutData>
#include <KActionCollection>
#include <KActionMenu>
#include <KLocale>
#include <KPluginFactory>
#include <KPluginLoader>
#include <QDir>

using KDevelop::ICore;
using KDevelop::IUiController;

using Veritas::CoveragePlugin;
using Veritas::ReportViewFactory;
using Veritas::ReportWidget;

K_PLUGIN_FACTORY(CoveragePluginFactory, registerPlugin<CoveragePlugin>();)
K_EXPORT_PLUGIN(CoveragePluginFactory(KAboutData("kdevcoverage","kdevcoverage", ki18n("Coverage Support"), "0.1", ki18n("Support for running coverage tools"), KAboutData::License_GPL)))

//////////////////////////// CoveragePlugin ///////////////////////////////

CoveragePlugin::CoveragePlugin(QObject* parent, const QVariantList&)
        : KDevelop::IPlugin( CoveragePluginFactory::componentData(), parent ),
          m_factory(new ReportViewFactory())
{
    setXMLFile("kdevcoverage.rc");
    core()->uiController()->addToolView(i18n("Coverage"), m_factory);
}

CoveragePlugin::~CoveragePlugin()
{
}

QFileInfoList CoveragePlugin::findGcdaFilesIn(QDir& dir)
{
    QFileInfoList gcdaFiles;
    QDir current(dir);
    current.setFilter(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable | QDir::Writable);
    const QStringList subDirs = current.entryList();
    foreach(const QString& subDir, subDirs) {
        current.cd(subDir);
        gcdaFiles += findGcdaFilesIn(current);
        current.cdUp();
    }
    current = QDir(dir);
    current.setNameFilters(QStringList() << "*.gcda");
    current.setFilter(QDir::Files |  QDir::Writable | QDir::NoSymLinks);
    return current.entryInfoList() + gcdaFiles;
}

//////////////////////////// ReportViewFactory ///////////////////////////////

ReportViewFactory::ReportViewFactory()
{
}

ReportViewFactory::~ReportViewFactory()
{
}

QWidget* ReportViewFactory::create(QWidget *parent)
{
    ReportWidget* w = new ReportWidget(parent);
    w->init();
    return w;
}

Qt::DockWidgetArea ReportViewFactory::defaultPosition()
{
    return Qt::RightDockWidgetArea;
}

QString ReportViewFactory::id() const
{
    return "org.kdevelop.CoverageReport";
}

#include "coverageplugin.moc"

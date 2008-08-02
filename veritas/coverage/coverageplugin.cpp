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

#include <KAction>
#include <KActionCollection>
#include <KDebug>
#include <KLocale>
#include <KPluginFactory>
#include <KPluginLoader>

#include <outputview/covoutputdelegate.h>
#include <outputview/covoutputjob.h>

#include <interfaces/icore.h>
#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <project/projectmodel.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/interfaces/iprojectfilemanager.h>


using KDevelop::Context;
using KDevelop::ContextMenuExtension;
using KDevelop::ProjectBaseItem;
using KDevelop::ProjectFolderItem;
using KDevelop::ProjectItemContext;
using KDevelop::ICore;
using KDevelop::IBuildSystemManager;
using KDevelop::IProject;
using KDevelop::IProjectFileManager;


using Veritas::CoveragePlugin;
using Veritas::CovOutputJob;
using Veritas::CovOutputDelegate;

K_PLUGIN_FACTORY(CoveragePluginFactory, registerPlugin<CoveragePlugin>();)
K_EXPORT_PLUGIN(CoveragePluginFactory("kdevcoverage"))

CoveragePlugin::CoveragePlugin(QObject* parent, const QVariantList&)
        : KDevelop::IPlugin( CoveragePluginFactory::componentData(), parent ),
          m_delegate(new CovOutputDelegate(this))
{
    setXMLFile( "kdevcoverage.rc" );

    KAction *action = actionCollection()->addAction("spawn_coverage");
    action->setText(i18n("Spawn Coverage Outputview"));
    action->setShortcut( QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_B) );
    connect(action, SIGNAL(triggered(bool)), this, SLOT(startCovOutputJob()));
}

CoveragePlugin::~CoveragePlugin()
{}

void CoveragePlugin::startCovOutputJob()
{
    CovOutputJob* job = new CovOutputJob(m_delegate, m_buildRoot);
    ICore::self()->runController()->registerJob(job);
}

ContextMenuExtension CoveragePlugin::contextMenuExtension(Context* context)
{
    ContextMenuExtension cm;
    if (context->type() != Context::ProjectItemContext) {
        return cm; // NO-OP
    }
    ProjectItemContext *pc = dynamic_cast<ProjectItemContext*>(context);
    if (!pc) {
        kWarning() << "Context::ProjectItemContext but cast failed. Not good.";
        return cm;
    }
    QList<ProjectBaseItem*> bl = pc->items();
    if (!bl.count()) {
        return cm;
    }
    IProject* proj = bl[0]->project();
    IBuildSystemManager* bm = proj->buildSystemManager();
    m_buildRoot = bm->buildDirectory(bl[0]);

    KAction *action = new KAction(i18n("Coverage Report"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(startCovOutputJob()));
    cm.addAction(ContextMenuExtension::ExtensionGroup, action);
    return cm;
}

#include "coverageplugin.moc"

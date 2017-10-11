/* This file is part of KDevelop
    Copyright 2017 Aleix Pol Gonzalez <aleixpol@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "mesonmanager.h"
#include "mesonbuilder.h"
#include "mesonconfig.h"
#include "mesonimportjob.h"
#include <interfaces/iproject.h>
#include <project/projectmodel.h>
#include <util/executecompositejob.h>

#include <QFileDialog>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>

#include "debug.h"

using namespace KDevelop;

K_PLUGIN_FACTORY_WITH_JSON(MesonSupportFactory, "kdevmesonmanager.json", registerPlugin<MesonManager>();)

MesonManager::MesonManager( QObject* parent, const QVariantList& args )
    : AbstractFileManagerPlugin( QStringLiteral( "kdevmesonmanager" ), parent, args )
    , m_builder(new MesonBuilder(this))
{
}

KJob * MesonManager::createImportJob(KDevelop::ProjectFolderItem* item)
{
    auto project = item->project();

    auto job = new MesonImportJob(this, project, this);
    connect(job, &KJob::result, this, [this, job, project](){
        if (job->error() != 0) {
            qCWarning(KDEV_Meson) << "couldn't load project successfully" << project->name();
            m_projects.remove(project);
        }
    });

    const QList<KJob*> jobs = {
        builder()->configure(project),
        job,
        KDevelop::AbstractFileManagerPlugin::createImportJob(item) // generate the file system listing
    };

    Q_ASSERT(!jobs.contains(nullptr));
    auto composite = new KDevelop::ExecuteCompositeJob(this, jobs);
//     even if the cmake call failed, we want to load the project so that the project can be worked on
    composite->setAbortOnError(false);
    return composite;
}

KDevelop::Path MesonManager::buildDirectory(KDevelop::ProjectBaseItem* item) const
{
    KConfigGroup projectGroup = Meson::projectGroup(item->project());
    Path builddir(projectGroup.readEntry(Meson::BuildDirectory, QString()));
    if (builddir.isEmpty()) {
        const QString path = QFileDialog::getExistingDirectory(nullptr, i18n("Choose a build directory"));
        builddir = Path(path);
        projectGroup.writeEntry(Meson::BuildDirectory, path);
    }
    return builddir;
}

KDevelop::IProjectBuilder * MesonManager::builder() const
{
    return m_builder;
}

void MesonManager::setProjectData(KDevelop::IProject* project, const QJsonObject& data)
{
    m_projects[project] = data;
}

#include "mesonmanager.moc"

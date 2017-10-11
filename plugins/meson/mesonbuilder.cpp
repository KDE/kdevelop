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

#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iproject.h>
#include <util/path.h>
#include <outputview/outputexecutejob.h>
#include "mesonbuilder.h"
#include "mesonconfig.h"

MesonBuilder::MesonBuilder(QObject* parent)
    : QObject(parent)
{
    auto p = KDevelop::ICore::self()->pluginController()->pluginForExtension(QStringLiteral("org.kdevelop.IProjectBuilder"), QStringLiteral("KDevNinjaBuilder"));
    if (p) {
        m_ninjaBuilder = p->extension<KDevelop::IProjectBuilder>();
        if (m_ninjaBuilder) {
            connect(p, SIGNAL(built(KDevelop::ProjectBaseItem*)), this,
                    SIGNAL(built(KDevelop::ProjectBaseItem*)));
            connect(p, SIGNAL(cleaned(KDevelop::ProjectBaseItem*)), this,
                    SIGNAL(cleaned(KDevelop::ProjectBaseItem*)));
            connect(p, SIGNAL(installed(KDevelop::ProjectBaseItem*)), this,
                    SIGNAL(installed(KDevelop::ProjectBaseItem*)));
            connect(p, SIGNAL(failed(KDevelop::ProjectBaseItem*)), this,
                    SIGNAL(failed(KDevelop::ProjectBaseItem*)));
            connect(p, SIGNAL(makeTargetBuilt(KDevelop::ProjectBaseItem*,QString)), this,
                    SIGNAL(pruned(KDevelop::ProjectBaseItem*)));
        }
    }
}

KJob * MesonBuilder::configure(KDevelop::IProject* project)
{
    auto job = new KDevelop::OutputExecuteJob();
    *job << "meson" << project->path().toLocalFile();
    job->setWorkingDirectory(Meson::buildDirectory(project));
    return job;
}

KJob * MesonBuilder::build(KDevelop::ProjectBaseItem* item)
{
    //TODO: probably want to make sure it's configured first
    return m_ninjaBuilder->install(item);
}

KJob * MesonBuilder::clean(KDevelop::ProjectBaseItem* item)
{
    return m_ninjaBuilder->clean(item);
}

KJob * MesonBuilder::install(KDevelop::ProjectBaseItem* dom, const QUrl& installPath)
{
    return m_ninjaBuilder->install(dom, installPath);
}

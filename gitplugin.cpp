/***************************************************************************
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "gitplugin.h"

#include <KPluginFactory>
#include <KPluginLoader>
#include <klocalizedstring.h>
#include <QDebug>

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>

#include <vcs/vcsjob.h>
#include <vcs/vcsrevision.h>
#include <vcs/dvcs/dvcsjob.h>

#include "gitexecutor.h"

K_PLUGIN_FACTORY(KDevGitFactory, registerPlugin<GitPlugin>(); )
K_EXPORT_PLUGIN(KDevGitFactory("kdevgit"))

GitPlugin::GitPlugin( QObject *parent, const QVariantList & )
    : DistributedVersionControlPlugin(parent, KDevGitFactory::componentData())
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IBasicVersionControl )
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IDistributedVersionControl )

    core()->uiController()->addToolView(i18n("Git"), DistributedVersionControlPlugin::d->m_factory);

    QString EasterEgg = i18n("Thanks for the translation! Have a nice day, mr. translator!");
    Q_UNUSED(EasterEgg)

    setXMLFile("kdevgit.rc");

    DistributedVersionControlPlugin::d->m_exec = new GitExecutor(this);
}

GitPlugin::~GitPlugin()
{
    delete DistributedVersionControlPlugin::d;
}

KDevelop::VcsJob*
        GitPlugin::log(const KUrl& localLocation,
                       const KDevelop::VcsRevision& rev,
                       unsigned long limit)
{
    Q_UNUSED(limit)
    Q_UNUSED(rev)

    DVCSjob* job = d->m_exec->log(localLocation);
    return job;
}

KDevelop::VcsJob*
        GitPlugin::log(const KUrl& localLocation,
                       const KDevelop::VcsRevision& rev,
                       const KDevelop::VcsRevision& limit)
{
    Q_UNUSED(limit)
    return log(localLocation, rev, 0);
}

KDevelop::VcsJob* 
        GitPlugin::checkout(const QString &localLocation,
                            const QString &branch)
{
    DVCSjob* job = d->m_exec->checkout(localLocation, branch);
    return job;
}

// #include "gitplugin.moc"

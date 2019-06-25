/* This file is part of KDevelop
Copyright 2006 Adam Treat <treat@kde.org>
Copyright 2007 Andreas Pakulat <apaku@gmx.de>

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

#include "projectconfigskeleton.h"
#include "debug.h"

#include <interfaces/iproject.h>
#include <util/path.h>

#include <KIO/CopyJob>

using namespace KDevelop;

class KDevelop::ProjectConfigSkeletonPrivate
{
public:
    QString m_developerTempFile;
    QString m_projectTempFile;
    Path m_projectFile;
    Path m_developerFile;
    bool mUseDefaults;
};

ProjectConfigSkeleton::ProjectConfigSkeleton( const QString & configname )
    : KConfigSkeleton(configname)
    , d_ptr(new ProjectConfigSkeletonPrivate)
{
    Q_D(ProjectConfigSkeleton);

    d->m_developerTempFile = configname;
}

ProjectConfigSkeleton::ProjectConfigSkeleton( KSharedConfigPtr config )
    : KConfigSkeleton(config)
    , d_ptr(new ProjectConfigSkeletonPrivate)
{
    Q_D(ProjectConfigSkeleton);

    Q_ASSERT(config);
    d->m_developerTempFile = config->name();
}

ProjectConfigSkeleton::~ProjectConfigSkeleton() = default;

void ProjectConfigSkeleton::setDeveloperTempFile( const QString& cfg )
{
    Q_D(ProjectConfigSkeleton);

    d->m_developerTempFile = cfg;
    setSharedConfig( KSharedConfig::openConfig( cfg ) );
}

void ProjectConfigSkeleton::setProjectTempFile( const QString& cfg )
{
    Q_D(ProjectConfigSkeleton);

    d->m_projectTempFile = cfg;
    config()->addConfigSources( QStringList() << cfg );
    load();
}

void ProjectConfigSkeleton::setProjectFile( const Path& cfg )
{
    Q_D(ProjectConfigSkeleton);

    d->m_projectFile = cfg;
}

void ProjectConfigSkeleton::setDeveloperFile( const Path& cfg )
{
    Q_D(ProjectConfigSkeleton);

    d->m_developerFile = cfg;
}

Path ProjectConfigSkeleton::projectFile() const
{
    Q_D(const ProjectConfigSkeleton);

    return d->m_projectFile;
}

Path ProjectConfigSkeleton::developerFile() const
{
    Q_D(const ProjectConfigSkeleton);

    return d->m_developerFile;
}

void ProjectConfigSkeleton::setDefaults()
{
    Q_D(ProjectConfigSkeleton);

    qCDebug(PROJECT) << "Setting Defaults";
    KConfig cfg( d->m_projectTempFile );
    const auto items = this->items();
    for (KConfigSkeletonItem* item : items) {
        item->swapDefault();
        if( cfg.hasGroup( item->group() ) )
        {
            KConfigGroup grp = cfg.group( item->group() );
            if( grp.hasKey( item->key() ) )
                item->setProperty( grp.readEntry( item->key(), item->property() ) );
        }
    }
}

bool ProjectConfigSkeleton::useDefaults( bool b )
{
    Q_D(ProjectConfigSkeleton);

    if( b == d->mUseDefaults )
        return d->mUseDefaults;

    if( b )
    {
        KConfig cfg( d->m_projectTempFile );
        const auto items = this->items();
        for (KConfigSkeletonItem* item : items) {
            item->swapDefault();
            if( cfg.hasGroup( item->group() ) )
            {
                qCDebug(PROJECT) << "reading";
                KConfigGroup grp = cfg.group( item->group() );
                if( grp.hasKey( item->key() ) )
                    item->setProperty( grp.readEntry( item->key(), item->property() ) );
            }
        }
    } else
    {
        KConfig cfg( d->m_developerTempFile );
        KConfig defCfg( d->m_projectTempFile );
        const auto items = this->items();
        for (KConfigSkeletonItem* item : items) {
            if( cfg.hasGroup( item->group() ) )
            {
                KConfigGroup grp = cfg.group( item->group() );
                if( grp.hasKey( item->key() ) )
                    item->setProperty( grp.readEntry( item->key(), item->property() ) );
                else
                {
                    KConfigGroup grp = defCfg.group( item->group() );
                    item->setProperty( grp.readEntry( item->key(), item->property() ) );
                }
            } else
            {
                KConfigGroup grp = defCfg.group( item->group() );
                item->setProperty( grp.readEntry( item->key(), item->property() ) );
            }
        }
    }
    d->mUseDefaults = b;
    return !d->mUseDefaults;
}

bool ProjectConfigSkeleton::writeConfig()
{
    Q_D(ProjectConfigSkeleton);

    KConfigSkeletonItem::List myitems = items();
    KConfigSkeletonItem::List::ConstIterator it;
    for( it = myitems.constBegin(); it != myitems.constEnd(); ++it )
    {
        (*it)->writeConfig( config() );
    }

    config()->sync();

    load();

    auto copyJob = KIO::copy(QUrl::fromLocalFile(d->m_developerTempFile), d->m_developerFile.toUrl(), KIO::HideProgressInfo);
    copyJob ->exec();

    emit configChanged();
    return true;
}

/*
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
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

    const KConfigSkeletonItem::List myitems = items();
    for (auto* item : myitems) {
        item->writeConfig(config());
    }

    config()->sync();

    load();

    auto copyJob = KIO::copy(QUrl::fromLocalFile(d->m_developerTempFile), d->m_developerFile.toUrl(), KIO::HideProgressInfo);
    copyJob ->exec();

    emit configChanged();
    return true;
}

#include "moc_projectconfigskeleton.cpp"

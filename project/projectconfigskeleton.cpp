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
#include <interfaces/iproject.h>

#include <kdebug.h>
#include <kio/netaccess.h>

namespace KDevelop
{

struct ProjectConfigSkeletonPrivate
{
    QString m_developerTempFile;
    QString m_projectTempFile;
    KUrl m_projectFileUrl;
    KUrl m_developerFileUrl;
    bool mUseDefaults;
};

ProjectConfigSkeleton::ProjectConfigSkeleton( const QString & configname )
        : KConfigSkeleton( configname ), d( new ProjectConfigSkeletonPrivate )
{
    d->m_developerTempFile = configname;
}

ProjectConfigSkeleton::ProjectConfigSkeleton( KSharedConfig::Ptr config )
        : KConfigSkeleton( config ), d( new ProjectConfigSkeletonPrivate )
{
}

void ProjectConfigSkeleton::setDeveloperTempFile( const QString& cfg )
{
    d->m_developerTempFile = cfg;
    setSharedConfig( KSharedConfig::openConfig( cfg ) );
}

void ProjectConfigSkeleton::setProjectTempFile( const QString& cfg )
{
    d->m_projectTempFile = cfg;
    config()->addConfigSources( QStringList() << cfg );
    readConfig();
}

void ProjectConfigSkeleton::setProjectFileUrl( const QString& cfg )
{
    d->m_projectFileUrl = KUrl(cfg);
}

void ProjectConfigSkeleton::setDeveloperFileUrl( const QString& cfg )
{
    d->m_developerFileUrl = KUrl(cfg);
}

KUrl ProjectConfigSkeleton::projectFileUrl() const
{
    return d->m_projectFileUrl;
}

KUrl ProjectConfigSkeleton::developerFileUrl() const
{
    return d->m_developerFileUrl;
}

void ProjectConfigSkeleton::setDefaults()
{
    kDebug(9503) << "Setting Defaults";
    KConfig cfg( d->m_projectTempFile );
    Q_FOREACH( KConfigSkeletonItem* item, items() )
    {
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
    if( b == d->mUseDefaults )
        return d->mUseDefaults;

    if( b )
    {
        KConfig cfg( d->m_projectTempFile );
        Q_FOREACH( KConfigSkeletonItem* item, items() )
        {
            item->swapDefault();
            if( cfg.hasGroup( item->group() ) )
            {
                kDebug(9503) << "reading";
                KConfigGroup grp = cfg.group( item->group() );
		if( grp.hasKey( item->key() ) )
                    item->setProperty( grp.readEntry( item->key(), item->property() ) );
            }
        }
    }else
    {
        KConfig cfg( d->m_developerTempFile );
        KConfig defCfg( d->m_projectTempFile );
        Q_FOREACH( KConfigSkeletonItem* item, items() )
        {
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
            }else
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
    KConfigSkeletonItem::List myitems = items();
    KConfigSkeletonItem::List::ConstIterator it;
    for( it = myitems.constBegin(); it != myitems.constEnd(); ++it )
    {
        (*it)->writeConfig( config() );
    }

    config()->sync();

    readConfig();

    KIO::NetAccess::upload( d->m_developerTempFile, d->m_developerFileUrl, 0 );

    emit configChanged();
    return true;
}

ProjectConfigSkeleton::~ProjectConfigSkeleton()
{
    delete d;
}

}
#include "projectconfigskeleton.moc"


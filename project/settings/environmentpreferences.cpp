/* This file is part of KDevelop
Copyright 2006 Adam Treat <treat@kde.org>

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
#include "environmentpreferences.h"

#include <QVBoxLayout>

#include <kgenericfactory.h>
#include <kconfiggroup.h>
#include <kconfig.h>

#include "environmentwidget.h"
#include "projectconfigskeleton.h"

namespace KDevelop
{

class EnvironmentPreferencesPrivate
{
public:
    ProjectConfigSkeleton *m_skel;
    EnvironmentWidget *preferencesDialog;
    KConfig* m_config;
};

typedef KGenericFactory<EnvironmentPreferences> PreferencesFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kdev_envsettings, PreferencesFactory( "kcm_kdev_envsettings" ) )

EnvironmentPreferences::EnvironmentPreferences( QWidget *parent, const QStringList &args )
    : KCModule( PreferencesFactory::componentData(), parent, args )
    , d( new EnvironmentPreferencesPrivate )
{
    QVBoxLayout * l = new QVBoxLayout( this );
    d->preferencesDialog = new EnvironmentWidget( this );
    l->addWidget( d->preferencesDialog );

    connect( d->preferencesDialog, SIGNAL( changed() ),
             this, SLOT( settingsChanged() ) );

    // init kconfigskeleton
    d->m_skel = new ProjectConfigSkeleton( args.first() );
    d->m_skel->setDeveloperTempFile( args.at(1) );
    d->m_skel->setProjectFileUrl( args.at(2) );
    d->m_skel->setDeveloperFileUrl( args.at(3) );

    addConfig( d->m_skel, d->preferencesDialog );

    d->m_config = d->m_skel->config();

    load();
}

EnvironmentPreferences::~EnvironmentPreferences( )
{
    delete d->m_skel;
    delete d;
}

void EnvironmentPreferences::save()
{
    d->preferencesDialog->saveSettings( d->m_config );
    KCModule::save();
}

void EnvironmentPreferences::load()
{
    d->preferencesDialog->loadSettings( d->m_config );
    KCModule::load();
}

void EnvironmentPreferences::defaults()
{
    d->preferencesDialog->defaults( d->m_config );
    KCModule::defaults();
}

void EnvironmentPreferences::settingsChanged( bool changed )
{
    Q_UNUSED( changed )
    unmanagedWidgetChangeState( true );
}

KUrl EnvironmentPreferences::localNonShareableFile() const
{
    return KUrl::fromPath(
               KStandardDirs::locate( "data", "kdevelop/data.kdev4" ) );
}

}
#include "environmentpreferences.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on

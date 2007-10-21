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

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kconfiggroup.h>
#include <kconfig.h>
#include <kdebug.h>

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

K_PLUGIN_FACTORY(PreferencesFactory, registerPlugin<EnvironmentPreferences>(); )
K_EXPORT_PLUGIN(PreferencesFactory("kcm_kdev_envsettings"))

EnvironmentPreferences::EnvironmentPreferences( QWidget *parent, const QVariantList &args )
    : KCModule( PreferencesFactory::componentData(), parent, args )
    , d( new EnvironmentPreferencesPrivate )
{
    QVBoxLayout * l = new QVBoxLayout( this );
    d->preferencesDialog = new EnvironmentWidget( this );
    l->addWidget( d->preferencesDialog );

    connect( d->preferencesDialog, SIGNAL( changed() ),
             this, SLOT( settingsChanged() ) );

    // init kconfigskeleton
    d->m_skel = new ProjectConfigSkeleton( args.first().toString() );
    d->m_skel->setDeveloperTempFile( args.at(1).toString() );
    d->m_skel->setProjectFileUrl( args.at(2).toString() );
    d->m_skel->setDeveloperFileUrl( args.at(3).toString() );

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
    kDebug(9503) << "Loading env settings";
    d->preferencesDialog->loadSettings( d->m_config );
    KCModule::load();
}

void EnvironmentPreferences::defaults()
{
    d->preferencesDialog->defaults( d->m_config );
    KCModule::defaults();
}

void EnvironmentPreferences::settingsChanged()
{
    unmanagedWidgetChangeState( true );
}

KUrl EnvironmentPreferences::localNonShareableFile() const
{
    return KUrl::fromPath(
               KStandardDirs::locate( "data", "kdevelop/data.kdev4" ) );
}

}
#include "environmentpreferences.moc"


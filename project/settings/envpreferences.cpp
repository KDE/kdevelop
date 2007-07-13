/* This file is part of KDevelop
Copyright (C) 2006 Adam Treat <treat@kde.org>

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
#include "envpreferences.h"

#include <QVBoxLayout>

#include <kgenericfactory.h>
#include <kconfiggroup.h>
#include <kconfig.h>

#include "envwidget.h"
// #include "envconfig.h"
#include "projectconfigskeleton.h"

namespace KDevelop
{

class EnvPreferences::Private
{
public:
    ProjectConfigSkeleton *m_skel;
    EnvWidget *preferencesDialog;
    KConfig *m_config;
};

typedef KGenericFactory<EnvPreferences> PreferencesFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kdev_envsettings, PreferencesFactory( "kcm_kdev_envsettings" ) )

EnvPreferences::EnvPreferences( QWidget *parent, const QStringList &args )
//     : ProjectKCModule<EnvSettings>( PreferencesFactory::componentData(), parent, args )
    : KCModule( PreferencesFactory::componentData(), parent, args )
    , d( new EnvPreferences::Private )
{
    QVBoxLayout * l = new QVBoxLayout( this );
    d->preferencesDialog = new EnvWidget( this );
    l->addWidget( d->preferencesDialog );

    connect( d->preferencesDialog, SIGNAL( changed() ),
             this, SLOT( settingsChanged() ) );

    // init kconfigskeleton
    d->m_skel = new ProjectConfigSkeleton( args.first() );
    d->m_skel->setDeveloperTempFile( args.at(1) );
    d->m_skel->setProjectFileUrl( args.at(2) );
    d->m_skel->setDeveloperFileUrl( args.at(3) );

//     addConfig( EnvSettings::self(), d->preferencesDialog );
    addConfig( d->m_skel, d->preferencesDialog );

//     d->m_config = EnvSettings::self()->config();
    d->m_config = d->m_skel->config();
    d->preferencesDialog->setConfig( d->m_config, "Project Env Settings" );

    load();
}

EnvPreferences::~EnvPreferences( )
{
    delete d->m_skel;
    delete d;
}

void EnvPreferences::save()
{
    d->preferencesDialog->saveSettings();
//     ProjectKCModule<EnvSettings>::save();
    KCModule::save();
}

void EnvPreferences::load()
{
    d->preferencesDialog->loadSettings();
//     ProjectKCModule<EnvSettings>::load();
    KCModule::load();
}

void EnvPreferences::defaults()
{
    d->preferencesDialog->defaults();
//     ProjectKCModule<EnvSettings>::defaults();
    KCModule::defaults();
}

void EnvPreferences::settingsChanged(/* bool changed */)
{
    unmanagedWidgetChangeState( true );
}

}
#include "envpreferences.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on

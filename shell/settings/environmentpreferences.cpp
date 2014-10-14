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
#include <KSharedConfig>
#include <KConfigSkeleton>
#include <kconfiggroup.h>
#include <kaboutdata.h>
#include <kconfig.h>

#include "environmentwidget.h"

namespace KDevelop
{

class EnvironmentPreferencesPrivate
{
public:
    EnvironmentWidget *preferencesDialog;
    KConfigSkeleton* skel;
    QString activeGroup;
};

K_PLUGIN_FACTORY_WITH_JSON(PreferencesFactory, "kcm_kdev_envsettings.json", registerPlugin<EnvironmentPreferences>();)

EnvironmentPreferences::EnvironmentPreferences( QWidget *parent, const QVariantList &args )
    : KCModule( KAboutData::pluginData("kcm_kdev_envsettings"), parent, args )
    , d( new EnvironmentPreferencesPrivate )
{
    QVBoxLayout * l = new QVBoxLayout( this );
    d->preferencesDialog = new EnvironmentWidget( this );
    l->addWidget( d->preferencesDialog );

    connect( d->preferencesDialog, SIGNAL(changed()),
             this, SLOT(settingsChanged()) );


    d->skel = new KConfigSkeleton(KSharedConfig::openConfig());
    addConfig( d->skel, d->preferencesDialog );

    if (!args.isEmpty() && args.first().canConvert<QString>()) {
        d->activeGroup = args.first().toString();
    }
}

EnvironmentPreferences::~EnvironmentPreferences( )
{
    delete d;
}

void EnvironmentPreferences::save()
{
    d->preferencesDialog->saveSettings( d->skel->config() );
    KCModule::save();
}

void EnvironmentPreferences::load()
{
    d->preferencesDialog->loadSettings( d->skel->config() );
    d->preferencesDialog->setActiveGroup( d->activeGroup );
    KCModule::load();
}

void EnvironmentPreferences::defaults()
{
    d->preferencesDialog->defaults( d->skel->config() );
    KCModule::defaults();
}

void EnvironmentPreferences::settingsChanged()
{
    unmanagedWidgetChangeState( true );
}

}
#include "environmentpreferences.moc"


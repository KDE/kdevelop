/* This file is part of KDevelop
Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>

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
#include "environmentselectwidget.h"
#include "environmentgrouplist.h"
#include <ksettings/dispatcher.h>
#include <kcomponentdata.h>

namespace KDevelop
{

class EnvironmentSelectWidgetPrivate
{
public:
    KSharedConfigPtr m_config;
    QString m_group;
    QString m_entry;
};

EnvironmentSelectWidget::EnvironmentSelectWidget( QWidget *parent )
    : KComboBox( parent ), d( new EnvironmentSelectWidgetPrivate )
{ 
    // doesn't work for some reason. reimplement showEvent() instead.
//     KComponentData data( "kdevplatformproject", "kdevplatformproject",
//                          KComponentData::SkipMainComponentRegistration );
//     KSettings::Dispatcher::registerComponent( data, this, SLOT(updateEnvGroup()) );
//     KSettings::Dispatcher::registerComponent( KGlobal::mainComponent(), this, SLOT(updateEnvGroup()) );

}

EnvironmentSelectWidget::~EnvironmentSelectWidget()
{
    delete d;
}

void EnvironmentSelectWidget::setConfigObject( KSharedConfigPtr config, const QString &group,
                                       const QString &entry )
{
    d->m_config = config;
    d->m_group = group;
    d->m_entry = entry;
}

void EnvironmentSelectWidget::loadSettings()
{
    KConfigGroup cfgGroup( d->m_config, d->m_group );
    QString active = cfgGroup.readEntry( d->m_entry, QString() );
    EnvironmentGroupList env( d->m_config );
    QStringList profiles = env.groups();

    clear();
    addItems( profiles );
    if( profiles.contains( active ) )
    {
        int idx = findText( active );
        setCurrentIndex( idx );
    }
}

void EnvironmentSelectWidget::saveSettings()
{
    KConfigGroup cfgGroup( d->m_config, d->m_group );
    cfgGroup.writeEntry( d->m_entry, currentText() );
    cfgGroup.sync();
}

// void EnvironmentSelectWidget::updateEnvGroup()
// {
//     QStringList items = EnvWidget::environmentProfiles( d->m_config );
//     kDebug() << "Environment Profiles" << items;
//     combo()->clear();
//     combo()->addItems( items );
// }

void EnvironmentSelectWidget::showEvent( QShowEvent *ev )
{
    loadSettings();
    KComboBox::showEvent( ev );
}

}

#include "EnvironmentSelectWidget.moc"

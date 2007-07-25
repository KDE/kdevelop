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
#include "envselectwidget.h"
#include "envwidget.h"
#include <ksettings/dispatcher.h>
#include <kcomponentdata.h>

namespace KDevelop
{

class EnvSelectWidget::Private
{
public:
    KConfig *m_config;
    QString m_group;
    QString m_entry;
};

EnvSelectWidget::EnvSelectWidget( QWidget *parent )
    : KComboBox( parent ), d( new EnvSelectWidget::Private )
{
    // doesn't work for some reason. reimplement showEvent() instead.
//     KComponentData data( "kdevplatformproject", "kdevplatformproject",
//                          KComponentData::SkipMainComponentRegistration );
//     KSettings::Dispatcher::registerComponent( data, this, SLOT(updateEnvGroup()) );
//     KSettings::Dispatcher::registerComponent( KGlobal::mainComponent(), this, SLOT(updateEnvGroup()) );

    connect( combo(), SIGNAL(currentIndexChanged(int)), this, SLOT(currentIndexChanged(int)) );
}

EnvSelectWidget::~EnvSelectWidget()
{
    delete d;
}

void EnvSelectWidget::setConfigObject( KConfig *config, const QString &group,
                                       const QString &entry )
{
    d->m_config = config;
    d->m_group = group;
    d->m_entry = entry;
}

void EnvSelectWidget::loadSettings()
{
    KConfigGroup cfgGroup( d->m_config, d->m_group );
    QString active = cfgGroup.readEntry( d->m_entry, QString() );
    QStringList profiles = EnvWidget::environmentProfiles( d->m_config );

    combo()->clear();
    combo()->addItems( profiles );
    if( profiles.contains( active ) )
    {
        int idx = combo()->findText( active );
        combo()->setCurrentIndex( idx );
    }
}

void EnvSelectWidget::saveSettings()
{
    KConfigGroup cfgGroup( d->m_config, d->m_group );
    cfgGroup.writeEntry( d->m_entry, combo()->currentText() );
    cfgGroup.sync();
}

// void EnvSelectWidget::updateEnvGroup()
// {
//     QStringList items = EnvWidget::environmentProfiles( d->m_config );
//     kDebug() << "Environment Profiles " << items << endl;
//     combo()->clear();
//     combo()->addItems( items );
// }

void EnvSelectWidget::showEvent( QShowEvent *ev )
{
    loadSettings();
    KComboBox::showEvent( ev );
}

void EnvSelectWidget::currentIndexChanged(int /*idx*/)
{
    emit changed();
}

KComboBox* EnvSelectWidget::combo()
{
    return this;
}

}

#include "envselectwidget.moc"

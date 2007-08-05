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
#include "environmentselectionwidget.h"
#include "environmentgrouplist.h"
#include <ksettings/dispatcher.h>
#include <kcomponentdata.h>

namespace KDevelop
{

class EnvironmentSelectionWidgetPrivate
{
};

EnvironmentSelectionWidget::EnvironmentSelectionWidget( QWidget *parent )
    : KComboBox( parent ), d( new EnvironmentSelectionWidgetPrivate )
{ 
    // doesn't work for some reason. reimplement showEvent() instead.
//     KComponentData data( "kdevplatformproject", "kdevplatformproject",
//                          KComponentData::SkipMainComponentRegistration );
//     KSettings::Dispatcher::registerComponent( data, this, SLOT(updateEnvGroup()) );
//     KSettings::Dispatcher::registerComponent( KGlobal::mainComponent(), this, SLOT(updateEnvGroup()) );

}

EnvironmentSelectionWidget::~EnvironmentSelectionWidget()
{
    delete d;
}

QString EnvironmentSelectionWidget::currentProfile() const
{
    return currentText();
}

void EnvironmentSelectionWidget::setCurrentProfile( const QString& profile )
{
    setCurrentItem( profile );
}


}

#include "environmentselectionwidget.moc"

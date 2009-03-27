/* KDevelop Covergage lcov settings
*
* Copyright 2009  Andreas Pakulat <apaku@gmx.de>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
* 02110-1301, USA.
*/
#include "lcovprefs.h"

#include <kgenericfactory.h>
#include <kaboutdata.h>

#include "lcovconfig.h"

K_PLUGIN_FACTORY(LCovPrefsFactory, registerPlugin<LCovPrefs>();)
K_EXPORT_PLUGIN(LCovPrefsFactory(KAboutData("kcm_kdev_lcovsettings", "kdevelop", ki18n("LCov Settings"), "0.1")))

LCovPrefs::LCovPrefs( QWidget *parent, const QVariantList &args )
        : KCModule( LCovPrefsFactory::componentData(), parent, args )
{

    QVBoxLayout * l = new QVBoxLayout( this );
    QWidget* w = new QWidget(parent);
    preferencesDialog.setupUi( w );
    l->addWidget( w );

    addConfig( LCovSettings::self(), w );

    load();
}

void LCovPrefs::save()
{
    KCModule::save();
}

void LCovPrefs::load()
{
    KCModule::load();
}

void LCovPrefs::defaults()
{
    KCModule::defaults();
}

#include "lcovprefs.moc"


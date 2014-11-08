/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
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

#include "makebuilderpreferences.h"

#include <QVBoxLayout>

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <util/environmentgrouplist.h>
#include <KAboutData>

#include "ui_makeconfig.h"
#include "makebuilderconfig.h"

K_PLUGIN_FACTORY(MakeBuilderPreferencesFactory, registerPlugin<MakeBuilderPreferences>(); )
// K_EXPORT_PLUGIN(MakeBuilderPreferencesFactory("kcm_kdev_makebuilder"))

MakeBuilderPreferences::MakeBuilderPreferences(QWidget* parent, const QVariantList& args)
    : ProjectKCModule<MakeBuilderSettings>( KAboutData::pluginData("kcm_kdev_makebuilder"),
                parent, args)
{
    QVBoxLayout* l = new QVBoxLayout( this );
    QWidget* w = new QWidget;
    m_prefsUi = new Ui::MakeConfig;
    m_prefsUi->setupUi( w );
    connect( m_prefsUi->makeBinary, &KUrlRequester::textChanged, this, static_cast<void(MakeBuilderPreferences::*)()>(&MakeBuilderPreferences::changed) );
    connect( m_prefsUi->makeBinary, &KUrlRequester::urlSelected, this, static_cast<void(MakeBuilderPreferences::*)()>(&MakeBuilderPreferences::changed) );
    l->addWidget( w );

    m_prefsUi->configureEnvironment->setSelectionWidget( m_prefsUi->kcfg_environmentProfile );

    addConfig( MakeBuilderSettings::self(), w );
}

void MakeBuilderPreferences::load()
{
    KConfigSkeletonItem* item = MakeBuilderSettings::self()->findItem("makeBinary");
    if( item )
    {
        bool tmp = m_prefsUi->makeBinary->blockSignals( true );
        m_prefsUi->makeBinary->setText( item->property().toString() );
        m_prefsUi->makeBinary->blockSignals( tmp );
    }
    ProjectKCModule<MakeBuilderSettings>::load();
}

void MakeBuilderPreferences::save()
{
    KConfigSkeletonItem* item = MakeBuilderSettings::self()->findItem("makeBinary");
    if( item && !item->isEqual( QVariant( m_prefsUi->makeBinary->text() ) ) )
    {
        item->setProperty( m_prefsUi->makeBinary->text() );
        MakeBuilderSettings::self()->writeConfig();
    }
    ProjectKCModule<MakeBuilderSettings>::save();
}

void MakeBuilderPreferences::defaults()
{
    qDebug() << "setting to defaults";
    KConfigSkeletonItem* item = MakeBuilderSettings::self()->findItem("makeBinary");
    if( item )
    {
        bool sig = m_prefsUi->makeBinary->blockSignals( true );
        item->swapDefault();
        m_prefsUi->makeBinary->setText( item->property().toString() );
        item->swapDefault();
        m_prefsUi->makeBinary->blockSignals( sig );
        unmanagedWidgetChangeState(true);
    }
    ProjectKCModule<MakeBuilderSettings>::defaults();
}

MakeBuilderPreferences::~MakeBuilderPreferences()
{
    delete m_prefsUi;
}

QString MakeBuilderPreferences::standardMakeComannd()
{
#ifdef _MSC_VER
    return QLatin1String("nmake");
#else
    return QLatin1String("make");
#endif
}

#include "makebuilderpreferences.moc"


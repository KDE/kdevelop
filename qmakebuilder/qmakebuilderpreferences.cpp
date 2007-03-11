/* KDevelop QMake Support
 *
 * Copyright 2007 Andreas Pakulat <apaku@gmx.de>
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

#include "qmakebuilderpreferences.h"

#include <QVBoxLayout>

#include <kgenericfactory.h>
#include <kurlrequester.h>

#include "ui_qmakeconfig.h"
#include "qmakebuilderconfig.h"

typedef KGenericFactory<QMakeBuilderPreferences> QMakeBuilderPreferencesFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kdev_qmakebuilder, QMakeBuilderPreferencesFactory( "kcm_kdev_qmakebuilder" )  )

QMakeBuilderPreferences::QMakeBuilderPreferences(QWidget* parent, const QStringList& args)
    : KDevelop::ConfigModule(QMakeBuilderSettings::self(),
                             QMakeBuilderPreferencesFactory::componentData(),
                             parent, args)
{
    QVBoxLayout* l = new QVBoxLayout( this );
    QWidget* w = new QWidget;
    m_prefsUi = new Ui::QMakeConfig;
    m_prefsUi->setupUi( w );
    l->addWidget( w );

    addConfig( QMakeBuilderSettings::self(), w );
    kDebug(9024) << "Creating QMake KCM" << endl;
    load();

}

KUrl QMakeBuilderPreferences::localNonShareableFile() const
{
    return KUrl::fromPath(
            KStandardDirs::locate( "data", "kdevqmakebuilder/data.kdev4") );
}

QMakeBuilderPreferences::~QMakeBuilderPreferences()
{
}

void QMakeBuilderPreferences::load()
{
    kDebug(9024) << "Loading qmake config" << endl;
    KDevelop::ConfigModule::load();
    m_prefsUi->qmakebin->setUrl(QMakeBuilderSettings::qmakebin());
    connect(m_prefsUi->qmakebin, SIGNAL(textChanged(const QString&)), this, SLOT(changed()));
    connect(m_prefsUi->qmakebin, SIGNAL(returnPressed()), this, SLOT(changed()));
    connect(m_prefsUi->qmakebin, SIGNAL(urlSelected(const KUrl&)), this, SLOT(changed()));
}

void QMakeBuilderPreferences::save()
{
    kDebug(9024) << "Saving qmake config" << endl;
    QMakeBuilderSettings::setQmakebin(m_prefsUi->qmakebin->url().url());
    KDevelop::ConfigModule::save();
}

void QMakeBuilderPreferences::defaults()
{
    KDevelop::ConfigModule::defaults();
}

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;

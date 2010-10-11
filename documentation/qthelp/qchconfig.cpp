/*  This file is part of KDevelop

    Copyright 2010 Benjamin Port <port.benjamin@gmail.com>

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

#include "qchconfig.h"

#include <KLocalizedString>
#include <KPluginFactory>
#include <KPluginLoader>
#include <KConfigGroup>
#include <KUrlRequester>
#include <KUrl>
#include <KAboutData>
#include <KPushButton>
#include <KIconLoader>
#include <KDebug>
#include <KFile>
#include <KSettings/Dispatcher>

#include <QtGui/QBoxLayout>
#include <QtGui/QLabel>
#include <QtCore/QFile>
#include <QtCore/QStringList>

#include "qchsettings.h"
#include "ui_qchconfig.h"

K_PLUGIN_FACTORY(QchConfigFactory, registerPlugin<QchConfig>();)
K_EXPORT_PLUGIN(QchConfigFactory("kdevqch_config"))


QchConfig::QchConfig(QWidget *parent, const QVariantList &args)
    : KCModule(QchConfigFactory::componentData(), parent, args)
{
    QVBoxLayout * l = new QVBoxLayout( this );

    QWidget* w = new QWidget;
    m_configWidget = new Ui::QchConfigUI;
    m_configWidget->setupUi( w );

    KUrlRequester *k= new KUrlRequester();
    k->setMode( KFile::File | KFile::ExistingOnly );
    k->setFilter("*.qch|Qt Compressed Help");
    m_configWidget->kcfg_qchList->setCustomEditor(k->customEditor());
    l->addWidget( w );

    addConfig( QchSettings::self(), w );

    load();
}

QchConfig::~QchConfig()
{
    delete m_configWidget;
}

void QchConfig::save()
{
    KCModule::save();

    // looks like we have to force a write so readConfig() can get the new values
    QchSettings::self()->writeConfig();

    KSettings::Dispatcher::reparseConfiguration( componentData().componentName() );
}

#include "qchconfig.moc"

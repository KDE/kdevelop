/*
    SPDX-FileCopyrightText: 2006 Matt Rogers <mattr@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "preferences.h"

#include <QVBoxLayout>

#include "ui_settingswidget.h"
#include <config.h>


using PreferencesFactory = KGenericFactory<Preferences>;
K_EXPORT_COMPONENT_FACTORY( kcm_documentview_settings, PreferencesFactory( "kcm_documentview_settings" ) )

Preferences::Preferences(QWidget *parent, const QStringList &args)
    : KDevelop::ConfigModule( DocumentViewSettings::self(), PreferencesFactory::componentData(), parent, args )
{

    QVBoxLayout* l = new QVBoxLayout( this );
    QWidget* w = new QWidget;
    preferencesDialog = new Ui::SettingsWidget;
    preferencesDialog->setupUi( w );
    l->addWidget( w );

    addConfig( DocumentViewSettings::self(), w );

    load();
}

Preferences::~Preferences( )
{
    delete preferencesDialog;
}

void Preferences::save()
{
    KDevelop::ConfigModule::save();
}

void Preferences::load()
{
    KDevelop::ConfigModule::load();
}

void Preferences::slotSettingsChanged()
{
    emit changed( true );
}

void Preferences::defaults()
{
}



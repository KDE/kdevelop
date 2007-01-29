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

#include "kdevconfig.h"

#include <kmenu.h>
#include <klocale.h>
#include <kglobal.h>
#include <kcomponentdata.h>

#include <kcmultidialog.h>
#include <ksettings/dialog.h>

#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "kdevprojectcontroller.h"

namespace Koncrete
{

static KStaticDeleter<ConfigPrivate> s_deleter;

ConfigPrivate *ConfigPrivate::s_private = 0;

ConfigPrivate::ConfigPrivate()
        : QObject( 0 ),
        mode( Config::Standard ),
        settingsDialog( 0 )
{}

ConfigPrivate::~ConfigPrivate()
{
}

ConfigPrivate *ConfigPrivate::self()
{
    if ( !s_private )
        s_deleter.setObject( s_private, new ConfigPrivate );
    return s_private;
}

void ConfigPrivate::local()
{
    setMode( Config::LocalProject );
}

void ConfigPrivate::shared()
{
    setMode( Config::GlobalProject );
}

void ConfigPrivate::global()
{
    setMode( Config::Standard );
}

void ConfigPrivate::setMode( Config::Mode m )
{
    mode = m;
    switch ( mode )
    {
    case Config::Standard:
        settingsDialog->dialog() ->setButtonText( KDialog::User2, i18n( "Standard" ) );
        break;
    case Config::LocalProject:
        settingsDialog->dialog() ->setButtonText( KDialog::User2, i18n( "Local Project" ) );
        break;
    case Config::GlobalProject:
        settingsDialog->dialog() ->setButtonText( KDialog::User2, i18n( "Global Project" ) );
        break;
    default:
        break;
    }
}

#define d (ConfigPrivate::self())

Config::Config()
{}

Config::~Config()
{}

Config::Mode Config::mode()
{
    return d->mode;
}

// void Config::setMode( Config::Mode m )
// {
//     d->setMode( m );
// }

// void Config::setAllowedModes( Config::Mode m )
// {
// }

void Config::settingsDialog()
{
    if ( !d->settingsDialog )
    {
        if ( Core::mainWindow() ->componentData() .componentName() == "kdevelop" )
            d->settingsDialog = new KSettings::Dialog(
                                    KSettings::Dialog::Static, Core::mainWindow() );
        else
            d->settingsDialog = new KSettings::Dialog( QStringList( "kdevelop" ),
                                KSettings::Dialog::Static, Core::mainWindow() );

        KCMultiDialog *dialog = d->settingsDialog->dialog();
        dialog->setButtons( KDialog::Help | KDialog::Default | KDialog::Cancel
                            | KDialog::Apply | KDialog::Ok | KDialog::User2 );

        KMenu *m = new KMenu;
        QAction *action;
        action = m->addAction( i18n( "Local Project" ) );
        QObject::connect( action, SIGNAL( triggered() ), d, SLOT( local() ) );
        action = m->addAction( i18n( "Global Project" ) );
        QObject::connect( action, SIGNAL( triggered() ), d, SLOT( shared() ) );
        action = m->addAction( i18n( "Standard" ) );
        QObject::connect( action, SIGNAL( triggered() ), d, SLOT( global() ) );
        dialog->setButtonText( KDialog::User2, i18n( "Standard" ) );
        dialog->setButtonMenu( KDialog::User2, m );
    }

    d->settingsDialog->show();
}

KConfig *Config::standard()
{
    return sharedStandard().data();
}

KConfig *Config::localProject()
{
    return sharedLocalProject().data();
}

KConfig *Config::globalProject()
{
    return sharedGlobalProject().data();
}

KSharedConfig::Ptr Config::sharedStandard()
{
    KSharedConfig::Ptr config = KGlobal::config();
    QStringList current = config->extraConfigFiles();
    QStringList extraConfig;
    KUrl local = Core::projectController() ->localFile();
    KUrl global = Core::projectController() ->globalFile();
    if ( local.isValid() )
        extraConfig.append( local.path() );
    if ( global.isValid() )
        extraConfig.append( global.path() );

    if ( current != extraConfig )
    {
        config ->sync();
        config ->setExtraConfigFiles( extraConfig );
        config ->reparseConfiguration();
    }

    return config;
}

KSharedConfig::Ptr Config::sharedLocalProject()
{
    KSharedConfig::Ptr config = KGlobal::config();
    QStringList current = config->extraConfigFiles();
    QStringList extraConfig;
    KUrl local = Core::projectController() ->localFile();
    KUrl global = Core::projectController() ->globalFile();
    if ( global.isValid() )
        extraConfig.append( global.path() );
    if ( local.isValid() )
        extraConfig.append( local.path() );

    if ( current != extraConfig )
    {
        config ->sync();
        config ->setExtraConfigFiles( extraConfig );
        config ->reparseConfiguration();
    }

    return config;
}

KSharedConfig::Ptr Config::sharedGlobalProject()
{
    KSharedConfig::Ptr config = KGlobal::config();
    QStringList current = config->extraConfigFiles();
    QStringList extraConfig;
    KUrl local = Core::projectController() ->localFile();
    KUrl global = Core::projectController() ->globalFile();
    if ( local.isValid() )
        extraConfig.append( local.path() );
    if ( global.isValid() )
        extraConfig.append( global.path() );

    if ( current != extraConfig )
    {
        config ->sync();
        config ->setExtraConfigFiles( extraConfig );
        config ->reparseConfiguration();
    }

    return config;
}

}

#include "kdevconfig.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on

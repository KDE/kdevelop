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
#include <kinstance.h>

#include <kcmultidialog.h>
#include <ksettings/dialog.h>

#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "kdevprojectcontroller.h"

static KStaticDeleter<KDevConfigPrivate> s_deleter;

KDevConfigPrivate *KDevConfigPrivate::s_private = 0;

KDevConfigPrivate::KDevConfigPrivate()
        : QObject( 0 ),
        mode( KDevConfig::Standard ),
        settingsDialog( 0 )
{}

KDevConfigPrivate::~KDevConfigPrivate()
{
}

KDevConfigPrivate *KDevConfigPrivate::self()
{
    if ( !s_private )
        s_deleter.setObject( s_private, new KDevConfigPrivate );
    return s_private;
}

void KDevConfigPrivate::local()
{
    setMode( KDevConfig::LocalProject );
}

void KDevConfigPrivate::shared()
{
    setMode( KDevConfig::GlobalProject );
}

void KDevConfigPrivate::global()
{
    setMode( KDevConfig::Standard );
}

void KDevConfigPrivate::setMode( KDevConfig::Mode m )
{
    mode = m;
    switch ( mode )
    {
    case KDevConfig::Standard:
        settingsDialog->dialog() ->setButtonText( KDialog::User2, i18n( "Standard" ) );
        break;
    case KDevConfig::LocalProject:
        settingsDialog->dialog() ->setButtonText( KDialog::User2, i18n( "Local Project" ) );
        break;
    case KDevConfig::GlobalProject:
        settingsDialog->dialog() ->setButtonText( KDialog::User2, i18n( "Global Project" ) );
        break;
    default:
        break;
    }
}

#define d (KDevConfigPrivate::self())

KDevConfig::KDevConfig()
{}

KDevConfig::~KDevConfig()
{}

KDevConfig::Mode KDevConfig::mode()
{
    return d->mode;
}

// void KDevConfig::setMode( KDevConfig::Mode m )
// {
//     d->setMode( m );
// }

// void KDevConfig::setAllowedModes( KDevConfig::Mode m )
// {
// }

void KDevConfig::settingsDialog()
{
    if ( !d->settingsDialog )
    {
        if ( KDevCore::mainWindow() ->instance() ->instanceName() == "kdevelop" )
            d->settingsDialog = new KSettings::Dialog(
                                    KSettings::Dialog::Static, KDevCore::mainWindow() );
        else
            d->settingsDialog = new KSettings::Dialog( QStringList( "kdevelop" ),
                                KSettings::Dialog::Static, KDevCore::mainWindow() );

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

KConfig *KDevConfig::standard()
{
    return sharedStandard().data();
}

KConfig *KDevConfig::localProject()
{
    return sharedLocalProject().data();
}

KConfig *KDevConfig::globalProject()
{
    return sharedGlobalProject().data();
}

KSharedConfig::Ptr KDevConfig::sharedStandard()
{
    KSharedConfig::Ptr config = KSharedPtr<KSharedConfig>( KGlobal::sharedConfig() );
    QStringList current = config->extraConfigFiles();
    QStringList extraConfig;
    KUrl local = KDevCore::projectController() ->localFile();
    KUrl global = KDevCore::projectController() ->globalFile();
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

KSharedConfig::Ptr KDevConfig::sharedLocalProject()
{
    KSharedConfig::Ptr config = KSharedPtr<KSharedConfig>( KGlobal::sharedConfig() );
    QStringList current = config->extraConfigFiles();
    QStringList extraConfig;
    KUrl local = KDevCore::projectController() ->localFile();
    KUrl global = KDevCore::projectController() ->globalFile();
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

KSharedConfig::Ptr KDevConfig::sharedGlobalProject()
{
    KSharedConfig::Ptr config = KSharedPtr<KSharedConfig>( KGlobal::sharedConfig() );
    QStringList current = config->extraConfigFiles();
    QStringList extraConfig;
    KUrl local = KDevCore::projectController() ->localFile();
    KUrl global = KDevCore::projectController() ->globalFile();
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

#include "kdevconfig.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on

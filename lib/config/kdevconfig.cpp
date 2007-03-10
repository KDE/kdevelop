/* This file is part of KDevelop
Copyright (C) 2006 Adam Treat <treat@kde.org>
Copyright (C) 2007 Andreas Pakulat <apaku@gmx.de>

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
#include <kconfig.h>
#include <kglobal.h>
#include <kcomponentdata.h>

#include <kcmultidialog.h>
#include <ksettings/dialog.h>

#include "icore.h"
#include "iprojectcontroller.h"
#include "iproject.h"

namespace KDevelop
{

Config *Config::m_self = 0;

class ConfigPrivate
{
public:
    Config::Mode mode;
    KSettings::Dialog *settingsDialog;
    ICore* m_core;
    void setMode( Config::Mode m )
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

    void local()
    {
        setMode( Config::LocalProject );
    }
    void shared()
    {
        setMode( Config::GlobalProject );
    }
    void global()
    {
        setMode( Config::Standard );
    }
};

Config::Config( ICore* parent )
    : QObject(parent), d(new ConfigPrivate)
{
    d->m_core = parent;
    d->mode = Config::Standard;
}

void Config::initialize( ICore* core )
{
    if( m_self )
        return;
    m_self = new Config( core );
}

Config* Config::self()
{
    return m_self;
}

Config::~Config()
{}

Config::Mode Config::mode()
{
    return d->mode;
}

void Config::settingsDialog()
{
    if ( !d->settingsDialog )
    {
        d->settingsDialog = new KSettings::Dialog( QStringList( "kdevelop" ),
                                KSettings::Dialog::Static, 0 );

        KCMultiDialog *dialog = d->settingsDialog->dialog();
        dialog->setButtons( KDialog::Help | KDialog::Default | KDialog::Cancel
                            | KDialog::Apply | KDialog::Ok | KDialog::User2 );

        KMenu *m = new KMenu;
        QAction *action;
        action = m->addAction( i18n( "Local Project" ) );
        QObject::connect( action, SIGNAL( triggered() ), this, SLOT( local() ) );
        action = m->addAction( i18n( "Global Project" ) );
        QObject::connect( action, SIGNAL( triggered() ), this, SLOT( shared() ) );
        action = m->addAction( i18n( "Standard" ) );
        QObject::connect( action, SIGNAL( triggered() ), this, SLOT( global() ) );
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
    KUrl local;
    KUrl global;
    if( d->m_core->projectController() )
    {
        IProject* project = d->m_core->projectController()->currentProject();
        if( project )
        {
            local = project->localFile();
            global = project->globalFile();
        }
    }
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
    KUrl local;
    KUrl global;
    if( d->m_core->projectController() )
    {
        IProject* project = d->m_core->projectController()->currentProject();
        if( project )
        {
            local = project->localFile();
            global = project->globalFile();
        }
    }
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
    KUrl local;
    KUrl global;
    QStringList extraConfig;
    if( d->m_core->projectController() )
    {
        IProject* project = d->m_core->projectController()->currentProject();
        if( project )
        {
            local = project->localFile();
            global = project->globalFile();
        }
    }
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

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on

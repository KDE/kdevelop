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

#include "configuration.h"

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

Configuration *Configuration::m_self = 0;

class ConfigurationPrivate
{
public:
    Configuration::Mode mode;
    KSettings::Dialog *settingsDialog;
    ICore* m_core;
    void setMode( Configuration::Mode m )
    {
        mode = m;
        switch ( mode )
        {
        case Configuration::Standard:
            settingsDialog->dialog() ->setButtonText( KDialog::User2, i18n( "Standard" ) );
            break;
        case Configuration::LocalProject:
            settingsDialog->dialog() ->setButtonText( KDialog::User2, i18n( "Local Project" ) );
            break;
        case Configuration::GlobalProject:
            settingsDialog->dialog() ->setButtonText( KDialog::User2, i18n( "Global Project" ) );
            break;
        default:
            break;
        }
    }

    void local()
    {
        setMode( Configuration::LocalProject );
    }
    void shared()
    {
        setMode( Configuration::GlobalProject );
    }
    void global()
    {
        setMode( Configuration::Standard );
    }
};

Configuration::Configuration( ICore* parent )
    : QObject(parent), d(new ConfigurationPrivate)
{
    d->m_core = parent;
    d->settingsDialog = 0;
    d->mode = Configuration::Standard;
}

void Configuration::initialize( ICore* core )
{
    if( m_self )
        return;
    m_self = new Configuration( core );
}

Configuration* Configuration::self()
{
    return m_self;
}

Configuration::~Configuration()
{}

Configuration::Mode Configuration::mode()
{
    return d->mode;
}

void Configuration::settingsDialog()
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

KConfig *Configuration::standard()
{
    return sharedStandard().data();
}

KConfig *Configuration::localProject()
{
    return sharedLocalProject().data();
}

KConfig *Configuration::globalProject()
{
    return sharedGlobalProject().data();
}

KSharedConfig::Ptr Configuration::sharedStandard()
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

KSharedConfig::Ptr Configuration::sharedLocalProject()
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

KSharedConfig::Ptr Configuration::sharedGlobalProject()
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

#include "configuration.moc"

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on

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

#include "kdevlanguagecontroller.h"

#include <kdebug.h>
#include <klocale.h>
#include <kservice.h>
#include <kmessagebox.h>
#include <kmainwindow.h>
#include <kplugininfo.h>
#include <kservicetypetrader.h>

#include "kdevcore.h"
#include "kdevconfig.h"
#include "kdevmainwindow.h"
#include "kdevplugincontroller.h"

namespace KDevelop
{

LanguageController::LanguageController( QObject *parent )
        : QObject( parent ),
        m_activeLanguage( 0 )
{}

LanguageController::~LanguageController()
{}

void LanguageController::loadSettings( bool projectIsLoaded )
{
    if ( projectIsLoaded )
    {
        KConfig * config = Config::standard();
        config->setGroup( "General Options" );
    
        QString language = config->readPathEntry( "PrimaryLanguage", "C++" );
        Core::languageController() ->languageSupport( language );
    }
}

void LanguageController::saveSettings( bool projectIsLoaded )
{
    Q_UNUSED( projectIsLoaded );
}

void LanguageController::initialize()
{}

void LanguageController::cleanup()
{}

LanguageSupport *LanguageController::activeLanguage() const
{
    return m_activeLanguage;
}

LanguageSupport *LanguageController::languageSupport( const QString &language )
{
    if ( language.isEmpty() )
        return 0;

    if ( m_languages.contains( language ) )
        return m_languages[ language ];
    else if ( loadLanguageSupport( language ) )
        return m_languages[ language ];
    else
        return 0;
}

bool LanguageController::loadLanguageSupport( const QString &language )
{
    const QString constraint = QString::fromLatin1("[X-KDevelop-Language] == '%1'").arg( language );

    KPluginInfo::List languageSupportOffers = PluginController::query( "KDevelop/LanguageSupport",
                                                                           constraint );

    if ( languageSupportOffers.isEmpty() )
    {
        KMessageBox::sorry( Core::mainWindow(),
                            i18n( "No language plugin for %1 found.", language ) );
        return false;
    }

    KPluginInfo* languageSupportService = *languageSupportOffers.begin();

    PluginController *pc = PluginController::self();
    Plugin* plugin = pc->loadPlugin( languageSupportService->pluginName() );
    LanguageSupport *langSupport = dynamic_cast<LanguageSupport*>( plugin );

    if ( !langSupport )
    {
        KMessageBox::sorry( Core::mainWindow(),
                            i18n( "Could not create language plugin for %1.", language ) );
        return false;
    }

    m_activeLanguage = langSupport;
    m_languages.insert( language, langSupport );

    return true;
}

}
#include "kdevlanguagecontroller.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on

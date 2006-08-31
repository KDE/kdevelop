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
#include <kservicetypetrader.h>

#include "kdevcore.h"
#include "kdevconfig.h"
#include "kdevmainwindow.h"
#include "kdevplugincontroller.h"

KDevLanguageController::KDevLanguageController( QObject *parent )
        : QObject( parent ),
        m_activeLanguage( 0 )
{}

KDevLanguageController::~KDevLanguageController()
{}

void KDevLanguageController::loadSettings( bool projectIsLoaded )
{
    KConfig * config = KDevConfig::standard();
    config->setGroup( "General Options" );

    QString language = config->readPathEntry( "PrimaryLanguage", "C++" );
    KDevCore::languageController() ->languageSupport( language );
}

void KDevLanguageController::saveSettings( bool projectIsLoaded )
{}

void KDevLanguageController::initialize()
{}

void KDevLanguageController::cleanup()
{}

KDevLanguageSupport *KDevLanguageController::activeLanguage() const
{
    return m_activeLanguage;
}

KDevLanguageSupport *KDevLanguageController::languageSupport( const QString &language )
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

bool KDevLanguageController::loadLanguageSupport( const QString &language )
{
    const QString constraint =
        "[X-KDevelop-Language] == '%1' and [X-KDevelop-Version] == %2";

    KService::List languageSupportOffers =
        KServiceTypeTrader::self() ->query(
            QLatin1String( "KDevelop/LanguageSupport" ),
            constraint.arg( language ).arg( KDEVELOP_PLUGIN_VERSION ) );

    if ( languageSupportOffers.isEmpty() )
    {
        KMessageBox::sorry( KDevCore::mainWindow(),
                            i18n( "No language plugin for %1 found.", language ) );
        return false;
    }

    KService::Ptr languageSupportService = *languageSupportOffers.begin();

    KDevLanguageSupport *langSupport =
        KService::createInstance<KDevLanguageSupport>(
            languageSupportService, 0,
            KDevPluginController::argumentsFromService( languageSupportService ) );

    if ( !langSupport )
    {
        KMessageBox::sorry( KDevCore::mainWindow(),
                            i18n( "Could not create language plugin for %1.", language ) );
        return false;
    }

    m_activeLanguage = langSupport;
    KDevCore::pluginController() ->integratePart( langSupport );

    m_languages.insert( language, langSupport );

    return true;
}

#include "kdevlanguagecontroller.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on

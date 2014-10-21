/* KDevelop Project Settings
*
* Copyright 2006  Matt Rogers <mattr@kde.org>
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
#include "bgpreferences.h"

#include <QVBoxLayout>

#include <interfaces/ilanguagecontroller.h>
#include <language/backgroundparser/backgroundparser.h>

#include "../core.h"

#include "bgconfig.h"

#include "ui_bgsettings.h"

namespace KDevelop
{


BGPreferences::BGPreferences(QWidget* parent)
    : ConfigPage(nullptr, BGSettings::self(), parent)
{
    QVBoxLayout * l = new QVBoxLayout( this );
    QWidget* w = new QWidget;
    preferencesDialog = new Ui::BGSettings;
    preferencesDialog->setupUi( w );

    l->addWidget( w );
}

void BGPreferences::reset()
{
    ConfigPage::reset();

    // stay backwards compatible
    Q_ASSERT(ICore::self()->activeSession());
    KConfigGroup config(ICore::self()->activeSession()->config(), "Background Parser");
    KConfigGroup oldConfig(KSharedConfig::openConfig(), "Background Parser");
#define BACKWARDS_COMPATIBLE_ENTRY(entry, default) \
config.readEntry(entry, oldConfig.readEntry(entry, default))

    preferencesDialog->kcfg_delay->setValue( BACKWARDS_COMPATIBLE_ENTRY("Delay", 500) );
    preferencesDialog->kcfg_threads->setValue( BACKWARDS_COMPATIBLE_ENTRY("Number of Threads", 2) );
    preferencesDialog->kcfg_enable->setChecked( BACKWARDS_COMPATIBLE_ENTRY("Enabled", true) );
}

BGPreferences::~BGPreferences( )
{
    delete preferencesDialog;
}

void BGPreferences::apply()
{
    ConfigPage::apply();

    if ( preferencesDialog->kcfg_enable->isChecked() )
        Core::self()->languageController()->backgroundParser()->enableProcessing();
    else
        Core::self()->languageController()->backgroundParser()->disableProcessing();

    Core::self()->languageController()->backgroundParser()->setDelay( preferencesDialog->kcfg_delay->value() );
    Core::self()->languageController()->backgroundParser()->setThreadCount( preferencesDialog->kcfg_threads->value() );

}

QString BGPreferences::name() const
{
    return i18n("Background Parser");
}

QString BGPreferences::fullName() const
{
    return i18n("Configure Background Parser");
}

QIcon BGPreferences::icon() const
{
    return QIcon::fromTheme(QStringLiteral("code-context"));
}

}

#include "bgpreferences.moc"


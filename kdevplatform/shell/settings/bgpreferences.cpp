/*
    SPDX-FileCopyrightText: 2006 Matt Rogers <mattr@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "bgpreferences.h"

#include <QThread>

#include <interfaces/ilanguagecontroller.h>
#include <language/backgroundparser/backgroundparser.h>

#include "../core.h"

#include "bgconfig.h"

#include "ui_bgpreferences.h"

namespace KDevelop
{


BGPreferences::BGPreferences(QWidget* parent)
    : ConfigPage(nullptr, BGSettings::self(), parent)
{
    preferencesDialog = new Ui::BGPreferences;
    preferencesDialog->setupUi(this);
}

void BGPreferences::reset()
{
    ConfigPage::reset();

    Q_ASSERT(ICore::self()->activeSession());
    KConfigGroup config(ICore::self()->activeSession()->config(), QStringLiteral("Background Parser"));

    preferencesDialog->kcfg_delay->setValue(config.readEntry("Delay", 500));
    preferencesDialog->kcfg_threads->setValue(config.readEntry("Number of Threads", QThread::idealThreadCount()));
    preferencesDialog->kcfg_enable->setChecked(config.readEntry("Enabled", true));
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

    KConfigGroup config(ICore::self()->activeSession()->config(), QStringLiteral("Background Parser"));
    config.writeEntry("Enabled", preferencesDialog->kcfg_enable->isChecked());
    config.writeEntry("Delay", preferencesDialog->kcfg_delay->value());
    config.writeEntry("Number of Threads", preferencesDialog->kcfg_threads->value());
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

#include "moc_bgpreferences.cpp"

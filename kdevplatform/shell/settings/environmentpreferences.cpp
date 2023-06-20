/*
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "environmentpreferences.h"

#include <QVBoxLayout>

#include <KConfigSkeleton>
#include <KLocalizedString>
#include <KSharedConfig>

#include "environmentwidget.h"

namespace KDevelop
{

class EnvironmentPreferencesPrivate
{
public:
    EnvironmentWidget *preferencesDialog;
    KConfigSkeleton* skel;
    QString preselectedProfileName;
};

EnvironmentPreferences::EnvironmentPreferences(const QString& preselectedProfileName, QWidget* parent)
    : ConfigPage(nullptr, nullptr, parent)
    , d_ptr(new EnvironmentPreferencesPrivate)
{
    Q_D(EnvironmentPreferences);

    auto * l = new QVBoxLayout( this );
    l->setContentsMargins(0, 0, 0, 0);
    d->preferencesDialog = new EnvironmentWidget( this );
    l->addWidget( d->preferencesDialog );

    connect(d->preferencesDialog, &EnvironmentWidget::changed,
            this, &EnvironmentPreferences::changed);

    d->skel = new KConfigSkeleton(KSharedConfig::openConfig(), this);
    setConfigSkeleton(d->skel);

    d->preselectedProfileName = preselectedProfileName;
}

EnvironmentPreferences::~EnvironmentPreferences() = default;

void EnvironmentPreferences::apply()
{
    Q_D(EnvironmentPreferences);

    d->preferencesDialog->saveSettings(d->skel->config());
    ConfigPage::apply();
}

void EnvironmentPreferences::reset()
{
    Q_D(EnvironmentPreferences);

    d->preferencesDialog->loadSettings(d->skel->config());
    if (!d->preselectedProfileName.isEmpty()) {
        d->preferencesDialog->selectProfile(d->preselectedProfileName);
    }
    ConfigPage::reset();
}

void EnvironmentPreferences::defaults()
{
    Q_D(EnvironmentPreferences);

    d->preferencesDialog->defaults(d->skel->config());
    ConfigPage::defaults();
}

QString EnvironmentPreferences::name() const
{
    return i18n("Environment");
}

QString EnvironmentPreferences::fullName() const
{
    return i18n("Configure Environment Variables");
}

QIcon EnvironmentPreferences::icon() const
{
    return QIcon::fromTheme(QStringLiteral("utilities-terminal"));
}

}

#include "moc_environmentpreferences.cpp"

/* This file is part of KDevelop
Copyright 2006 Adam Treat <treat@kde.org>

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
#include "environmentpreferences.h"

#include <QVBoxLayout>

#include <KConfigSkeleton>
#include <KConfigGroup>
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
    QString activeGroup;
};

EnvironmentPreferences::EnvironmentPreferences(const QString& activeGroup, QWidget* parent)
    : ConfigPage(nullptr, nullptr, parent), d(new EnvironmentPreferencesPrivate)
{
    QVBoxLayout * l = new QVBoxLayout( this );
    d->preferencesDialog = new EnvironmentWidget( this );
    l->addWidget( d->preferencesDialog );

    connect(d->preferencesDialog, &EnvironmentWidget::changed,
            this, &EnvironmentPreferences::changed);

    d->skel = new KConfigSkeleton(KSharedConfig::openConfig());
    setConfigSkeleton(d->skel);

    d->activeGroup = activeGroup;
}

EnvironmentPreferences::~EnvironmentPreferences( )
{
    delete d;
}

void EnvironmentPreferences::apply()
{
    d->preferencesDialog->saveSettings(d->skel->config());
    ConfigPage::apply();
}

void EnvironmentPreferences::reset()
{
    d->preferencesDialog->loadSettings(d->skel->config());
    d->preferencesDialog->setActiveGroup(d->activeGroup);
    ConfigPage::reset();
}

void EnvironmentPreferences::defaults()
{
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
#include "environmentpreferences.moc"


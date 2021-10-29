/*
    SPDX-FileCopyrightText: 2006 Matt Rogers <mattr@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_PREFERENCES_H
#define KDEVPLATFORM_PLUGIN_PREFERENCES_H

#include <kdevcmodule.h>

namespace Ui
{
class SettingsWidget;
}

class Preferences : public KDevelop::ConfigModule
{
    Q_OBJECT
public:
    Preferences( QWidget *parent, const QStringList &args );
    virtual ~Preferences();

    virtual void save();
    virtual void load();
    virtual void defaults();

private Q_SLOTS:
    void slotSettingsChanged();

private:
    Ui::SettingsWidget *preferencesDialog;

};

#endif

/*
    SPDX-FileCopyrightText: 2010 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later
*/

#ifndef CUSTOMBUILDSYSTEMCONFIGWIDGET_H
#define CUSTOMBUILDSYSTEMCONFIGWIDGET_H

#include <QWidget>
#include "custombuildsystemconfig.h"

class KConfig;
class KConfigGroup;

namespace KDevelop
{
class IProject;
}

namespace Ui
{
class CustomBuildSystemConfigWidget;
}

class CustomBuildSystemConfigWidget : public QWidget
{
Q_OBJECT
public:
    explicit CustomBuildSystemConfigWidget( QWidget* parent );
    void loadFrom( KConfig* );
    void saveTo( KConfig*, KDevelop::IProject* project );
    void loadDefaults();
Q_SIGNALS:
    void changed();
private Q_SLOTS:
    void configChanged();
    void changeCurrentConfig(int);
    void renameCurrentConfig(const QString& name);
    void addConfig();
    void removeConfig();
    void verify();
private:
    void saveConfig( KConfigGroup& grp, const CustomBuildSystemConfig& c, int index );
    Ui::CustomBuildSystemConfigWidget* ui;
    QVector<CustomBuildSystemConfig> configs;
};

#endif

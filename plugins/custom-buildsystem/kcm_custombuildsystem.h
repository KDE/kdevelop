/*
    SPDX-FileCopyrightText: 2010 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later
*/

#ifndef KCM_CUSTOMBUILDSYSTEM_H
#define KCM_CUSTOMBUILDSYSTEM_H

#include <project/projectconfigpage.h>
#include "custombuildsystemplugin.h"
#include "kcfg_custombuildsystemconfig.h"

class CustomBuildSystemSettings;
namespace Ui
{
class CustomBuildSystemConfig;
}

class CustomBuildSystemKCModule : public ProjectConfigPage<CustomBuildSystemSettings>
{
Q_OBJECT
public:
    CustomBuildSystemKCModule(KDevelop::IPlugin* plugin, const KDevelop::ProjectConfigOptions& options, QWidget* parent);
    ~CustomBuildSystemKCModule() override;

    QString name() const override;
    QString fullName() const override;
    QIcon icon() const override;
public Q_SLOTS:
    void defaults() override;
    void reset() override;
    void apply() override;
private:
    class CustomBuildSystemConfigWidget* configWidget;
};

#endif

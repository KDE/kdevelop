/*
    SPDX-FileCopyrightText: 2018 Daniel Mensinger <daniel@mensinger-ka.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "mesonconfig.h"
#include "mintro/mesonoptions.h"

#include <interfaces/configpage.h>

namespace KDevelop
{
class IPlugin;
class IProject;
}

namespace Ui
{
class MesonConfigPage;
}

class MesonConfigPage : public KDevelop::ConfigPage
{
    Q_OBJECT
public:
    explicit MesonConfigPage(KDevelop::IPlugin* plugin, KDevelop::IProject* project, QWidget* parent = nullptr);

    QString name() const override;
    QString fullName() const override;
    QIcon icon() const override;

public Q_SLOTS:
    void apply() override;
    void defaults() override;
    void reset() override;

    void addBuildDir();
    void removeBuildDir();
    void changeBuildDirIndex(int index);
    void emitChanged();

private:
    void checkStatus();
    void updateUI();
    void readUI();
    void writeConfig();
    void setWidgetsDisabled(bool disabled);

private:
    KDevelop::IProject* m_project = nullptr;
    Ui::MesonConfigPage* m_ui = nullptr;
    Meson::MesonConfig m_config;
    Meson::BuildDir m_current;
    bool m_configChanged = false;

    MesonOptsPtr m_options = nullptr;
};

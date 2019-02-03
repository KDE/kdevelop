/* This file is part of KDevelop
    Copyright 2018 Daniel Mensinger <daniel@mensinger-ka.de>

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

#pragma once

#include "mesonconfig.h"
#include "mesonoptions.h"

#include <interfaces/configpage.h>

namespace KDevelop
{
class IPlugin;
class IProject;
}

namespace Ui {
    class MesonConfigPage;
}

class MesonConfigPage : public KDevelop::ConfigPage
{
    Q_OBJECT
public:
    explicit MesonConfigPage(KDevelop::IPlugin* plugin, KDevelop::IProject *project, QWidget* parent = nullptr);

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
    KDevelop::IProject *m_project = nullptr;
    Ui::MesonConfigPage *m_ui = nullptr;
    Meson::MesonConfig m_config;
    Meson::BuildDir m_current;
    bool m_configChanged = false;

    MesonOptsPtr m_options = nullptr;
};

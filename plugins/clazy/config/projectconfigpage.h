/*
    SPDX-FileCopyrightText: 2018 Anton Anikin <anton@anikin.xyz>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVCLAZY_PROJECT_CONFIG_PAGE_H
#define KDEVCLAZY_PROJECT_CONFIG_PAGE_H

// plugin
#include "ui_projectconfigpage.h"
#include "checksetselection.h"
// KDevPlatform
#include <interfaces/configpage.h>
// Qt
#include <QVector>

namespace KDevelop { class IProject; }

namespace Clazy
{

class Plugin;
class CheckSetSelectionManager;
class ProjectSettings;

class ProjectConfigPage : public KDevelop::ConfigPage
{
    Q_OBJECT

public:
    ProjectConfigPage(Plugin* plugin, KDevelop::IProject* project,
                      CheckSetSelectionManager* checkSetSelectionManager,
                      QWidget* parent);
    ~ProjectConfigPage() override;

    QIcon icon() const override;
    QString name() const override;

    void apply() override;
    void defaults() override;
    void reset() override;

private Q_SLOTS:
    void onSelectionChanged(const QString& selection);
    void onChecksChanged(const QString& checks);

    void updateCommandLine();

private:
    Ui::ProjectConfigPage m_ui;

    ProjectSettings* m_settings;

    const QVector<CheckSetSelection> m_checkSetSelections;
    const QString m_defaultCheckSetSelectionId;
};

}

#endif

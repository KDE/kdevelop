/*
    SPDX-FileCopyrightText: 2017 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef DOCKERPREFERENCES_H
#define DOCKERPREFERENCES_H

#include <interfaces/iplugin.h>
#include <interfaces/configpage.h>

namespace Ui { class DockerPreferences; }

class DockerPreferences : public KDevelop::ConfigPage
{
    Q_OBJECT
public:
    explicit DockerPreferences(KDevelop::IPlugin* plugin, KCoreConfigSkeleton* config, QWidget* parent = nullptr);
    ~DockerPreferences() override;

    KDevelop::ConfigPage::ConfigPageType configPageType() const override;
    QString name() const override;

protected:
    bool needsResetDuringInitialization() const override
    {
        return false;
    }

private:
    QScopedPointer<Ui::DockerPreferences> m_prefsUi;
};

#endif

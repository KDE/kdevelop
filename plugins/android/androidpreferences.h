/*
    SPDX-FileCopyrightText: 2017 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef ANDROIDPREFERENCES_H
#define ANDROIDPREFERENCES_H

#include <interfaces/iplugin.h>
#include <interfaces/configpage.h>

namespace Ui { class AndroidPreferences; }

class AndroidPreferences : public KDevelop::ConfigPage
{
    Q_OBJECT
    public:
        explicit AndroidPreferences(KDevelop::IPlugin* plugin, KCoreConfigSkeleton* config, QWidget* parent = nullptr);
        ~AndroidPreferences() override;

        KDevelop::ConfigPage::ConfigPageType configPageType() const override;
        QString name() const override;

    protected:
        bool needsResetDuringInitialization() const override
        {
            return false;
        }

    private:
        QScopedPointer<Ui::AndroidPreferences> m_prefsUi;
};

#endif

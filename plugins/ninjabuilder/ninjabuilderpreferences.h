/*
    SPDX-FileCopyrightText: 2006 Matt Rogers <mattr@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NINJABUILDERPREFERENCES_H
#define NINJABUILDERPREFERENCES_H

#include <project/projectconfigpage.h>

#include "ninjabuilderconfig.h"

class QWidget;
namespace Ui {
class NinjaConfig;
}

class NinjaBuilderPreferences
    : public ProjectConfigPage<NinjaBuilderSettings>
{
    Q_OBJECT

public:
    explicit NinjaBuilderPreferences(KDevelop::IPlugin* plugin, const KDevelop::ProjectConfigOptions& options, QWidget* parent = nullptr);
    ~NinjaBuilderPreferences() override;

    QString name() const override;
    QString fullName() const override;
    QIcon icon() const override;

private:
    Ui::NinjaConfig* m_prefsUi;
};

#endif

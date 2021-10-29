/*
    SPDX-FileCopyrightText: 2006 Matt Rogers <mattr@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MAKEBUILDERPREFERENCES_H
#define MAKEBUILDERPREFERENCES_H

#include <project/projectconfigpage.h>

#include "makebuilderconfig.h"

class QWidget;
namespace Ui { class MakeConfig; }

class MakeBuilderPreferences : public ProjectConfigPage<MakeBuilderSettings>
{
    Q_OBJECT

public:
    explicit MakeBuilderPreferences(KDevelop::IPlugin* plugin, const KDevelop::ProjectConfigOptions& options, QWidget* parent = nullptr);
    ~MakeBuilderPreferences() override;
    void reset() override;
    void apply() override;
    void defaults() override;

    QString name() const override;
    QString fullName() const override;
    QIcon icon() const override;

    static QString standardMakeExecutable();

private:
    Ui::MakeConfig* m_prefsUi;
};

#endif

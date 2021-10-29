/*
    SPDX-FileCopyrightText: 2013 Christoph Thielecke <crissi99@gmx.de>
    SPDX-FileCopyrightText: 2016 Anton Anikin <anton.anikin@htower.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CPPCHECK_PROJECT_CONFIG_PAGE_H
#define CPPCHECK_PROJECT_CONFIG_PAGE_H

#include "parameters.h"

#include <interfaces/configpage.h>

namespace KDevelop
{
class IProject;
}

namespace cppcheck
{

namespace Ui
{
class ProjectConfigPage;
}

class Parameters;

class ProjectConfigPage : public KDevelop::ConfigPage
{
    Q_OBJECT

public:

    ProjectConfigPage(KDevelop::IPlugin* plugin, KDevelop::IProject* project, QWidget* parent);
    ~ProjectConfigPage() override;

    QIcon icon() const override;
    QString name() const override;

public Q_SLOTS:
    void defaults() override;
    void reset() override;

private:
    void updateCommandLine();

    QScopedPointer<Ui::ProjectConfigPage> ui;
    QScopedPointer<Parameters> m_parameters;
};

}

#endif

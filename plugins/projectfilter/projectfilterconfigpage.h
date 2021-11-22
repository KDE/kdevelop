/*
    SPDX-FileCopyrightText: 2013 Milian Wolff <mail@milianw.de>
    SPDX-FileCopyrightText: 2008 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_PROJECTFILTERCONFIGPAGE_H
#define KDEVPLATFORM_PLUGIN_PROJECTFILTERCONFIGPAGE_H

#include <project/projectconfigpage.h>

#include "projectfiltersettings.h"

namespace Ui
{
class ProjectFilterSettings;
}

namespace KDevelop
{

class FilterModel;
class ProjectFilterProvider;

class ProjectFilterConfigPage : public ProjectConfigPage<ProjectFilterSettings>
{
    Q_OBJECT
public:
    ProjectFilterConfigPage(ProjectFilterProvider* provider, const KDevelop::ProjectConfigOptions& options, QWidget* parent);
    ~ProjectFilterConfigPage() override;

    QString name() const override;
    QIcon icon() const override;
    QString fullName() const override;

protected:
    bool eventFilter(QObject* object, QEvent* event) override;

private Q_SLOTS:
    void add();
    void remove();
    void moveUp();
    void moveDown();
    void selectionChanged();
    void checkFiltersAndEmitChanged();

public Q_SLOTS:
    void apply() override;
    void reset() override;
    void defaults() override;

private:
    void checkFilters();

    FilterModel *m_model;
    ProjectFilterProvider* m_projectFilterProvider;
    QScopedPointer<Ui::ProjectFilterSettings> m_ui;
};

}
#endif

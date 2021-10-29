/*
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef PROJECTCONFIGPAGE_H
#define PROJECTCONFIGPAGE_H

#include <interfaces/configpage.h>

namespace KDevelop
{
class IProject;
}

namespace SourceFormatter
{

namespace Ui
{
class ProjectConfigPage;
}

class ProjectConfigPage : public KDevelop::ConfigPage
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    ProjectConfigPage(KDevelop::IPlugin* plugin, KDevelop::IProject* project, QWidget* parent);
    ~ProjectConfigPage() override;

    QString name() const override;
    QString fullName() const override;
    QIcon icon() const override;

    void apply() override;
    void reset() override;
    
private Q_SLOTS:
    void disableCustomSettings(bool checked);

private:
    QScopedPointer<Ui::ProjectConfigPage> ui;
};

}

#endif // PROJECTCONFIGPAGE_H

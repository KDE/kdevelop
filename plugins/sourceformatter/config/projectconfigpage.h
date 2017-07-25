/*
 * This file is part of KDevelop
 * Copyright (C) 2017  Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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

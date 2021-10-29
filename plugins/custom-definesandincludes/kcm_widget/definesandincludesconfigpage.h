/*
    SPDX-FileCopyrightText: 2010 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later
*/

#ifndef KCM_DEFINESANDINCLUDESCONFIGPAGE_H
#define KCM_DEFINESANDINCLUDESCONFIGPAGE_H

#include <project/projectconfigpage.h>

#include "customdefinesandincludes.h"

class DefinesAndIncludesConfigPage : public ProjectConfigPage<CustomDefinesAndIncludes>
{
    Q_OBJECT
public:
    DefinesAndIncludesConfigPage(KDevelop::IPlugin* plugin, const KDevelop::ProjectConfigOptions& options, QWidget* parent);
    ~DefinesAndIncludesConfigPage() override;

    QString name() const override;
    QString fullName() const override;
    QIcon icon() const override;

    void apply() override;
    void reset() override;
private:
    class ProjectPathsWidget* configWidget;
    void loadFrom( KConfig* cfg );
    void saveTo( KConfig* cfg, KDevelop::IProject* );
};

#endif

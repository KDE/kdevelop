/* KDevelop QMake Support
 *
 * Copyright 2007 Andreas Pakulat <apaku@gmx.de>
 * Copyright 2014 Kevin Funk <kfunk@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef QMAKEBUILDERPREFERENCES_H
#define QMAKEBUILDERPREFERENCES_H

#include <project/projectconfigpage.h>

class QWidget;
class QStringList;
class QMakeBuilderSettings;
class QMakeBuildDirChooser;
namespace Ui {
    class QMakeConfig;
    class QMakeBuildDirChooser;
}

/**
 * @author Andreas Pakulat <apaku@gmx.de>
 */
class QMakeBuilderPreferences : public KDevelop::ConfigPage
{
    Q_OBJECT

public:
    explicit QMakeBuilderPreferences(KDevelop::IPlugin* plugin, const KDevelop::ProjectConfigOptions& options, QWidget* parent = 0);
    ~QMakeBuilderPreferences();

public slots:
    virtual void apply() override;
    virtual void reset() override;
    virtual QString name() const override;

    void loadOtherConfig(const QString &config);
    void addBuildConfig();
    void removeBuildConfig();
    void validate();

private:
    bool verifyChanges();

    KDevelop::IProject* m_project;

    Ui::QMakeConfig* m_prefsUi;
    QMakeBuildDirChooser* m_chooserUi;
};

#endif

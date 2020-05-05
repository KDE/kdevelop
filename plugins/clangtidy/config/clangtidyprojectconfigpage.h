/*
 * This file is part of KDevelop
 *
 * Copyright 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
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

#ifndef CLANGTIDY_PROJECTCONFIGPAGE_H
#define CLANGTIDY_PROJECTCONFIGPAGE_H

// plugin
#include "ui_clangtidyprojectconfigpage.h"
#include "checksetselection.h"
// KDevPlatform
#include <interfaces/configpage.h>
// Qt
#include <QVector>


namespace KDevelop
{
class IProject;
}

class ClangTidyProjectSettings;

namespace ClangTidy
{
class CheckSetSelectionManager;
class CheckSet;

/**
 * \class
 * \brief Implements the clang-tidy's configuration project for the current
 * project.
 */
class ProjectConfigPage : public KDevelop::ConfigPage
{
    Q_OBJECT

public:
    ProjectConfigPage(KDevelop::IPlugin* plugin,
                      KDevelop::IProject* project,
                      CheckSetSelectionManager* checkSetSelectionManager,
                      const CheckSet* checkSet,
                      QWidget* parent);
    ~ProjectConfigPage() override;

public: // KDevelop::ConfigPage API
    ConfigPageType configPageType() const override;
    QString name() const override;
    QIcon icon() const override;

    void apply() override;
    void defaults() override;
    void reset() override;

private Q_SLOTS:
    void onSelectionChanged(const QString& selection);
    void onChecksChanged(const QString& checks);

private:
    Ui::ProjectConfigPage m_ui;

    ClangTidyProjectSettings* m_settings;
    KDevelop::IProject* m_project;
    const QVector<CheckSetSelection> m_checkSetSelections;
    const QString m_defaultCheckSetSelectionId;
};

}

#endif /* CLANGTIDY_PROJECTCONFIGPAGE_H_ */

/*
    SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
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

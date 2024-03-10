/*
    SPDX-FileCopyrightText: 2016 Carlos Nihelton <carlosnsoliveira@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CLANGTIDYPREFERENCES_H
#define CLANGTIDYPREFERENCES_H

// KDevPlatform
#include <interfaces/configpage.h>

namespace Ui
{
class ClangTidyPreferences;
}
namespace ClangTidy
{
class CheckSetSelectionManager;
class CheckSet;
}
/**
 * \class
 * \brief Implements the session configuration page for clang-tidy.
 */
class ClangTidyPreferences : public KDevelop::ConfigPage
{
    Q_OBJECT

public:
    explicit ClangTidyPreferences(ClangTidy::CheckSetSelectionManager* checkSetSelectionManager,
                                  const ClangTidy::CheckSet* checkSet,
                                  KDevelop::IPlugin* plugin = nullptr, QWidget* parent = nullptr);
    ~ClangTidyPreferences() override;

public: // KDevelop::ConfigPage API
    ConfigPage::ConfigPageType configPageType() const override;
    QString name() const override;
    QString fullName() const override;
    QIcon icon() const override;

    void apply() override;
    void defaults() override;
    void reset() override;

private:
    void updateJobCountEnabledState();

private:
    Ui::ClangTidyPreferences* ui;
};

#endif

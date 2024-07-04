/*
    SPDX-FileCopyrightText: 2016 Carlos Nihelton <carlosnsoliveira@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CLANGTIDY_PLUGIN_H
#define CLANGTIDY_PLUGIN_H

// plugin
#include "checkset.h"
// KDevPlatform
#include <interfaces/iplugin.h>

namespace ClangTidy
{
class Analyzer;
class CheckSetSelectionManager;

/**
 * \class
 * \brief implements the support for clang-tidy inside KDevelop by using the IPlugin interface.
 */
class Plugin : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    explicit Plugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList& = QVariantList());
    ~Plugin() override;

public: // KDevelop::IPlugin API
    void unload() override;
    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context, QWidget* parent) override;
    int configPages() const override { return 1; }
    /**
     * \function
     * \return the session configuration page for clang-tidy plugin.
     */
    KDevelop::ConfigPage* configPage(int number, QWidget* parent) override;
    int perProjectConfigPages() const override { return 1; }
    /**
     * \function
     * \return the clang-tidy configuration page for the current project.
     */
    KDevelop::ConfigPage* perProjectConfigPage(int number, const KDevelop::ProjectConfigOptions& options,
                                               QWidget* parent) override;

public:
    /**
     *\function
     *\returns all available checks, obtained from clang-tidy program, ran with "--checks=* --list-checks"
     * parameters.
     */
    QStringList allAvailableChecks() { return m_checkSet.all(); }
    CheckSet& checkSet() { return m_checkSet; }

private:
    Analyzer* m_analyzer;
    CheckSet m_checkSet;
    CheckSetSelectionManager* m_checkSetSelectionManager;
};

} // namespace ClangTidy

#endif // CLANGTIDY_PLUGIN_H

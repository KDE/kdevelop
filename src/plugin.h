/*
 * This file is part of KDevelop
 *
 * Copyright 2016 Carlos Nihelton <carlosnsoliveira@gmail.com>
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

#ifndef CLANGTIDY_PLUGIN_H
#define CLANGTIDY_PLUGIN_H

// plugin
#include "checkset.h"
#include <debug.h>
// KDevPlatform
#include <interfaces/iplugin.h>
// Qt
#include <QPointer>
#include <QVariant>

class KJob;
class QAction;

namespace ClangTidy
{

class ProblemModel;

/**
 * \class
 * \brief implements the support for clang-tidy inside KDevelop by using the IPlugin interface.
 */
class Plugin : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    explicit Plugin(QObject* parent, const QVariantList& = QVariantList());
    ~Plugin();

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
     *\returns all available checks, obtained from clang-tidy program, ran with with "--checks=* --list-checks"
     * parameters.
     */
    QStringList allAvailableChecks() { return m_checkSet.all(); }
    bool isRunning() const;
    void runClangTidy(const QUrl& url, bool allFiles = false);

private Q_SLOTS:
    void runClangTidy(bool allFiles = false);
    void runClangTidyFile();
    void runClangTidyAll();
    void result(KJob* job);
    void updateActions();

private:
    void showModel();

private:
    QPointer<KJob> m_runningJob;

    QAction* m_checkFileAction;
    QAction* m_checkProjectAction;
    QScopedPointer<ProblemModel> m_model;
    CheckSet m_checkSet;
};

} // namespace ClangTidy

#endif // CLANGTIDY_PLUGIN_H

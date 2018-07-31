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

// Qt
#include <QPointer>
#include <QVariant>
// KDevPlatform
#include <interfaces/iplugin.h>
// plugin
#include "config/configgroup.h"
#include "debug.h"

class KJob;
class QAction;

namespace KDevelop
{
class ProblemModel;
}

namespace ClangTidy
{
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
    /**
     *\function
     *\returns all available checks, obtained from clang-tidy program, ran with with "--checks=* --list-checks"
     * parameters.
     */
    QStringList allAvailableChecks() { return m_allChecks; }

protected:
    /**
     * \function
     * \brief collects all available checks by running clang-tidy with the following parameters: "--checks=*
     * --list-checks".
     * \param clangtidyPath QString - the system path for the clang-tidy program.
     */
    void collectAllAvailableChecks(const QString& clangtidyPath);

private Q_SLOTS:
    void runClangTidy(bool allFiles = false);
    void runClangTidy(const QUrl& url, bool allFiles = false);
    void runClangTidyFile();
    void runClangTidyAll();
    void result(KJob* job);
    void updateActions();

private:
    bool isRunning() const;

private:
    QPointer<KJob> m_runningJob;

    QAction* m_checkFileAction;
    ConfigGroup m_config;
    QScopedPointer<KDevelop::ProblemModel> m_model;
    QStringList m_allChecks;
    QStringList m_activeChecks;
};

} // namespace ClangTidy

#endif // CLANGTIDY_PLUGIN_H

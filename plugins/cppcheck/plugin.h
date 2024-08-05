/*
    SPDX-FileCopyrightText: 2013 Christoph Thielecke <crissi99@gmx.de>
    SPDX-FileCopyrightText: 2016-2017 Anton Anikin <anton.anikin@htower.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CPPCHECK_PLUGIN_H
#define CPPCHECK_PLUGIN_H

#include "job.h"

#include <interfaces/iplugin.h>
#include <QScopedPointer>

class KJob;

namespace KDevelop
{
class IProject;
}

namespace cppcheck
{

class ProblemModel;

class Plugin : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    explicit Plugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList& = QVariantList());

    ~Plugin() override;

    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context, QWidget* parent) override;

    int configPages() const override { return 1; }
    KDevelop::ConfigPage* configPage(int number, QWidget* parent) override;

    int perProjectConfigPages() const override { return 1; }
    KDevelop::ConfigPage* perProjectConfigPage(int number, const KDevelop::ProjectConfigOptions& options, QWidget* parent) override;

    void runCppcheck(KDevelop::IProject* project, const QString& path);
    bool isRunning();

private:
    void killCppcheck();

    void raiseProblemsView();
    void raiseOutputView();

    void updateActions();
    void projectClosed(KDevelop::IProject* project);

    void runCppcheck(bool checkProject);

    void result(KJob* job);

    Job* m_job;

    KDevelop::IProject* m_currentProject;

    QAction* m_menuActionFile;
    QAction* m_menuActionProject;
    QAction* m_contextActionFile;
    QAction* m_contextActionProject;
    QAction* m_contextActionProjectItem;

    QScopedPointer<ProblemModel> m_model;
};

}

#endif

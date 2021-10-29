/*
    SPDX-FileCopyrightText: 2018 Anton Anikin <anton@anikin.xyz>
    SPDX-FileCopyrightText: 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COMPILEANALYZER_COMPILEANALYZER_H
#define COMPILEANALYZER_COMPILEANALYZER_H

// lib
#include <compileanalyzercommonexport.h>
// KDevPlatform
#include <shell/problemmodel.h>
// Qt
#include <QObject>
#include <QIcon>

class KJob;
class QAction;
class QWidget;

namespace KDevelop
{
class IPlugin;
class ICore;
class IProject;
class ContextMenuExtension;
class Context;
class Path;
class CompileAnalyzeProblemModel;
class CompileAnalyzeJob;

class KDEVCOMPILEANALYZERCOMMON_EXPORT CompileAnalyzer : public QObject
{
    Q_OBJECT

public:
    CompileAnalyzer(IPlugin* plugin,
                    const QString& toolName, const QString& toolIconName,
                    const QString& fileActionId, const QString& allActionId,
                    const QString& modelId,
                    ProblemModel::Features modelFeatures,
                    QObject* parent);
    ~CompileAnalyzer() override;

public:
    void fillContextMenuExtension(ContextMenuExtension &extension,
                                  Context* context, QWidget* parent);

protected: // API to implement
    virtual CompileAnalyzeJob* createJob(KDevelop::IProject* project, const KDevelop::Path& buildDirectory,
                                         const QUrl& url, const QStringList& filePaths) = 0;
    virtual bool isOutputToolViewPreferred() const;

private:
    void raiseProblemsToolView();
    void raiseOutputToolView();
    void killJob();

    bool isRunning() const;
    ICore* core() const;

private Q_SLOTS:
    void runTool(const QUrl& url, bool allFiles = false);
    void runTool(bool allFiles = false);
    void runToolOnFile();
    void runToolOnAll();

    void handleRerunRequest(const QUrl& url, bool allFiles);
    void result(KJob* job);
    void updateActions();
    void handleProjectClosed(KDevelop::IProject* project);

private:
    ICore* const m_core;
    const QString m_toolName;
    const QIcon m_toolIcon;
    const QString m_modelId;

    CompileAnalyzeProblemModel* m_model;

    CompileAnalyzeJob* m_job = nullptr;

    QAction* m_checkFileAction;
    QAction* m_checkProjectAction;
};

}

#endif

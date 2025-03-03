/*
    SPDX-FileCopyrightText: 2018, 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COMPILEANALYZER_COMPILEANALYZEPROBLEMMODEL_H
#define COMPILEANALYZER_COMPILEANALYZEPROBLEMMODEL_H

// KDevPlatfrom
#include <shell/problemmodel.h>
// Qt
#include <QUrl>

namespace KDevelop { class IProject; }

namespace KDevelop
{

class CompileAnalyzeProblemModel : public KDevelop::ProblemModel
{
    Q_OBJECT

public:
    explicit CompileAnalyzeProblemModel(const QString& toolName, QObject* parent);
    ~CompileAnalyzeProblemModel() override;

public: // KDevelop::ProblemModel API
    void forceFullUpdate() override;

public:
    void addProblems(const QVector<KDevelop::IProblem::Ptr>& problems);

    void finishAddProblems(bool jobSucceeded);

    void reset();
    void reset(KDevelop::IProject* project, const QUrl& path, bool allFiles);

    KDevelop::IProject* project() const;

Q_SIGNALS:
    void rerunRequested(const QUrl& path, bool allFiles);

private:
    void setMessage(const QString& message);
    bool problemExists(KDevelop::IProblem::Ptr newProblem);

private:
    const QString m_toolName;
    KDevelop::IProject* m_project = nullptr;
    QUrl m_path;
    bool m_allFiles = false;
    KDevelop::DocumentRange m_pathLocation;

    QVector<KDevelop::IProblem::Ptr> m_problems;
    int m_maxProblemDescriptionLength = 0;
};

}

#endif

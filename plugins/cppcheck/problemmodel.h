/*
    SPDX-FileCopyrightText: 2017 Anton Anikin <anton.anikin@htower.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <shell/problemmodel.h>

namespace KDevelop
{
    class IProject;
}

namespace cppcheck
{

class Plugin;

class ProblemModel : public KDevelop::ProblemModel
{
    Q_OBJECT
public:
    explicit ProblemModel(Plugin* plugin);
    ~ProblemModel() override;

    KDevelop::IProject* project() const;

    void addProblems(const QVector<KDevelop::IProblem::Ptr>& problems);

    void setProblems(bool jobSucceeded);

    void reset();
    void reset(KDevelop::IProject* project, const QString& path);

    void show();

    void forceFullUpdate() override;

private:
    void fixProblemFinalLocation(KDevelop::IProblem::Ptr problem);
    bool problemExists(KDevelop::IProblem::Ptr newProblem);
    void setMessage(const QString& message);

    using KDevelop::ProblemModel::setProblems;

    Plugin* m_plugin;

    KDevelop::IProject* m_project;

    QString m_path;
    KDevelop::DocumentRange m_pathLocation;

    QVector<KDevelop::IProblem::Ptr> m_problems;
};

}

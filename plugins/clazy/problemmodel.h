/* This file is part of KDevelop

   Copyright 2018 Anton Anikin <anton@anikin.xyz>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KDEVCLAZY_PROBLEM_MODEL_H
#define KDEVCLAZY_PROBLEM_MODEL_H

#include <shell/problemmodel.h>

namespace KDevelop { class IProject; }

namespace Clazy
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

    void setProblems();
    using KDevelop::ProblemModel::setProblems;

    void reset();
    void reset(KDevelop::IProject* project, const QString& path);

    void show();

    void forceFullUpdate() override;

private:
    void setMessage(const QString& message);
    bool problemExists(KDevelop::IProblem::Ptr newProblem);

private:
    Plugin* m_plugin;

    KDevelop::IProject* m_project;

    QString m_path;
    KDevelop::DocumentRange m_pathLocation;

    QVector<KDevelop::IProblem::Ptr> m_problems;
};

}

#endif

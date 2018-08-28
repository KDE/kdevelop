/*
 * This file is part of KDevelop
 *
 * Copyright 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
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

#ifndef CLANGTIDY_PROBLEMMODEL_H
#define CLANGTIDY_PROBLEMMODEL_H

// KDevPlatfrom
#include <shell/problemmodel.h>

namespace KDevelop { class IProject; }

namespace ClangTidy
{

class Plugin;

class ProblemModel : public KDevelop::ProblemModel
{
    Q_OBJECT

public:
    explicit ProblemModel(Plugin* plugin, QObject* parent);
    ~ProblemModel() override;

public: // KDevelop::ProblemModel API
    void forceFullUpdate() override;

public:
    void addProblems(const QVector<KDevelop::IProblem::Ptr>& problems);

    void finishAddProblems();

    void reset(KDevelop::IProject* project, const QUrl& url, bool allFiles);

private:
    void setMessage(const QString& message);
    bool problemExists(KDevelop::IProblem::Ptr newProblem);

private:
    Plugin* const m_plugin;
    QUrl m_url;
    bool m_allFiles = false;

    QVector<KDevelop::IProblem::Ptr> m_problems;
    int m_maxProblemDescriptionLength = 0;
};

}

#endif

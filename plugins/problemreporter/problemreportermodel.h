/*
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2015 Laszlo Kis-Adam <laszlo.kis-adam@kdemail.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef PROBLEMREPORTERMODEL_H
#define PROBLEMREPORTERMODEL_H

#include <shell/problemmodel.h>

namespace KDevelop
{
class IndexedString;
class TopDUContext;
}

class WatchedDocumentSet;
class QTimer;

/**
 * @brief ProblemModel subclass that retrieves the problems from DUChain.
 *
 * Provides a ProblemModel interface so these problems can be shown in the Problems toolview.
 */
class ProblemReporterModel : public KDevelop::ProblemModel
{
    Q_OBJECT
public:
    explicit ProblemReporterModel(QObject* parent);
    ~ProblemReporterModel() override;

    /**
     * Get problems for @ref url.
     */
    QVector<KDevelop::IProblem::Ptr> problems(const KDevelop::IndexedString& url, bool showImports) const;
    /**
     * Get merged list of problems for all @ref urls.
     */
    QVector<KDevelop::IProblem::Ptr> problems(const QSet<KDevelop::IndexedString>& urls, bool showImports) const;

public Q_SLOTS:
    /**
     * List of problems for @ref url has been updated
     */
    void problemsUpdated(const KDevelop::IndexedString& url);

    void setShowImports(bool showImports) override;
    void forceFullUpdate() override;

protected Q_SLOTS:
    /// Triggered when the problemstore's problems have changed
    void onProblemsChanged() override;

private Q_SLOTS:
    void timerExpired();
    void setCurrentDocument(KDevelop::IDocument* doc) override;

private:
    void problemsInternal(KDevelop::TopDUContext* context, bool showImports,
                          QSet<KDevelop::TopDUContext*>& visitedContexts,
                          QVector<KDevelop::IProblem::Ptr>& result) const;
    void rebuildProblemList();

    bool m_showImports; /// include problems from imported documents
    QTimer* m_minTimer;
    QTimer* m_maxTimer;
    const static int MinTimeout;
    const static int MaxTimeout;
};

#endif

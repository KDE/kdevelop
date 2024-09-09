/*
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2015 Laszlo Kis-Adam <laszlo.kis-adam@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
 * Provides a ProblemModel interface so these problems can be shown in the Problems tool view.
 */
class ProblemReporterModel : public KDevelop::ProblemModel
{
    Q_OBJECT
public:
    explicit ProblemReporterModel(QObject* parent);
    ~ProblemReporterModel() override;

    /**
     * Get merged list of problems for all @ref urls.
     */
    QVector<KDevelop::IProblem::Ptr> problems(const QSet<KDevelop::IndexedString>& urls) const;

public Q_SLOTS:
    /**
     * List of problems for @ref url has been updated
     */
    void problemsUpdated(const KDevelop::IndexedString& url);

    void forceFullUpdate() override;

protected Q_SLOTS:
    /// Triggered when the problemstore's problems have changed
    void onProblemsChanged() override;

private Q_SLOTS:
    void setCurrentDocument(KDevelop::IDocument* doc) override;

private:
    void rebuildProblemList();
};

#endif

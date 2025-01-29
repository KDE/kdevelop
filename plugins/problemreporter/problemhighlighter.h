/*
    SPDX-FileCopyrightText: 2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2008-2009 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_PROBLEMHIGHLIGHTER_H
#define KDEVPLATFORM_PLUGIN_PROBLEMHIGHLIGHTER_H

#include <language/duchain/problem.h>
#include <interfaces/iproblem.h>

#include <KTextEditor/MovingRange>

#include <QPointer>

class ProblemHighlighter : public QObject
{
    Q_OBJECT
public:
    explicit ProblemHighlighter(KTextEditor::Document* document);
    ~ProblemHighlighter() override;

    KTextEditor::Document* document() const
    {
        return m_document;
    }

    void setProblems(const QVector<KDevelop::IProblem::Ptr>& problems);

private Q_SLOTS:
    void aboutToRemoveText(const KTextEditor::Range& range);
    void clearProblems();

private:
    void settingsChanged();

    QPointer<KTextEditor::Document> m_document;
    QList<KTextEditor::MovingRange*> m_topHLRanges;
    QVector<KDevelop::IProblem::Ptr> m_problems;
};

#endif // KDEVPLATFORM_PLUGIN_PROBLEMHIGHLIGHTER_H

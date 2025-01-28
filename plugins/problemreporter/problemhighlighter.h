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

    /**
     * Store given problems and re-highlight unless the currently stored problems are equal to @p problems.
     */
    void setProblems(const QVector<KDevelop::IProblem::Ptr>& problems);

private Q_SLOTS:
    void aboutToRemoveText(const KTextEditor::Range& range);
    void clearProblems();

private:
    /**
     * Stores the few Language Support settings that affect problem highlighting.
     */
    struct Settings
    {
        bool highlightSemanticProblems;
        bool highlightProblematicLines;

        [[nodiscard]] bool operator==(const Settings& other) const noexcept;
    };
    /**
     * Read Settings from config and return them.
     */
    [[nodiscard]] static Settings readSettings();

    void settingsChanged();

    /**
     * Store given problems and re-highlight.
     */
    void forceSetProblems(const QList<KDevelop::IProblem::Ptr>& problems);

    QPointer<KTextEditor::Document> m_document;
    QList<KTextEditor::MovingRange*> m_topHLRanges;
    QVector<KDevelop::IProblem::Ptr> m_problems;

    Settings m_currentSettings = readSettings();
};

#endif // KDEVPLATFORM_PLUGIN_PROBLEMHIGHLIGHTER_H

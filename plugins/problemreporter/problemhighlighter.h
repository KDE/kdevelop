/*
    SPDX-FileCopyrightText: 2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2008-2009 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_PROBLEMHIGHLIGHTER_H
#define KDEVPLATFORM_PLUGIN_PROBLEMHIGHLIGHTER_H

#include <interfaces/iproblem.h>

#include <QObject>

#include <memory>
#include <vector>

namespace KTextEditor {
class Document;
class MovingRange;
class Range;
}

class ProblemHighlighter : public QObject
{
    Q_OBJECT
public:
    /**
     * Construct a problem highlighter.
     *
     * @param document a non-null document to be highlighted that must
     *        remain valid throughout this highlighter's lifetime.
     */
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
    using MovingRangePtr = std::unique_ptr<KTextEditor::MovingRange>;

    KTextEditor::Document* const m_document;
    std::vector<MovingRangePtr> m_topHLRanges;
    QVector<KDevelop::IProblem::Ptr> m_problems;

public Q_SLOTS:
    void settingsChanged();
};

#endif // KDEVPLATFORM_PLUGIN_PROBLEMHIGHLIGHTER_H

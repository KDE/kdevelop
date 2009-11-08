/*
 * KDevelop Problem Reporter
 *
 * Copyright 2008 Hamish Rodda <rodda@kde.org>
 * Copyright 2008-2009 David Nolden <david.nolden.kdevelop@art-master.de>
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

#ifndef PROBLEM_HIGHLIGHT_H
#define PROBLEM_HIGHLIGHT_H

#include <KTextEditor/SmartRangeWatcher>

#include <language/interfaces/iproblem.h>
#include <qpointer.h>

class ProblemHighlighter : public QObject, KTextEditor::SmartRangeWatcher
{
    Q_OBJECT
public:
    ProblemHighlighter(KTextEditor::Document* document);
    virtual ~ProblemHighlighter();

    void setProblems(const QList<KDevelop::ProblemPointer>& problems);

private slots:
    void viewCreated(KTextEditor::Document*,KTextEditor::View*);
    void textHintRequested(const KTextEditor::Cursor&, QString&);
private:
    /// Detect and respond to a highlighted range being deleted
    virtual void rangeDeleted(KTextEditor::SmartRange *range);
    /// Hide the error when the text is changed, it's likely to be different on the next parse
    virtual void rangeContentsChanged(KTextEditor::SmartRange* range);

    void clearHighlights();

    QPointer<KTextEditor::Document> m_document;
    QList<KTextEditor::SmartRange*> m_topHLRanges;
    QList<KDevelop::ProblemPointer> m_problems;
    QMap<KTextEditor::SmartRange*, KDevelop::ProblemPointer> m_problemsForRanges;
public slots:
    void settingsChanged();
};

#endif // PROBLEM_HIGHLIGHT_H

/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_PROBLEMINLINENOTEPROVIDER_H
#define KDEVPLATFORM_PLUGIN_PROBLEMINLINENOTEPROVIDER_H

#include <interfaces/iproblem.h>
#include <interfaces/icompletionsettings.h>

#include <KTextEditor/InlineNoteProvider>

#include <QHash>
#include <QPointer>
#include <QVector>

namespace KTextEditor {
class Document;
}

class ProblemInlineNoteProvider : public KTextEditor::InlineNoteProvider
{
    Q_OBJECT
public:
    explicit ProblemInlineNoteProvider(KTextEditor::Document* document);
    ~ProblemInlineNoteProvider();

    /**
     * Store given problems and recreate inline notes unless the currently stored problems are equal to @p problems.
     */
    void setProblems(const QVector<KDevelop::IProblem::Ptr>& problems);

    QVector<int> inlineNotes(int line) const override;
    QSize inlineNoteSize(const KTextEditor::InlineNote& note) const override;
    void paintInlineNote(const KTextEditor::InlineNote& note, QPainter& painter,
                         Qt::LayoutDirection direction) const override;

private:
    void completionSettingsChanged();

    /**
     * Store given problems and recreate inline notes.
     */
    void forceSetProblems(const QList<KDevelop::IProblem::Ptr>& problems);

    QPointer<KTextEditor::Document> m_document;
    QVector<KDevelop::IProblem::Ptr> m_problems;
    QHash<int, KDevelop::IProblem::Ptr> m_problemForLine;
    KDevelop::ICompletionSettings::ProblemInlineNotesLevel m_currentLevel;
};
#endif

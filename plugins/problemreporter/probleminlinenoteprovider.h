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
#include <QVector>

namespace KTextEditor {
class Document;
}

class ProblemInlineNoteProvider : public KTextEditor::InlineNoteProvider
{
    Q_OBJECT
public:
    /**
     * Construct a problem inline note provider.
     *
     * @param document a non-null document to be annotated that must remain valid throughout this provider's lifetime.
     */
    explicit ProblemInlineNoteProvider(KTextEditor::Document* document);
    ~ProblemInlineNoteProvider();

    void setProblems(const QVector<KDevelop::IProblem::Ptr>& problems);

    QVector<int> inlineNotes(int line) const override;
    QSize inlineNoteSize(const KTextEditor::InlineNote& note) const override;
    void paintInlineNote(const KTextEditor::InlineNote& note, QPainter& painter,
                         Qt::LayoutDirection direction) const override;

private:
    void completionSettingsChanged();

    KTextEditor::Document* const m_document;
    QVector<KDevelop::IProblem::Ptr> m_problems;
    QHash<int, KDevelop::IProblem::Ptr> m_problemForLine;
    KDevelop::ICompletionSettings::ProblemInlineNotesLevel m_currentLevel;
};
#endif

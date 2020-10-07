/*
 * InlineNoteProvider
 *
 * Copyright 2020 David Redondo <kde@david-redondo.de>
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

    void setProblems(const QVector<KDevelop::IProblem::Ptr>& problems);

    QVector<int> inlineNotes(int line) const override;
    QSize inlineNoteSize(const KTextEditor::InlineNote& note) const override;
    void paintInlineNote(const KTextEditor::InlineNote& note, QPainter& painter) const override;
private:
    void completionSettingsChanged();

    QPointer<KTextEditor::Document> m_document;
    QVector<KDevelop::IProblem::Ptr> m_problems;
    QHash<int, KDevelop::IProblem::Ptr> m_problemForLine;
    KDevelop::ICompletionSettings::ProblemInlineNotesLevel m_currentLevel;
};
#endif

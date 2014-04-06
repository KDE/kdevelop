/*
 * Copyright 2014 Kevin Funk <kfunk@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "clangproblem.h"

#include <language/duchain/duchainlock.h>
#include <language/codegen/documentchangeset.h>

#include <KDebug>
#include <KLocale>

using namespace KDevelop;

KSharedPtr<IAssistant> ClangProblem::solutionAssistant() const
{
    if (allFixits().isEmpty()) {
        return {};
    }

    return KSharedPtr<IAssistant>(new ClangFixitAssistant(allFixits()));
}

ClangFixits ClangProblem::fixits() const
{
    return m_fixits;
}

void ClangProblem::setFixits(const ClangFixits& fixits)
{
    m_fixits = fixits;
}

ClangFixits ClangProblem::allFixits() const
{
    ClangFixits result;
    result << m_fixits;

    for (const ProblemPointer& diagnostic : diagnostics()) {
        const Ptr problem = Ptr::staticCast(diagnostic);
        Q_ASSERT(problem);
        result << problem->allFixits();
    }
    return result;
}

ClangFixitAssistant::ClangFixitAssistant(const ClangFixits& fixits)
    : m_fixits(fixits)
{
}

void ClangFixitAssistant::createActions()
{
    KDevelop::IAssistant::createActions();

    for (const ClangFixit& fixit : m_fixits) {
        addAction(IAssistantAction::Ptr(new ClangFixitAction(fixit)));
    }
}

ClangFixitAction::ClangFixitAction(const ClangFixit& fixit)
    : m_fixit(fixit)
{
}

QString ClangFixitAction::description() const
{
    if (!m_fixit.description.isEmpty())
        return m_fixit.description;

    // fallback in case there's no hint for the diagnostic
    // Make sure we don't break on a replacementText such as '#include <foobar>'
    auto formattedReplacement = m_fixit.replacementText;
    formattedReplacement.replace("<", "&amp;lt;").replace(">", "&amp;gt;");

    const auto range = m_fixit.range;
    if (range.start == range.end) {
        return i18n("Insert \"%1\" at line: %2, column: %3",
                    formattedReplacement, range.start.line, range.start.column);
    } else if (range.start.line == range.end.line) {
        return i18n("Replace text at line: %1, column: %2 with: \"%3\"",
                    range.start.line, range.start.column, formattedReplacement);
    } else {
        return i18n("Replace multiple lines starting at line: %1, column: %2 with: \"%3\"",
                    range.start.line, range.start.column, formattedReplacement);
    }
}

void ClangFixitAction::execute()
{
    DocumentChangeSet changes;
    {
        DUChainReadLocker lock;

        DocumentChange change(m_fixit.range.document, m_fixit.range,
                    QString(), m_fixit.replacementText);
        // TODO: We probably don't want this
        change.m_ignoreOldText = true;
        changes.addChange(change);
    }

    changes.setReplacementPolicy(DocumentChangeSet::WarnOnFailedChange);
    changes.applyAllChanges();
    emit executed(this);
}

#include "clangproblem.moc"

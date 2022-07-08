/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_COMPLETIONSETTINGS_H
#define KDEVPLATFORM_COMPLETIONSETTINGS_H

#include <interfaces/icompletionsettings.h>
#include <KConfigGroup>

namespace KDevelop
{

class CompletionSettings : public KDevelop::ICompletionSettings
{
    Q_OBJECT
public:
    CompletionLevel completionLevel() const override;

    bool automaticCompletionEnabled() const override;

    void emitChanged() { emit settingsChanged(this); }

    int localColorizationLevel() const override;

    int globalColorizationLevel() const override;

    GlobalColorSource globalColorSource() const override;

    bool highlightSemanticProblems() const override;

    bool highlightProblematicLines() const override;

    ProblemInlineNotesLevel problemInlineNotesLevel() const override;

    bool boldDeclarations() const override;

    bool showMultiLineSelectionInformation() const override;

    int minFilesForSimplifiedParsing() const override;

    QStringList todoMarkerWords() const override;

    static CompletionSettings& self();

private:
    CompletionSettings();

    const CompletionLevel m_level = MinimalWhenAutomatic;
    const bool m_automatic = true;
    const bool m_highlightSemanticProblems = true;
    const bool m_highlightProblematicLines = false;
    const bool m_showMultiLineInformation = false;
    const bool m_boldDeclarations = true;
    const int m_localColorizationLevel = 170;
    const int m_globalColorizationLevel = 255;
    const GlobalColorSource m_globalColorSource = GlobalColorSource::FromTheme;
    const int m_minFilesForSimplifiedParsing = 100000;
    const QString m_todoMarkerWords;

    const KConfigGroup m_languageGroup;
};
}
#endif

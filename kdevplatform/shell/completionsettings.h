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

    PrecompiledPreambleStorage precompiledPreambleStorage() const override;

    static CompletionSettings& self();

private:
    CompletionSettings() = default;
};
}
#endif

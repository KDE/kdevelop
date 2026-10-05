/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2013 Vlas Puhov <vlas.puhov@mail.ru>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "completionsettings.h"
#include "languageconfig.h"

#include <KShell>
#include <KSharedConfig>

#include <QMetaEnum>

namespace KDevelop
{

CompletionSettings& CompletionSettings::self()
{
    static CompletionSettings settings;
    return settings;
}

QStringList CompletionSettings::todoMarkerWords() const
{
    // TODO store a QStringList directly in the config. This requires migration, probably using kconf_update.
    return KShell::splitArgs(LanguageConfig::todoMarkerWords());
}

int CompletionSettings::minFilesForSimplifiedParsing() const
{
    return LanguageConfig::minFilesForSimplifiedParsing();
}

bool CompletionSettings::showMultiLineSelectionInformation() const
{
    return LanguageConfig::showMultiLineSelectionInformation();
}

bool CompletionSettings::highlightProblematicLines() const
{
    return LanguageConfig::highlightProblematicLines();
}

bool CompletionSettings::highlightSemanticProblems() const
{
    return LanguageConfig::highlightSemanticProblems();
}

ICompletionSettings::ProblemInlineNotesLevel CompletionSettings::problemInlineNotesLevel() const
{
    return LanguageConfig::problemInlineNotesLevel();
}

bool CompletionSettings::boldDeclarations() const
{
    return LanguageConfig::boldDeclarations();
}

int CompletionSettings::globalColorizationLevel() const
{
    return LanguageConfig::globalColorization();
}

CompletionSettings::GlobalColorSource CompletionSettings::globalColorSource() const
{
    return LanguageConfig::globalColorSource();
}

int CompletionSettings::localColorizationLevel() const
{
    return LanguageConfig::localColorization();
}

bool CompletionSettings::automaticCompletionEnabled() const
{
    return LanguageConfig::automaticInvocation();
}

ICompletionSettings::CompletionLevel CompletionSettings::completionLevel() const
{
    return LanguageConfig::completionDetail();
}

auto CompletionSettings::precompiledPreambleStorage() const -> PrecompiledPreambleStorage
{
    return LanguageConfig::precompiledPreambleStorage();
}

CompletionSettings::CompletionSettings()
{
    connect(LanguageConfig::self(), &LanguageConfig::configChanged, this, &ICompletionSettings::settingsChanged);
}
}

#include "moc_completionsettings.cpp"

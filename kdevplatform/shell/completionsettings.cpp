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

static QString completionLevelToString(ICompletionSettings::CompletionLevel l)
{
    if (l < 0 || l >= ICompletionSettings::LAST_LEVEL) {
        return QString();
    }

    const static QString levels[ICompletionSettings::LAST_LEVEL] = {QStringLiteral("Minimal"), QStringLiteral("MinimalWhenAutomatic"), QStringLiteral("AlwaysFull")};
    return levels[l];
}

CompletionSettings& CompletionSettings::self()
{
    static CompletionSettings settings;
    return settings;
}

QStringList CompletionSettings::todoMarkerWords() const
{
    const QString markers = m_languageGroup.readEntry("todoMarkerWords", m_todoMarkerWords);
    return KShell::splitArgs(markers);
}

int CompletionSettings::minFilesForSimplifiedParsing() const
{
    return m_languageGroup.readEntry("minFilesForSimplifiedParsing", m_minFilesForSimplifiedParsing);
}

bool CompletionSettings::showMultiLineSelectionInformation() const
{
    return m_languageGroup.readEntry("showMultiLineSelectionInformation", m_showMultiLineInformation);
}

bool CompletionSettings::highlightProblematicLines() const
{
    return m_languageGroup.readEntry("highlightProblematicLines", m_highlightProblematicLines);
}

bool CompletionSettings::highlightSemanticProblems() const
{
    return m_languageGroup.readEntry("highlightSemanticProblems", m_highlightSemanticProblems);
}

ICompletionSettings::ProblemInlineNotesLevel CompletionSettings::problemInlineNotesLevel() const
{
    return LanguageConfig::problemInlineNotesLevel();
}

bool CompletionSettings::boldDeclarations() const
{
    return m_languageGroup.readEntry("boldDeclarations", m_boldDeclarations);
}

int CompletionSettings::globalColorizationLevel() const
{
    return m_languageGroup.readEntry("globalColorization", m_globalColorizationLevel);
}

CompletionSettings::GlobalColorSource CompletionSettings::globalColorSource() const
{
    const auto metaEnum = QMetaEnum::fromType<GlobalColorSource>();
    const auto globalColorSource = m_languageGroup.readEntry("globalColorSource", QByteArray());
    bool ok = false;
    auto value = metaEnum.keyToValue(globalColorSource.constData(), &ok);
    return ok ? static_cast<GlobalColorSource>(value) : m_globalColorSource;
}

int CompletionSettings::localColorizationLevel() const
{
    return m_languageGroup.readEntry("localColorization", m_localColorizationLevel);
}

bool CompletionSettings::automaticCompletionEnabled() const
{
    return m_languageGroup.readEntry("Automatic Invocation", m_automatic);
}

ICompletionSettings::CompletionLevel CompletionSettings::completionLevel() const
{
    const QString level = m_languageGroup.readEntry("completionDetail", completionLevelToString(m_level));

    for (int i = 0; i < ICompletionSettings::LAST_LEVEL; i++) {
        if (completionLevelToString(static_cast<CompletionLevel>(i)) == level) {
            return static_cast<CompletionLevel>(i);
        }
    }
    return m_level;
}

auto CompletionSettings::precompiledPreambleStorage() const -> PrecompiledPreambleStorage
{
    return static_cast<PrecompiledPreambleStorage>(LanguageConfig::precompiledPreambleStorage());
}

CompletionSettings::CompletionSettings()
    : m_todoMarkerWords(QStringLiteral("TODO FIXME"))
    , m_languageGroup(KSharedConfig::openConfig(), QStringLiteral("Language Support")){}
}

#include "moc_completionsettings.cpp"

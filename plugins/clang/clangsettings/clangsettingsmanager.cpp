/*
    SPDX-FileCopyrightText: 2015 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "clangsettingsmanager.h"

#include <KConfigGroup>
#include <KConfig>

#include <interfaces/icore.h>
#include <interfaces/isession.h>
#include <interfaces/iproject.h>
#include <custom-definesandincludes/idefinesandincludesmanager.h>
#include <util/path.h>

#include <project/projectmodel.h>

using namespace KDevelop;

namespace
{
    const QString settingsGroup = QStringLiteral("Clang Settings");

    const QString macros = QStringLiteral("macros");
    const QString lookAhead = QStringLiteral("lookAhead");

    const QString forwardDeclare = QStringLiteral("forwardDeclare");

AssistantsSettings readAssistantsSettings(KConfig* cfg)
{
    auto grp = cfg->group(settingsGroup);
    AssistantsSettings settings;

    settings.forwardDeclare = grp.readEntry(forwardDeclare, true);

    return settings;
}

CodeCompletionSettings readCodeCompletionSettings(KConfig* cfg)
{
    auto grp = cfg->group(settingsGroup);
    CodeCompletionSettings settings;

    settings.macros = grp.readEntry(macros, true);
    settings.lookAhead = grp.readEntry(lookAhead, false);

    return settings;
}
}

ClangSettingsManager* ClangSettingsManager::self()
{
    static ClangSettingsManager manager;
    return &manager;
}

AssistantsSettings ClangSettingsManager::assistantsSettings() const
{
    auto cfg = ICore::self()->activeSession()->config();
    return readAssistantsSettings(cfg.data());
}

CodeCompletionSettings ClangSettingsManager::codeCompletionSettings() const
{
    if (m_enableTesting) {
        CodeCompletionSettings settings;
        settings.lookAhead = true;
        settings.macros = true;
        return settings;
    }

    auto cfg = ICore::self()->activeSession()->config();
    return readCodeCompletionSettings(cfg.data());
}

ParserSettings ClangSettingsManager::parserSettings(KDevelop::ProjectBaseItem* item) const
{
    return {IDefinesAndIncludesManager::manager()->parserArguments(item)};
}

ParserSettings ClangSettingsManager::parserSettings(const QString& path) const
{
    return {IDefinesAndIncludesManager::manager()->parserArguments(path)};
}

ClangSettingsManager::ClangSettingsManager()
{}

bool ParserSettings::isCpp() const
{
    return parserOptions.contains(QLatin1String("-std=c++")) || parserOptions.contains(QLatin1String("-std=gnu++"));
}

QVector<QByteArray> ParserSettings::toClangAPI() const
{
    // TODO: This is not efficient.
    const auto list = QStringView(parserOptions).split(QLatin1Char(' '), Qt::SkipEmptyParts);
    QVector<QByteArray> result;
    result.reserve(list.size());

    std::transform(list.constBegin(), list.constEnd(),
                   std::back_inserter(result),
                   [] (const auto &argument) { return argument.toUtf8(); });

    return result;
}

bool ParserSettings::operator==(const ParserSettings& rhs) const
{
    return parserOptions == rhs.parserOptions;
}

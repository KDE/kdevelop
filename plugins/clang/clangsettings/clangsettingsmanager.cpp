/*
 * This file is part of KDevelop
 *
 * Copyright 2015 Sergey Kalinichev <kalinichev.so.0@gmail.com>
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
    return parserOptions.contains(QStringLiteral("-std=c++")) || parserOptions.contains(QStringLiteral("-std=gnu++"));
}

QVector<QByteArray> ParserSettings::toClangAPI() const
{
    // TODO: This is not efficient.
    auto list = parserOptions.split(QLatin1Char(' '), QString::SkipEmptyParts);
    QVector<QByteArray> result;
    result.reserve(list.size());

    std::transform(list.constBegin(), list.constEnd(),
                   std::back_inserter(result),
                   [] (const QString &argument) { return argument.toUtf8(); });

    return result;
}

bool ParserSettings::operator==(const ParserSettings& rhs) const
{
    return parserOptions == rhs.parserOptions;
}

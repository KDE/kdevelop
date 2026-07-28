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

#include <sessionconfig.h>
#include <project/projectmodel.h>

using namespace KDevelop;

ClangSettingsManager* ClangSettingsManager::self()
{
    static ClangSettingsManager manager;
    return &manager;
}

bool ClangSettingsManager::assistants_forwardDeclare() const
{
    return SessionConfig::self()->forwardDeclare();
}

bool ClangSettingsManager::codeCompletion_lookAhead() const
{
    return m_enableTesting ? true : SessionConfig::self()->lookAhead();
}

bool ClangSettingsManager::codeCompletion_macros() const
{
    return m_enableTesting ? true : SessionConfig::self()->macros();
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
    const auto list = QStringView{parserOptions}.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    QVector<QByteArray> result;
    result.reserve(list.size());

    std::transform(list.cbegin(), list.cend(), std::back_inserter(result), [](QStringView argument) {
        return argument.toUtf8();
    });

    return result;
}

bool ParserSettings::operator==(const ParserSettings& rhs) const
{
    return parserOptions == rhs.parserOptions;
}

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

#ifndef CLANGSETTINGSMANAGER_H
#define CLANGSETTINGSMANAGER_H

#include <QString>
#include <QMetaType>

class KConfig;

namespace KDevelop
{
class ProjectBaseItem;
class IProject;
}

struct ParserSettings
{
    QString parserOptions;
    bool isCpp() const;
    QVector<QByteArray> toClangAPI() const;
    bool operator==(const ParserSettings& rhs) const;
};
Q_DECLARE_METATYPE(ParserSettings);

struct CodeCompletionSettings
{
    bool macros = true;
    bool lookAhead = false;
};

struct AssistantsSettings
{
    bool forwardDeclare = true;
};

class ClangSettingsManager
{
public:
    static ClangSettingsManager* self();

    AssistantsSettings assistantsSettings() const;

    CodeCompletionSettings codeCompletionSettings() const;

    ParserSettings parserSettings(KDevelop::ProjectBaseItem* item) const;

private:
    ClangSettingsManager();

    bool m_enableTesting = false;
    friend class TestCodeCompletion;
};

#endif // CLANGSETTINGSMANAGER_H

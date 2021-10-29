/*
    SPDX-FileCopyrightText: 2015 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CLANGSETTINGSMANAGER_H
#define CLANGSETTINGSMANAGER_H

#include <QString>
#include <QMetaType>

#include "clangprivateexport.h"

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
Q_DECLARE_METATYPE(ParserSettings)

struct CodeCompletionSettings
{
    bool macros = true;
    bool lookAhead = false;
};

struct AssistantsSettings
{
    bool forwardDeclare = true;
};

class KDEVCLANGPRIVATE_EXPORT ClangSettingsManager
{
public:
    static ClangSettingsManager* self();

    AssistantsSettings assistantsSettings() const;

    CodeCompletionSettings codeCompletionSettings() const;

    ParserSettings parserSettings(KDevelop::ProjectBaseItem* item) const;

    ParserSettings parserSettings(const QString& path) const;

private:
    ClangSettingsManager();

    bool m_enableTesting = false;
    friend class CodeCompletionTestBase;
};

#endif // CLANGSETTINGSMANAGER_H

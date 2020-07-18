/* This file is part of KDevelop

   Copyright 2018 Anton Anikin <anton@anikin.xyz>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "checksdb.h"

#include "globalsettings.h"
#include "utils.h"

#include <KLocalizedString>

#include <QDir>
#include <QRegularExpression>

namespace Clazy
{

ChecksDB::ChecksDB(const QUrl& docsPath)
{
    static const QHash<QString, QString> levelName = {
        { QStringLiteral("manuallevel"), QStringLiteral("manual") },
        { QStringLiteral("hiddenlevel"), QStringLiteral("manual") }
    };

    static const QHash<QString, QString> levelDisplayName = {
        { QStringLiteral("level0"), i18nc("@item level of checks", "Level 0") },
        { QStringLiteral("level1"), i18nc("@item level of checks", "Level 1") },
        { QStringLiteral("level2"), i18nc("@item level of checks", "Level 2") },
        { QStringLiteral("level3"), i18nc("@item level of checks", "Level 3") },
        { QStringLiteral("manual"), i18nc("@item level of checks", "Manual Level") }
    };

    static const QHash<QString, QString> levelDescription = {
        { QStringLiteral("level0"),
          i18n("Very stable checks, 99.99% safe, mostly no false-positives, very desirable.") },

        { QStringLiteral("level1"),
          i18n("The default level. Very similar to level 0, slightly more false-positives but very few.") },

        { QStringLiteral("level2"),
          i18n("Also very few false-positives, but contains noisy checks which not everyone agree should be default.") },

        { QStringLiteral("level3"),
          i18n("Contains checks with high rate of false-positives.") },

        { QStringLiteral("manual"),
          i18n("Checks here need to be enabled explicitly, as they don't belong to any level. "
               "Checks here are very stable and have very few false-positives.") }
    };

    const QString defaultError = i18n(
        "Unable to load Clazy checks information from '%1'. Please check your settings.",
        docsPath.toLocalFile());

    QDir docsDir(docsPath.toLocalFile());
    if (!docsDir.exists()) {
        m_error = defaultError;
        return;
    }

    const QRegularExpression levelRE(QStringLiteral(".*level.*"));
    const QRegularExpression checkRE(QStringLiteral("^README-(.+)\\.md$"));

    const auto levelsDirs = docsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const auto& levelDir : levelsDirs) {
        if (!levelRE.match(levelDir).hasMatch()) {
            continue;
        }

        if (!docsDir.cd(levelDir)) {
            continue;
        }

        auto level = new Level;
        level->name = levelName.value(levelDir, levelDir);
        level->displayName = levelDisplayName.value(level->name, levelDir);
        level->description = levelDescription.value(level->name, {});

        const auto checksFiles = docsDir.entryList(QDir::Files | QDir::Readable);
        for (const auto& checkFile : checksFiles) {
            auto match = checkRE.match(checkFile);
            if (!match.hasMatch()) {
                continue;
            }

            QFile mdFile(docsDir.absoluteFilePath(checkFile));
            if (!mdFile.open(QIODevice::ReadOnly)) {
                continue;
            }

            auto check = new Check;
            check->level = level;
            check->name = match.captured(1);
            check->description = markdown2html(mdFile.readAll());
            level->checks[check->name] = check;

            m_checks[check->name] = check;
        }

        if (level->checks.isEmpty()) {
            delete level;
        } else {
            m_levels[level->name] = level;
        }

        docsDir.cdUp();
    }

    if (m_levels.isEmpty()) {
        m_error = defaultError;
    }
}

ChecksDB::~ChecksDB()
{
    qDeleteAll(m_levels);
    qDeleteAll(m_checks);
}

bool ChecksDB::isValid() const
{
    return m_error.isEmpty();
}

QString ChecksDB::error() const
{
    return m_error;
}

const QMap<QString, Level*>& ChecksDB::levels() const
{
    return m_levels;
}

const QMap<QString, Check*>& ChecksDB::checks() const
{
    return m_checks;
}

QString ChecksDB::defaultChecks()
{
    return QStringLiteral("level1");
}

}

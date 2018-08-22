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

#include "utils.h"

#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/projectmodel.h>

#include <KLocalizedString>

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QRegularExpression>

namespace Clazy
{

QString prettyPathName(const QString& path)
{
    return KDevelop::ICore::self()->projectController()->prettyFileName(
        QUrl::fromLocalFile(path),
        KDevelop::IProjectController::FormatPlain);
}

QStringList compileCommandsFiles(const QString& jsonFilePath, QString& error)
{
    QStringList paths;

    QFile jsonFile(jsonFilePath);
    if (!jsonFile.open(QFile::ReadOnly | QFile::Text)) {
        error = i18n("Unable to open compile commands file '%1' for reading", jsonFilePath);
        return paths;
    }

    QJsonParseError jsonError;
    auto document = QJsonDocument::fromJson(jsonFile.readAll(), &jsonError);

    if (jsonError.error) {
        error = i18n("JSON error during parsing compile commands file '%1': %2", jsonFilePath, jsonError.errorString());
        return paths;
    }

    if (!document.isArray()) {
        error = i18n("JSON error during parsing compile commands file '%1': document is not an array", jsonFilePath);
        return paths;
    }

    const QString KEY_FILE = QStringLiteral("file");

    const auto array = document.array();
    for (const auto& value : array) {
        if (!value.isObject()) {
            continue;
        }

        const QJsonObject entry = value.toObject();
        if (entry.contains(KEY_FILE)) {
            auto path = entry[KEY_FILE].toString();
            if (QFile::exists(path))
            {
                paths += path;
            }
        }
    }

    return paths;
}

// Very simple Markdown parser/converter. Does not provide full Markdown language support and
// was tested only with Clazy documentation.
class MarkdownConverter
{
public:
    MarkdownConverter()
    {
        tagStart.resize(STATE_COUNT);
        tagEnd.resize(STATE_COUNT);

        tagStart[EMPTY].clear();
        tagEnd  [EMPTY].clear();

        tagStart[HEADING] = QStringLiteral("<b>");
        tagEnd  [HEADING] = QStringLiteral("</b>");

        tagStart[PARAGRAPH] = QStringLiteral("<p>");
        tagEnd  [PARAGRAPH] = QStringLiteral("</p>");

        tagStart[PREFORMATTED] = QStringLiteral("<pre>");
        tagEnd  [PREFORMATTED] = QStringLiteral("</pre>");

        tagStart[LIST] = QStringLiteral("<ul><li>");
        tagEnd  [LIST] = QStringLiteral("</li></ul>");
    }

    ~MarkdownConverter() = default;

    QString toHtml(const QString& markdown)
    {
        const QRegularExpression hRE(QStringLiteral("(#+) (.+)"));
        QRegularExpressionMatch match;

        state = EMPTY;
        html.clear();
        html += QStringLiteral("<html>");

        auto lines = markdown.split('\n');
        for (auto line : lines) {
            if (line.isEmpty()) {
                setState(EMPTY);
                continue;
            }

            if (line.startsWith("#")) {
                auto match = hRE.match(line);
                if (match.hasMatch()) {
                    setState(HEADING);
                    html += match.captured(2);
                    setState(EMPTY);
                    if (match.captured(1).size() == 1) {
                        html += QStringLiteral("<hr>");
                    }
                }
                continue;
            }

            if (line.startsWith(QStringLiteral("```"))) {
                setState((state == PREFORMATTED) ? EMPTY : PREFORMATTED);
                continue;
            }

            if (line.startsWith(QStringLiteral("    "))) {
                if (state == EMPTY) {
                    setState(PREFORMATTED);
                }
            } else if (
                line.startsWith(QStringLiteral("- ")) ||
                line.startsWith(QStringLiteral("* "))) {
                // force close and reopen list - this fixes cases when we don't have
                // separator line between items
                setState(EMPTY);
                setState(LIST);
                line = line.mid(2);
            }

            if (state == EMPTY) {
                setState(PARAGRAPH);
            }

            processLine(line);
        }
        setState(EMPTY);

        html += QStringLiteral("</html>");
        return html.join(QLatin1Char('\n'));
    }

private:
    enum STATE {
        EMPTY,
        HEADING,
        PARAGRAPH,
        PREFORMATTED,
        LIST,

        STATE_COUNT
    };

    void setState(int newState)
    {
        if (state == newState) {
            return;
        }

        if (state != EMPTY) {
            html += tagEnd[state];
        }

        if (newState != EMPTY) {
            html += tagStart[newState];
        }

        state = newState;
    }

    void processLine(QString& line)
    {
        static const QRegularExpression ttRE(QStringLiteral("`([^`]+)`"));
        static const QRegularExpression bdRE(QStringLiteral("\\*\\*([^\\*]+)\\*\\*"));
        static const QRegularExpression itRE(QStringLiteral("[^\\*]\\*([^\\*]+)\\*[^\\*]"));

        static auto applyRE = [](const QRegularExpression& re, QString& line, const QString& tag) {
            auto i = re.globalMatch(line);
            while (i.hasNext()) {
                auto match = i.next();
                line.replace(match.captured(0), QStringLiteral("<%1>%2</%1>").arg(tag, match.captured(1)));
            }
        };

        if (state != PREFORMATTED) {
            line.replace(QLatin1Char('&'), QLatin1String("&amp;"));
            line.replace(QLatin1Char('<'), QLatin1String("&lt;"));
            line.replace(QLatin1Char('>'), QLatin1String("&gt;"));

            line.replace(QLatin1Char('\"'), QLatin1String("&quot;"));
            line.replace(QLatin1Char('\''), QLatin1String("&#39;"));

            applyRE(ttRE, line, QStringLiteral("tt"));
            applyRE(bdRE, line, QStringLiteral("b"));
            applyRE(itRE, line, QStringLiteral("i"));
        }

        html += line;
    }

private:
    int state;
    QVector<QString> tagStart;
    QVector<QString> tagEnd;
    QStringList html;
};

QString markdown2html(const QByteArray& markdown)
{
    MarkdownConverter converter;
    return converter.toHtml(markdown);
}

}

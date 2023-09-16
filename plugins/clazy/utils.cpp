/*
    SPDX-FileCopyrightText: 2018 Anton Anikin <anton@anikin.xyz>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "utils.h"

// KDevPlatform
#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
// KF
#include <KLocalizedString>
// Qt
#include <QFile>
#include <QRegularExpression>
#include <QVector>

namespace Clazy
{

QString prettyPathName(const QUrl& path)
{
    return KDevelop::ICore::self()->projectController()->prettyFileName(path, KDevelop::IProjectController::FormatPlain);
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

        const auto lines = markdown.split(QLatin1Char('\n'));
        for (auto line : lines) {
            if (line.isEmpty()) {
                setState(EMPTY);
                continue;
            }

            if (line.startsWith(QLatin1Char('#'))) {
                auto match = hRE.match(line);
                if (match.hasMatch()) {
                    setState(HEADING);
                    html += match.captured(2);
                    setState(EMPTY);
                    if (match.capturedView(1).size() == 1) {
                        html += QStringLiteral("<hr>");
                    }
                }
                continue;
            }

            if (line.startsWith(QLatin1String("```"))) {
                setState((state == PREFORMATTED) ? EMPTY : PREFORMATTED);
                continue;
            }

            if (line.startsWith(QLatin1String("    "))) {
                if (state == EMPTY) {
                    setState(PREFORMATTED);
                }
            } else if (
                line.startsWith(QLatin1String("- ")) ||
                line.startsWith(QLatin1String("* "))) {
                // force close and reopen list - this fixes cases when we don't have
                // separator line between items
                setState(EMPTY);
                setState(LIST);
                line.remove(0, 2);
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
    return converter.toHtml(QString::fromUtf8(markdown));
}

}

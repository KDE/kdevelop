/*
 * This file is part of KDevelop
 *
 * Copyright 2016 Carlos Nihelton <carlosnsoliveira@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "clangtidyparser.h"

// plugin
#include <qtcompat_p.h>
// KDevPlatform
#include <language/editor/documentrange.h>
#include <serialization/indexedstring.h>
#include <shell/problem.h>
// Qt
#include <QRegularExpression>

namespace ClangTidy
{
using KDevelop::IProblem;
using KDevelop::DetectedProblem;
using KDevelop::DocumentRange;
using KDevelop::IndexedString;
/**
 * Convert the value of <verbose> attribute of <error> element from clang-tidy's
 * XML-output to 'good-looking' HTML-version. This is necessary because the
 * displaying of the original message is performed without line breaks - such
 * tooltips are uncomfortable to read, and large messages will not fit into the
 * screen.
 *
 * This function put the original message into \<html\> tag that automatically
 * provides line wrapping by builtin capabilities of Qt library. The source text
 * also can contain tokens '\012' (line break) - they are present in the case of
 * source code examples. In such cases, the entire text between the first and
 * last tokens (i.e. source code) is placed into \<pre\> tag.
 *
 * @param[in] input the original value of <verbose> attribute
 * @return HTML version for displaying in problem's tooltip
 */
QString verboseMessageToHtml(const QString& input)
{
    QString output(QStringLiteral("<html>%1</html>").arg(input.toHtmlEscaped()));

    output.replace(QStringLiteral("\\012"), QStringLiteral("\n"));

    if (output.count(QLatin1Char('\n')) >= 2) {
        output.replace(output.indexOf(QLatin1Char('\n')), 1, QStringLiteral("<pre>"));
        output.replace(output.lastIndexOf(QLatin1Char('\n')), 1, QStringLiteral("</pre><br>"));
    }

    return output;
}

ClangTidyParser::ClangTidyParser(QObject* parent)
    : QObject(parent)
{
}

void ClangTidyParser::parse()
{
    //                                       (1filename                 ) (2lin) (3col)  (4s)  (5d) (6explain)
    QRegularExpression regex(QStringLiteral("(\\/.+\\.[ch]{1,2}[px]{0,2}):(\\d+):(\\d+): (.+): (.+) (\\[.+\\])"));

    for (const auto& line : qAsConst(m_stdout)) {
        auto smatch = regex.match(line);
        if (smatch.hasMatch()) {
            IProblem::Ptr problem(new DetectedProblem());
            problem->setSource(IProblem::Plugin);
            problem->setDescription(smatch.captured(5));
            problem->setExplanation(smatch.capturedRef(6) + QLatin1Char('\n'));

            DocumentRange range;
            range.document = IndexedString(smatch.captured(1));
            range.setBothColumns(smatch.capturedRef(3).toInt() - 1);
            range.setBothLines(smatch.capturedRef(2).toInt() - 1);
            problem->setFinalLocation(range);

            const auto sev = smatch.capturedRef(4);
            IProblem::Severity erity;
            if (sev == QStringLiteral("error")) {
                erity = IProblem::Error;
            } else if (sev == QStringLiteral("warning")) {
                erity = IProblem::Warning;
            } else if (sev == QStringLiteral("note")) {
                erity = IProblem::Hint;
            } else {
                erity = IProblem::NoSeverity;
            }
            problem->setSeverity(erity);
            m_problems.push_back(problem);

        } else if (!m_problems.isEmpty()) {
            auto problem = m_problems.last();
            const QString extendedExplanation = problem->explanation() + QLatin1Char('\n') + line;
            problem->setExplanation(extendedExplanation);
        } else {
            continue;
        }
    }
}
} // namespace ClangTidy

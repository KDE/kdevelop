/*
    SPDX-FileCopyrightText: 2016 Carlos Nihelton <carlosnsoliveira@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "clangtidyparser.h"

// KDevPlatform
#include <language/editor/documentrange.h>
#include <serialization/indexedstring.h>
#include <shell/problem.h>

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

    output.replace(QLatin1String("\\012"), QLatin1String("\n"));

    if (output.count(QLatin1Char('\n')) >= 2) {
        output.replace(output.indexOf(QLatin1Char('\n')), 1, QStringLiteral("<pre>"));
        output.replace(output.lastIndexOf(QLatin1Char('\n')), 1, QStringLiteral("</pre><br>"));
    }

    return output;
}

ClangTidyParser::ClangTidyParser(QObject* parent)
    : QObject(parent)
      //                            (1 filename                              ) (2ln) (3cl)  (4se)  (5d) (6expln)
    , m_hitRegExp(QStringLiteral(R"(((?:[A-Za-z]:\\|\/).+\.[ch]{1,2}[px]{0,2}):(\d+):(\d+): (.+?): (.+) (\[.+\]))"))
{
}

void ClangTidyParser::addData(const QStringList& stdoutList)
{
    QVector<KDevelop::IProblem::Ptr> problems;

    for (const auto& line : std::as_const(stdoutList)) {
        auto smatch = m_hitRegExp.match(line);

        if (!smatch.hasMatch()) {
            continue;
        }

        IProblem::Ptr problem(new DetectedProblem());
        problem->setSource(IProblem::Plugin);
        problem->setDescription(smatch.captured(5));
        problem->setExplanation(smatch.captured(6));

        DocumentRange range;
        range.document = IndexedString(smatch.captured(1));
        range.setBothColumns(smatch.capturedView(3).toInt() - 1);
        range.setBothLines(smatch.capturedView(2).toInt() - 1);
        problem->setFinalLocation(range);

        const auto sev = smatch.capturedView(4);
        const IProblem::Severity erity =
            (sev == QLatin1String("error")) ?   IProblem::Error :
            (sev == QLatin1String("warning")) ? IProblem::Warning :
            (sev == QLatin1String("note")) ?    IProblem::Hint :
            /* else */                          IProblem::NoSeverity;
        problem->setSeverity(erity);

        problems.append(problem);
    }

    if (!problems.isEmpty()) {
        emit problemsDetected(problems);
    }
}

} // namespace ClangTidy

#include "moc_clangtidyparser.cpp"

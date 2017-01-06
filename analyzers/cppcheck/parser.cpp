/* This file is part of KDevelop

   Copyright 2013 Christoph Thielecke <crissi99@gmx.de>
   Copyright 2016 Anton Anikin <anton.anikin@htower.ru>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "parser.h"

#include "debug.h"

#include <KLocalizedString>
#include <KMessageBox>
#include <shell/problem.h>

#include <QApplication>

namespace cppcheck
{

class CppcheckProblem : public KDevelop::DetectedProblem
{
public:
    CppcheckProblem() {}
    ~CppcheckProblem() override {}

    QString sourceString() const override { return QStringLiteral("Cppcheck"); };
};

/**
 * Convert the value of \<verbose\> attribute of \<error\> element from cppcheck's
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
 * @param[in] input the original value of \<verbose\> attribute
 * @return HTML version for displaying in problem's tooltip
 */
QString verboseMessageToHtml( const QString & input )
{
    QString output(QString("<html>%1</html>").arg(input.toHtmlEscaped()));

    output.replace("\\012", "\n");

    if (output.count('\n') >= 2) {
        output.replace(output.indexOf('\n'), 1, "<pre>" );
        output.replace(output.lastIndexOf('\n'), 1, "</pre><br>" );
    }

    return output;
}

CppcheckParser::CppcheckParser()
    : m_errorInconclusive(false)
{
}

CppcheckParser::~CppcheckParser()
{
}

void CppcheckParser::clear()
{
    m_stateStack.clear();
}

bool CppcheckParser::startElement()
{
    State newState = Unknown;

    qCDebug(KDEV_CPPCHECK) << "CppcheckParser::startElement: elem: " << qPrintable(name().toString());

    if (name() == "results")
        newState = Results;

    else if (name() == "cppcheck")
        newState = CppCheck;

    else if (name() == "errors")
        newState = Errors;

    else if (name() == "location") {
        newState = Location;
        if (attributes().hasAttribute("file") && attributes().hasAttribute("line")) {
            m_errorFiles += attributes().value("file").toString();
            m_errorLines += attributes().value("line").toString().toInt();
        }
    }

    else if (name() == "error") {
        newState = Error;
        m_errorSeverity = "unknown";
        m_errorInconclusive = false;
        m_errorFiles.clear();
        m_errorLines.clear();
        m_errorMessage.clear();
        m_errorVerboseMessage.clear();

        if (attributes().hasAttribute("msg"))
            m_errorMessage = attributes().value("msg").toString();

        if (attributes().hasAttribute("verbose"))
            m_errorVerboseMessage = verboseMessageToHtml(attributes().value("verbose").toString());

        if (attributes().hasAttribute("severity"))
            m_errorSeverity = attributes().value("severity").toString();

        if (attributes().hasAttribute("inconclusive"))
            m_errorInconclusive = true;
    }

    else {
        m_stateStack.push(m_stateStack.top());
        return true;
    }

    m_stateStack.push(newState);

    return true;
}

bool CppcheckParser::endElement(QVector<KDevelop::IProblem::Ptr>& problems)
{
    qCDebug(KDEV_CPPCHECK) << "CppcheckParser::endElement: elem: " << qPrintable(name().toString());

    State state = m_stateStack.pop();

    switch (state) {
    case CppCheck:
        if (attributes().hasAttribute("version"))
            qCDebug(KDEV_CPPCHECK) << "Cppcheck report version: " << attributes().value("version");
        break;

    case Errors:
        // errors finished
        break;

    case Error:
        qCDebug(KDEV_CPPCHECK) << "CppcheckParser::endElement: new error elem: line: "
                               << (m_errorLines.isEmpty() ? "?" : QString::number(m_errorLines.first()))
                               << " at " << (m_errorFiles.isEmpty() ? "?" : m_errorFiles.first())
                               << ", msg: " << m_errorMessage;

        storeError(problems);
        break;

    case Results:
        // results finished
        break;

    case Location:
        break;

    default:
        break;
    }

    return true;
}

QVector<KDevelop::IProblem::Ptr> CppcheckParser::parse()
{
    QVector<KDevelop::IProblem::Ptr> problems;
    qCDebug(KDEV_CPPCHECK) << "CppcheckParser::parse!";

    while (!atEnd()) {
        int readNextVal = readNext();

        switch (readNextVal) {
        case StartDocument:
            clear();
            break;

        case StartElement:
            startElement();
            break;

        case EndElement:
            endElement(problems);
            break;

        case Characters:
            break;

        default:
            qCDebug(KDEV_CPPCHECK) << "CppcheckParser::startElement: case: " << readNextVal;
            break;
        }
    }

    qCDebug(KDEV_CPPCHECK) << "CppcheckParser::parse: end";

    if (hasError()) {
        switch (error()) {
        case CustomError:
        case UnexpectedElementError:
        case NotWellFormedError:
            KMessageBox::error(
                qApp->activeWindow(),
                i18n("Cppcheck XML Parsing: error at line %1, column %2: %3", lineNumber(), columnNumber(), errorString()),
                i18n("Cppcheck Error"));
            break;

        case NoError:
        case PrematureEndOfDocumentError:
            break;
        }
    }

    return problems;
}

void CppcheckParser::storeError(QVector<KDevelop::IProblem::Ptr>& problems)
{
    // Construct problem with using first location element
    KDevelop::IProblem::Ptr problem = getProblem();

    // Adds other <location> elements as diagnostics.
    // This allows the user to track the problem.
    for (int locationIdx = 1; locationIdx < m_errorFiles.size(); ++locationIdx) {
        problem->addDiagnostic(getProblem(locationIdx));
    }

    problems.push_back(problem);
}

KDevelop::IProblem::Ptr CppcheckParser::getProblem(int locationIdx) const
{
    KDevelop::IProblem::Ptr problem(new CppcheckProblem);
    QStringList messagePrefix;
    QString errorMessage(m_errorMessage);

    problem->setSource(KDevelop::IProblem::Plugin);

    if (m_errorSeverity == "error")
        problem->setSeverity(KDevelop::IProblem::Error);

    else if (m_errorSeverity == "warning")
        problem->setSeverity(KDevelop::IProblem::Warning);

    else {
       problem->setSeverity(KDevelop::IProblem::Hint);

       messagePrefix.push_back(m_errorSeverity);
    }

    if (m_errorInconclusive)
        messagePrefix.push_back("inconclusive");

    if (!messagePrefix.isEmpty())
        errorMessage = QString("(%1) %2").arg(messagePrefix.join(", ")).arg(m_errorMessage);

    problem->setDescription(errorMessage);
    problem->setExplanation(m_errorVerboseMessage);

    KDevelop::DocumentRange range;

    if (locationIdx < 0 || locationIdx >= m_errorFiles.size())
        range = KDevelop::DocumentRange::invalid();
    else {
        range.document = KDevelop::IndexedString(m_errorFiles.at(locationIdx));
        range.setBothLines(m_errorLines.at(locationIdx) - 1);
        range.setBothColumns(0);
    }

    problem->setFinalLocation(range);

    return problem;
}

}

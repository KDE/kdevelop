/* This file is part of KDevelop
 * Copyright 2013 Christoph Thielecke <crissi99@gmx.de>
 * Copyright 2015 Anton Anikin <anton.anikin@htower.ru>

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

#include <KLocalizedString>
#include <KMessageBox>
#include <QApplication>
#include <shell/problem.h>

#include "clangtidyparser.h"
#include "debug.h"

namespace ClangTidy
{

/**
 * Convert the value of <verbose> attribute of <error> element from clangtidy's
 * XML-output to 'good-looking' HTML-version. This is necessary because the
 * displaying of the original message is performed without line breaks - such
 * tooltips are uncomfortable to read, and large messages will not fit into the
 * screen.
 *
 * This function put the original message into <html> tag that automatically
 * provides line wrapping by builtin capabilities of Qt library. The source text
 * also can contain tokens '\012' (line break) - they are present in the case of
 * source code examples. In such cases, the entire text between the first and
 * last tokens (i.e. source code) is placed into <pre> tag.
 *
 * @param[in] input the original value of <verbose> attribute
 * @return HTML version for displaying in problem's tooltip
 */
QString verboseMessageToHtml(const QString& input)
{
    QString output(QString("<html>%1</html>").arg(input.toHtmlEscaped()));

    output.replace("\\012", "\n");

    if (output.count('\n') >= 2) {
        output.replace(output.indexOf('\n'), 1, "<pre>");
        output.replace(output.lastIndexOf('\n'), 1, "</pre><br>");
    }

    return output;
}

ClangtidyParser::ClangtidyParser(QObject*)
    : m_errorLine(0)
    , m_errorInconclusive(false)
    , m_errorSeverity(Unknown)
{
}

ClangtidyParser::~ClangtidyParser()
{
}

void ClangtidyParser::clear()
{
    m_stateStack.clear();
}

bool ClangtidyParser::startElement()
{
    State newState = Unknown;

    qCDebug(KDEV_CLANGTIDY) << "ClangtidyParser::startElement: elem: " << qPrintable(name().toString());

    if (name() == "results")
        newState = Results;

    else if (name() == "clangtidy")
        newState = Clangtidy;

    else if (name() == "errors")
        newState = Errors;

    else if (name() == "location") {
        newState = Location;
        // use only first <location> element of the error
        if (m_errorLine < 0) {
            if (attributes().hasAttribute("line"))
                m_errorLine = attributes().value("line").toString().toInt();
            if (attributes().hasAttribute("file"))
                m_errorFile = attributes().value("file").toString();
        }
    }

    else if (name() == "error") {
        newState = Error;
        m_errorLine = -1;
        m_errorSeverity = "unknown";
        m_errorInconclusive = false;
        m_errorFile.clear();
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

bool ClangtidyParser::endElement()
{
    qCDebug(KDEV_CLANGTIDY) << "ClangtidyParser::endElement: elem: " << qPrintable(name().toString());

    State state = m_stateStack.pop();

    switch (state) {
    case Clangtidy:
        if (attributes().hasAttribute("version"))
            qCDebug(KDEV_CLANGTIDY) << "Clangtidy report version: " << attributes().value("version");
        break;

    case Errors:
        // errors finished
        break;

    case Error:
        qCDebug(KDEV_CLANGTIDY) << "ClangtidyParser::endElement: new error elem: line: " << m_errorLine << " at "
                                << m_errorFile << ", msg: " << m_errorMessage;

        storeError();
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

void ClangtidyParser::parse()
{
    qCDebug(KDEV_CLANGTIDY) << "ClangtidyParser::parse!";

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
            endElement();
            break;

        case Characters:
            break;

        default:
            qCDebug(KDEV_CLANGTIDY) << "ClangtidyParser::startElement: case: " << readNextVal;
            break;
        }
    }

    qCDebug(KDEV_CLANGTIDY) << "ClangtidyParser::parse: end";

    if (hasError()) {
        switch (error()) {
        case CustomError:
        case UnexpectedElementError:
        case NotWellFormedError:
            KMessageBox::error(qApp->activeWindow(), i18n("Clangtidy XML Parsing: error at line %1, column %2: %3",
                                                          lineNumber(), columnNumber(), errorString()),
                               i18n("Clangtidy Error"));
            break;

        case NoError:
        case PrematureEndOfDocumentError:
            break;
        }
    }
}

void ClangtidyParser::storeError()
{
    KDevelop::IProblem::Ptr problem(new KDevelop::DetectedProblem());
    QStringList messagePrefix;

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
        m_errorMessage = QString("(%1) %2").arg(messagePrefix.join(", ")).arg(m_errorMessage);

    problem->setDescription(m_errorMessage);
    problem->setExplanation(m_errorVerboseMessage);

    KDevelop::DocumentRange range;
    range.document = KDevelop::IndexedString(m_errorFile);
    range.setBothLines(m_errorLine - 1);
    problem->setFinalLocation(range);

    m_problems.push_back(problem);
}
}

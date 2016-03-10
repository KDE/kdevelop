/* This file is part of KDevelop
 * Copyright 2013 Christoph Thielecke <crissi99@gmx.de>

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

#include <interfaces/iprojectcontroller.h>
#include <interfaces/icore.h>
#include <interfaces/iproject.h>

#include <QDir>

#include <KLocalizedString>
#include <KMessageBox>

#include "debug.h"
#include "cppcheckparser.h"

#include <util/path.h>

#include <shell/problem.h>

namespace cppcheck {


CppcheckParser::CppcheckParser(QObject* parent) :
    ErrorLine(0),
    Severity(Unknown)
{
    Q_UNUSED(parent)
}

CppcheckParser::~CppcheckParser()
{
}

void CppcheckParser::clear()
{
    m_stateStack.clear();
    m_buffer.clear();
    cppcheckArgs.clear();
    programArgs.clear();
}
bool CppcheckParser::startElement()
{
    m_buffer.clear();
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
        if (attributes().hasAttribute("line"))
            ErrorLine = attributes().value("line").toString().toInt();
        if (attributes().hasAttribute("file")) {
            ErrorFile = attributes().value("file").toString();
            /* get project path */
            ProjectPath = "";
            for (int i = 0; i < KDevelop::ICore::self()->projectController()->projects().count(); i++) {
                if (KDevelop::ICore::self()->projectController()->findProjectForUrl(QUrl::fromLocalFile(ErrorFile)) != nullptr) {
                    ProjectPath = KDevelop::ICore::self()->projectController()->projects().at(i)->path().toUrl().toLocalFile();
                }
            }
            ErrorFile.remove(ProjectPath);
        }
    } else if (name() == "error") {
        newState = Error;
        ErrorLine = -1;
        ErrorFile.clear();
        Message.clear();
        MessageVerbose.clear();
        ProjectPath.clear();
        Severity = "unknown";
        if (attributes().hasAttribute("msg"))
            Message = attributes().value("msg").toString();
        if (attributes().hasAttribute("verbose"))
            MessageVerbose = attributes().value("verbose").toString();
        if (attributes().hasAttribute("severity"))
            Severity = attributes().value("severity").toString();

    } else {
        m_stateStack.push(m_stateStack.top());
        return true;
    }
    m_stateStack.push(newState);
    return true;
}

bool CppcheckParser::endElement()
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
        qCDebug(KDEV_CPPCHECK) << "CppcheckParser::endElement: new error elem: line: " << ErrorLine << " at " << ErrorFile << ", msg: " << Message;

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

void CppcheckParser::parse()
{
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
            endElement();
            break;
        case Characters:
            m_buffer += text().toString();
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
            KMessageBox::error(qApp->activeWindow(), i18n("Cppcheck XML Parsing: error at line %1, column %2: %3", lineNumber(), columnNumber(), errorString()), i18n("Cppcheck Error"));
            break;
        case NoError:
        case PrematureEndOfDocumentError:
            break;
        }
    }
}

void CppcheckParser::storeError()
{
   KDevelop::IProblem::Ptr problem(new KDevelop::DetectedProblem());
   problem->setDescription(Message);
   problem->setExplanation(MessageVerbose);

   KDevelop::DocumentRange range;
   range.document = KDevelop::IndexedString(ProjectPath + ErrorFile);
   range.setBothLines(ErrorLine - 1);

   problem->setFinalLocation(range);

   if (Severity == "error")
       problem->setSeverity(KDevelop::IProblem::Error);
   else
   if (Severity == "warning")
       problem->setSeverity(KDevelop::IProblem::Warning);
   else
       problem->setSeverity(KDevelop::IProblem::Hint);

   problem->setSource(KDevelop::IProblem::Plugin);

   m_problems.push_back(problem);
}

}

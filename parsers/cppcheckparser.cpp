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

#include <KDebug>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <QDir>
#include <kdeversion.h>

#include "cppcheckparser.h"

namespace cppcheck
{

CppcheckParser::CppcheckParser(QObject* parent) :
    ErrorLine(0),
    ErrorFile(""),
    Message(""),
    MessageVerbose(""),
    Severity(Unknown),
    ProjectPath("")
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

    kDebug() << "CppcheckParser::startElement: elem: " << qPrintable(name().toString());


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
                if (KDevelop::ICore::self()->projectController()->findProjectForUrl(KUrl(ErrorFile)) != 0) {
                    ProjectPath = KDevelop::ICore::self()->projectController()->projects().at(i)->folder().toLocalFile();
                }
            }
            ErrorFile.remove(ProjectPath);
        }
    } else if (name() == "error") {
        newState = Error;
        ErrorLine = -1;
        ErrorFile = "";
        Message = "";
        MessageVerbose = "";
        ProjectPath = "";
        Severity = "unknown";
        if (attributes().hasAttribute("msg"))
            Message = attributes().value("msg").toString();
        if (attributes().hasAttribute("verbose"))
            MessageVerbose = attributes().value("verbose").toString();
        if (attributes().hasAttribute("severity"))
            Severity = attributes().value("severity").toString();
        emit newElement(cppcheck::CppcheckModel::startError);
    } else {
        m_stateStack.push(m_stateStack.top());
        return true;
    }
    m_stateStack.push(newState);
    return true;
}

bool CppcheckParser::endElement()
{
    kDebug() << "CppcheckParser::endElement: elem: " << qPrintable(name().toString());
    State state = m_stateStack.pop();
    switch (state) {
    case CppCheck:
        if (attributes().hasAttribute("version"))
            kDebug() << "CppCheck report version: " << attributes().value("version");
        break;
    case Errors:
        // errors finished
        break;
    case Error:
        kDebug() << "CppcheckParser::endElement: new error elem: line: " << ErrorLine << " at " << ErrorFile << ", msg: " << Message;
        emit newData(cppcheck::CppcheckModel::error, name().toString(), m_buffer, ErrorLine, ErrorFile, Message, MessageVerbose, ProjectPath, Severity);
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
    kDebug() << "CppcheckParser::parse!";



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
            kDebug() << "CppcheckParser::startElement: case: " << readNextVal;
            break;
        }
    }
    kDebug() << "CppcheckParser::parse: end";

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

}

#include "cppcheckparser.moc"

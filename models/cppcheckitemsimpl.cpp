/* This file is part of KDevelop
 * Copyright 2006-2008 Hamish Rodda <rodda@kde.org>
 * Copyright 2011 Mathieu Lornac <mathieu.lornac@gmail.com>
 * Copyright 2011 Damien Coppel <damien.coppel@gmail.com>
 * Copyright 2011 Lionel Duc <lionel.data@gmail.com>
 * Copyright 2011 Sarie Lucas <lucas.sarie@gmail.com>

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

#include "cppcheckitemsimpl.h"
#include "cppcheckmodel.h"

#include <kdebug.h>

///////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

namespace cppcheck
{

CppcheckError::CppcheckError(cppcheck::CppcheckModel* parent)
    : m_parent(parent),
    ErrorLine(0),
    uniqueId(0),
    threadId(0),
    m_kind(Unknown)
{
}

CppcheckError::~CppcheckError()
{
}

CppcheckModel* CppcheckError::parent() const
{
    return m_parent;
}

void CppcheckError::incomingData(QString name, QString value, int ErrorLine, QString ErrorFile, QString Message, QString MessageVerbose, QString ProjectPath, QString Severity)
{
    if (name == "unique")
        this->uniqueId = value.toInt(0L, 16);
    else if (name == "tid")
        this->threadId = value.toInt();
    else if (name == "kind")
        this->setKind(value);
    else if (name == "what")
        this->what = value;
    else if (name == "text")
        this->text = value;
    else if (name == "auxwhat")
        this->auxWhat = value;

    this->ErrorLine = ErrorLine;
    this->ErrorFile = ErrorFile;
    this->Message = Message;
    this->MessageVerbose = MessageVerbose;
    this->ProjectPath = ProjectPath;
    this->Severity = Severity;
}

void CppcheckError::setKind(const QString& s)
{
    if (s == "Unknown")
        m_kind = Unknown;
    else
        m_kind = Unknown;
}


CppcheckStack *CppcheckError::addStack()
{
    m_stack << new CppcheckStack(this);
    return m_stack.back();
}

CppcheckStack *CppcheckError::lastStack() const
{
    return m_stack.back();
}

const QList<CppcheckStack *> &CppcheckError::getStack() const
{
    return m_stack;
}

////////////////////////

QString CppcheckStack::what() const
{
    return "In cppcheckstack what";
}

CppcheckStack::CppcheckStack(CppcheckError *parent)
    : m_parent(parent)
{
}


CppcheckStack::~CppcheckStack()
{
}

CppcheckError* CppcheckStack::parent() const
{
    return m_parent;
}

void CppcheckStack::incomingData(QString name, QString value, int ErrorLine, QString ErrorFile, QString Message, QString MessageVerbose, QString ProjectPath, QString Severity)
{
    Q_UNUSED(name);
    Q_UNUSED(value);
    Q_UNUSED(ErrorLine);
    Q_UNUSED(ErrorFile);
    Q_UNUSED(Message);
    Q_UNUSED(MessageVerbose);
    Q_UNUSED(ProjectPath);
    Q_UNUSED(Severity);
}


CppcheckFrame *CppcheckStack::addFrame()
{
    m_frames << new CppcheckFrame(this);
    return m_frames.back();
}

CppcheckFrame *CppcheckStack::lastFrame() const
{
    return m_frames.back();
}

const QList<CppcheckFrame *> &CppcheckStack::getFrames() const
{
    return m_frames;
}

CppcheckFrame::CppcheckFrame(CppcheckStack* parent)
    : m_parent(parent),
    ErrorLine(0),
    line(0),
    instructionPointer(0L)
{
}

CppcheckStack* CppcheckFrame::parent() const
{
    return m_parent;
}

void CppcheckFrame::incomingData(QString name, QString value, int ErrorLine, QString ErrorFile, QString Message, QString MessageVerbose, QString ProjectPath, QString Severity)
{
    if (name == "ip")
        this->instructionPointer = value.toInt(0L, 16);
    else if (name == "obj")
        this->obj = value;
    else if (name == "fn")
        this->fn = value;
    else if (name == "dir")
        this->dir = value;
    else if (name == "file")
        this->file = value;
    else if (name == "line")
        this->line = value.toInt();

    this->ErrorLine = ErrorLine;
    this->ErrorFile = ErrorFile;
    this->Message = Message;
    this->MessageVerbose = MessageVerbose;
    this->ProjectPath = ProjectPath;
    this->Severity = Severity;
}

KUrl CppcheckFrame::url() const
{
    if (dir.isEmpty() && file.isEmpty())
        return KUrl();

    KUrl base = KUrl::fromPath(dir);
    base.adjustPath(KUrl::AddTrailingSlash);
    KUrl url(base, file);
    url.cleanPath();
    return url;
}

}

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

#ifndef _CPPCHECKITEMIMPL_H_
#define _CPPCHECKITEMIMPL_H_

#include <KUrl>
#include <QHash>
#include <QList>
#include <QStack>
#include <QSortFilterProxyModel>
#include <QStringList>

#include "cppcheckmodel.h"

namespace cppcheck
{
class CppcheckError;
class CppcheckStack;
class CppcheckFrame;

class CppcheckError : public CppcheckItem
{

public:

    CppcheckError(cppcheck::CppcheckModel* parent);

    virtual ~CppcheckError();

    virtual CppcheckModel* parent() const;

    CppcheckStack *addStack();

    CppcheckStack *lastStack() const;

    const QList<CppcheckStack *> &getStack() const;

    virtual void incomingData(QString name, QString value, int ErrorLine, QString ErrorFile, QString Message, QString MessageVerbose, QString ProjectPath, QString Severity);

    void setKind(const QString& s);

    int uniqueId;
    int threadId;

    enum {
        Unknown,
    } m_kind;

    QString what, auxWhat, text;
    int ErrorLine;
    QString ErrorFile;
    QString Message;
    QString MessageVerbose;
    QString ProjectPath;
    QString Severity;

private:
    CppcheckModel* m_parent;
    QList<CppcheckStack *> m_stack;
};

class CppcheckStack : public CppcheckItem
{
public:
    CppcheckStack(CppcheckError *parent);

    virtual ~CppcheckStack();

    virtual CppcheckError* parent() const;

    virtual void incomingData(QString name, QString value, int ErrorLine, QString ErrorFile, QString Message, QString MessageVerbose, QString ProjectPath, QString Severity);

    QString what() const;

    CppcheckFrame *addFrame();

    CppcheckFrame *lastFrame() const;

    const QList<CppcheckFrame *> &getFrames() const;

private:
    QList<CppcheckFrame*> m_frames;
    CppcheckError* m_parent;
};

/**
 * A frame describes the location of a notification
 */
class CppcheckFrame : public CppcheckItem
{
public:

    /**
     * Takes a pointer on the parent stack
     */
    CppcheckFrame(CppcheckStack* parent);

    virtual CppcheckStack* parent() const;

    virtual void incomingData(QString name, QString value, int ErrorLine, QString ErrorFile, QString Message, QString MessageVerbose, QString ProjectPath, QString Severity);

    KUrl url() const;

    int instructionPointer, line;
    QString obj, fn, dir, file;
    CppcheckStack* m_parent;

private:
    int ErrorLine;
    QString ErrorFile;
    QString Message;
    QString MessageVerbose;
    QString ProjectPath;
    QString Severity;
};
}

#endif /* _CPPCHECKITEMIMPL_H */

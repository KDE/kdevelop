/* This file is part of KDevelo
 * Copyright 2006-2008 Hamish Rodda <rodda@kde.org>
 * Copyright 2011 Mathieu Lornac <mathieu.lornac@gmail.com>
 * Copyright 2011 Damien Coppel <damien.coppel@gmail.com>
 * Copyright 2011 Lionel Duc <lionel.data@gmail.com>
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

#ifndef _CPPCHECKPARSER_H_
#define _CPPCHECKPARSER_H_

#include <QXmlStreamReader>
#include <QApplication>
#include <KDebug>
#include <KMessageBox>
#include <KLocale>

#include "cppcheckitems.h"
#include "cppcheckmodel.h"

#include "iparser.h"

namespace cppcheck
{

/**
 * A class which parses cppcheck's XML output
 * and emits signals when items are parsed
 */
class CppcheckParser : public Parser
{
    Q_OBJECT

public:

    CppcheckParser(QObject* parent = 0);
    ~CppcheckParser();


signals:

    /**
     * Emits this signal when a new item is parsed
     */
    void newElement(cppcheck::Model::eElementType);
    void newData(cppcheck::Model::eElementType, QString name, QString value, int Line, QString SourceFile, QString Message, QString MessageVerbose, QString ProjectPath, QString Severity);

    void reset();

public slots:

    void parse();


private:

    // XML parsing
    bool endElement();
    bool startElement();
    void clear();

    enum State {
        Unknown,
        Results,
        CppCheck,
        Errors,
        Error,
        Location
    };

    QStack<State> m_stateStack;
    QString m_buffer;
    QHash<QString, QString> cppcheckArgs, programArgs;

    // error info
    int ErrorLine;
    QString ErrorFile;
    QString Message;
    QString MessageVerbose;
    QString Severity;
    QString ProjectPath;
};
}


#endif /* _CPPCHECKPARSER_H_ */
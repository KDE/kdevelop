/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "fixedformparser.h"

#include <qfile.h>
#include <qtextstream.h>
#include <kdebug.h>
#include "classstore.h"


FixedFormParser::FixedFormParser(ClassStore *classstore)
{
    store = classstore;

    functionre.compile("(integer|real|logical|complex|character|"
                       "double(precision)?)function([^(]+).*");
    subroutinere.compile("subroutine([^(]+).*");
}


void FixedFormParser::process(const QCString &line, const QString &fileName, int lineNum)
{
    if (line.isEmpty())
        return;
    
    QCString simplified;
    int l = line.length();
    for (int i=0; i < l; ++i)
        if (line[i] != ' ')
            simplified += line[i];

    QCString name;
    if (functionre.match(simplified))
        name = functionre.group(3);
    else if (subroutinere.match(simplified))
        name = subroutinere.group(1);
    else
        return;
    
    ParsedMethod *method = new ParsedMethod;
    method->setName(name);
    method->setDefinedInFile(fileName);
    method->setDefinedOnLine(lineNum);
            
    ParsedMethod *old = store->globalScope()->getMethod(method);
    if (!old)
        store->globalScope()->addMethod(method);
}


void FixedFormParser::parse(const QString &fileName)
{
    QFile f(QFile::encodeName(fileName));
    if (!f.open(IO_ReadOnly))
        return;
    QTextStream stream(&f);

    QCString line;
    int lineNum=0, startLineNum=0;
    while (!stream.atEnd()) {
        ++lineNum;
        QCString str = stream.readLine().latin1();
        if (!str.isEmpty() && QCString("*Cc#!").find(str[0]) != -1)
            continue;
        // Continuation line
        if (str.length() > 6 && str.left(5) == "     " && str[5] != ' ') {
            line += str.right(str.length()-6);
            continue;
        }
        // An initial or invalid line. We don't care
        // about validity
        process(line, fileName, startLineNum);
        line = str.right(str.length()-6);
        startLineNum = lineNum-1;
    }
    process(line, fileName, startLineNum);

    f.close();
}

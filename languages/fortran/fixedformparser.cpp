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
#include <codemodel.h>


FixedFormParser::FixedFormParser(CodeModel* model)
{
    m_model = model;

    functionre.setPattern("(integer|real|logical|complex|character|"
                       "double(precision)?)function([^(]+).*");
    subroutinere.setPattern("subroutine([^(]+).*");

    functionre.setCaseSensitive( false );
    subroutinere.setCaseSensitive( false );
}


void FixedFormParser::process(const QCString &line, const QString &fileName, int lineNum)
{
    QCString simplified;
    int l = line.length();
    for (int i=0; i < l; ++i)
        if (line[i] != ' ')
            simplified += line[i];

    if ( simplified.isEmpty() ) return;

    QString name;
    if (functionre.search(simplified) != -1)
        name = functionre.cap(3);
    else if (subroutinere.search(simplified) != -1) 
        name = subroutinere.cap(1);
    else
        return;

    FunctionDom method = m_model->create<FunctionModel>();
    method->setName(name);
    method->setFileName(fileName);
    method->setStartPosition(lineNum, 0);

    if( !m_file->hasFunction(method->name()) )
        m_file->addFunction(method);
}


void FixedFormParser::parse(const QString &fileName)
{
    QFile f(QFile::encodeName(fileName));
    if (!f.open(IO_ReadOnly))
        return;
    QTextStream stream(&f);

    m_file = m_model->create<FileModel>();
    m_file->setName( fileName );

    QCString line;
    int lineNum=0, startLineNum=0;
    while (!stream.atEnd()) {
        ++lineNum;
        QCString str = stream.readLine().local8Bit();
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

    m_model->addFile( m_file );
}

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

#include <qfile.h>
#include <qtextstream.h>
#include "classstore.h"

#include "fixedformparser.h"


FixedFormParser::FixedFormParser(ClassStore *classstore)
{
    store = classstore;
    stream = 0;
}


void FixedFormParser::parse(const QString &fileName)
{
    QFile f(QFile::encodeName(fileName));
    if (!f.open(IO_ReadOnly))
        return;
    stream = new QTextStream(&f);

    QCString line;
    int lineNo = 0;
    while (!stream->atEnd()) {
        line = stream->readLine();
        ++lineNo;
    }

    delete stream;
    f.close();
}

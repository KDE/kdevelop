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

#ifndef _FIXEDFORMPARSER_H_
#define _FIXEDFORMPARSER_H_

#include <qstring.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <codemodel.h>


class FixedFormParser
{
public:
    FixedFormParser(CodeModel* model);

    void parse(const QString &fileName);

private:
    void process(const QCString &line, const QString &fileName, int lineNum);
    CodeModel* m_model;
    FileDom m_file;
    QRegExp functionre, subroutinere;
};

#endif

/*
 * Copyright (C) 2003 Oliver Kellogg
 * okellogg@users.sourceforge.net
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#ifndef ADA_INDENT_H
#define ADA_INDENT_H

#include "qeditor_indenter.h"
#include <qregexp.h>

class AdaIndent: public QEditorIndenter
{
public:
    AdaIndent( QEditor* );
    virtual ~AdaIndent();

    virtual int indentForLine( int );

private:
/*    QRegExp rxLineEndedWithAColon;
    QRegExp rxStopExecutionStmt;
    QRegExp rxHeaderKeyword;
    QRegExp rxOneLinerStmt;*/
    QRegExp rxCompoundStmt;
};

#endif

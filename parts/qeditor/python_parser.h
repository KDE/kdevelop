/* $Id$
 *
 *  Copyright (C) 2002 Roberto Raggi (raggi@cli.di.unipi.it)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */

#ifndef PYTHON_PARSER_H
#define PYTHON_PARSER_H

#include "background_parser.h"
#include <qregexp.h>

class PythonParser: public BackgroundParser{
public:
    PythonParser( QEditor* =0, const char* =0 );
    virtual ~PythonParser();

protected:
    virtual void parseParag( QTextParag* );
    virtual QTextParag* findGoodStartParag( QTextParag* );
    virtual QTextParag* findGoodEndParag( QTextParag* );

private:
    QRegExp m_startClassRx;
    QRegExp m_startMethodRx;
};

#endif

/***************************************************************************
 *   Copyright (C) 2002 by Roberto Raggi                                   *
 *   raggi@cli.di.unipi.it                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ERRORS_H
#define ERRORS_H

#include <qstring.h>


namespace CppSupport{

    struct Error{
        int code;
        int level;
        QString text;

        Error( int c, int l, const QString& s )
            : code( c ), level( l ), text( s )
            {}
    };

    class Errors{
    public:
        static const Error& InternalError;
        static const Error& SyntaxError;
        static const Error& ParseError;
    };


} // CppSupport namespace

#endif

/***************************************************************************
                          tokenizer.h  -  description
                             -------------------
    begin                : Mon Mar 15 1999
    copyright            : (C) 1999 by Jonas Nordin
    email                : jonas.nordin@cenacle.se
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#ifndef _TOKENIZER_H_INCLUDED
#define _TOKENIZER_H_INCLUDED

#define CPCLASS        258
#define CPPUBLIC       259
#define CPPROTECTED    260
#define CPPRIVATE      261
#define ID             262
#define CLCL           263
#define CPCONST        264
#define CPVOLATILE     265
#define CPUNION        267
#define STRING         268
#define CPSTATIC       269
#define CPVIRTUAL      270
#define QTSIGNAL       271
#define QTSLOT         272
#define CPGLOBAL       273
#define CPENUM         274
#define CPTYPEDEF      275
#define CPFRIEND       276
#define CPSTRUCT       277
#define CPOPERATOR     278
#define CPTEMPLATE     279
#define NUM            280
#define CPTHROW        281
#define CPNAMESPACE      282

typedef const char * YYSTYPE;

extern YYSTYPE yylval;

#endif

/***************************************************************************
                          tokenizer.l  -  description
                             -------------------
    begin                : Mon Mar 15 1999
    copyright            : (C) 1999 by Jonas Nordin
    email                : jonas.nordin@syncom.se
   
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

/* N.B 
 * When adding new tokens, use the CP prefix to avoid nameclashes
 * with the compiler and other packages. 
 *
 * /Jonas
 */

#ifndef _TOKENIZER_H_INCLUDED
#define _TOKENIZER_H_INCLUDED

#define ID				260
#define NUM				261
#define STRING			262

#define JAVAPACKAGE		263
#define JAVACLASS       264
#define JAVAINTERFACE   265
#define JAVAEXTENDS		266
#define JAVAIMPLEMENTS	267
#define JAVAIMPORTS     268
#define JAVAPUBLIC     	269
#define JAVAPROTECTED   270
#define JAVAPRIVATE     271
#define JAVASTATIC      272
#define JAVANATIVE      273
#define JAVAABSTRACT    274
#define JAVAFINAL       275
#define JAVASYNCHRONIZED	276
#define JAVATRANSIENT  	277
#define JAVAVOLATILE	278
#define JAVASTRICTFP	279
#define JAVATHROWS		280
#define JAVATHROW		281

typedef const char * YYSTYPE;

extern YYSTYPE yylval;

#endif
